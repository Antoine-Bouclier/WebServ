#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "http/RequestHandler.hpp"

using std::string;

RequestHandler::RequestHandler() {}

RequestHandler::~RequestHandler() {}

HttpStatusCode RequestHandler::fileError()
{
	if (errno == ENOENT || errno == ENOTDIR) return (NOT_FOUND);
	if (errno == EACCES || errno == EPERM || errno == ELOOP || errno == EROFS) return (FORBIDDEN);
	return (INTERNAL_SERVER_ERROR);
}

bool RequestHandler::prepareFile(HttpResponse& response, const string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0)
		return (false);
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) return (false);
	response.setFile(path, info.st_size);
	return (true);
}

string RequestHandler::buildFilePath(const string& path, const string& root, const string& prefix)
{
	if (path.compare(0, prefix.size(), prefix) != 0)
		throw std::runtime_error("Invalid location prefix");
	string suffix = path.substr(prefix.size());
	string result = root;

	if (suffix.empty())
		return (result);
	if (!result.empty() && result[result.size() - 1] != '/' && suffix[0] != '/')
		result += '/';
	else if (!result.empty() && result[result.size() - 1] == '/' && suffix[0] == '/')
		suffix.erase(0, 1);
	return (result + suffix);
}

string	RequestHandler::getEffectiveRoot(const ConfigLocation* location, const ConfigServer* server)
{
	string root = "";
	if (location && !location->getRoot().empty())
		root = location->getRoot();
	else if (server && !server->getRoot().empty())
		root = server->getRoot();
	else
		root = "./www";
	return (root);
}

std::vector<std::string> RequestHandler::buildCgiEnv(const HttpRequest& request, const std::string& script_path)
{
	std::vector<std::string> env;

	env.push_back("REQUEST_METHOD=" + request.getMethod());
	env.push_back("SCRIPT_FILENAME=" + script_path);
	env.push_back("QUERY_STRING=" + request.getQuery());
	env.push_back("SERVER_PROTOCOL=" + request.getVersion());
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	
	env.push_back("REDIRECT_STATUS=200");

	const std::map<std::string, std::string>& headers = request.getheaders();
	std::map<std::string, std::string>::const_iterator it;

	it = headers.find("content-type");
	if (it != headers.end())
		env.push_back("CONTENT_TYPE=" + it->second);
		
	it = headers.find("content-length");
	if (it != headers.end())
		env.push_back("CONTENT_LENGTH=" + it->second);

	for (it = headers.begin(); it != headers.end(); ++it)
	{
		std::string header_name = "HTTP_";
		for (size_t i = 0; i < it->first.size(); ++i)
		{
			if (it->first[i] == '-')
				header_name += '_';
			else
				header_name += std::toupper(static_cast<unsigned char>(it->first[i]));
		}
		env.push_back(header_name + "=" + it->second);
	}

	return env;
}

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	if (location)
	{
		const std::vector<string>& methods = location->getMethods();
		if (std::find(methods.begin(), methods.end(), request.getMethod()) == methods.end())
		{
			HttpResponse response = buildErrorResponse(METHOD_NOT_ALLOWED, location, server);
			string allowed;
			for (size_t i = 0; i < methods.size(); ++i) allowed += (i ? ", " : "") + methods[i];
			response.addHeader("Allow", allowed);
			return (response);
		}
	}
	if (location && location->getRedirect().first)
	{
		HttpResponse response;
		response.setStatus(static_cast<HttpStatusCode>(location->getRedirect().first));
		response.addHeader("Location", location->getRedirect().second);
		return (response);
	}

	std::string path = buildFilePath(request.getPath(), getEffectiveRoot(location, server), location ? location->getPath() : "");
	size_t dotPos = path.find_last_of('.');
	
	if (location && dotPos != std::string::npos)
	{
		std::string ext = path.substr(dotPos);
		const std::map<std::string, std::string>& cgi_map = location->getCgi(); //[cite: 2]
		std::map<std::string, std::string>::const_iterator it = cgi_map.find(ext);
		
		if (it != cgi_map.end())
			return handleCgi(request, location, server, path, it->second);
	}

	if (request.getMethod() == "GET")
		return (handleGet(request, location, server));
	if (request.getMethod() == "DELETE")
		return (handleDelete(request, location, server));
	if (request.getMethod() == "POST")
		return (handlePost(request, location, server));
	return (buildErrorResponse(NOT_IMPLEMENTED, location, server));
}

HttpResponse RequestHandler::handleCgi(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server, const std::string& script_path, const std::string& cgi_bin)
{
	std::vector<std::string> env_vec = buildCgiEnv(request, script_path);
	
	char** envp = new char*[env_vec.size() + 1];
	for (size_t i = 0; i < env_vec.size(); ++i)
		envp[i] = const_cast<char*>(env_vec[i].c_str());
	envp[env_vec.size()] = NULL;

	char* argv[3];
	argv[0] = const_cast<char*>(cgi_bin.c_str());
	argv[1] = const_cast<char*>(script_path.c_str());
	argv[2] = NULL;

	int pipe_in[2];
	int pipe_out[2];
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
	{
		delete[] envp;
		return buildErrorResponse(INTERNAL_SERVER_ERROR, location, server);
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		delete[] envp;
		return buildErrorResponse(INTERNAL_SERVER_ERROR, location, server);
	}

	if (pid == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);

		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);

		execve(argv[0], argv, envp);
		
		std::exit(EXIT_FAILURE);
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);

		const std::vector<char>& body = request.getBody(); //
		if (!body.empty())
			write(pipe_in[1], &body[0], body.size());
		
		close(pipe_in[1]);

		std::vector<char> cgi_output;
		char buffer[4096];
		ssize_t bytes_read;
		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			cgi_output.insert(cgi_output.end(), buffer, buffer + bytes_read);
		
		close(pipe_out[0]);

		int status;
		waitpid(pid, &status, 0);
		delete[] envp;

		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			return buildErrorResponse(INTERNAL_SERVER_ERROR, location, server);

		HttpResponse response;
		response.setStatus(OK);
		response.setBody(cgi_output);
		
		return response;
	}
}

HttpResponse	RequestHandler::buildErrorResponse(HttpStatusCode error, const ConfigLocation* loc, const ConfigServer* server)
{
	HttpResponse	response;
	string		error_page_path = "";

	if (loc != NULL)
		error_page_path = loc->getErrorPagePath(error);
	
	if (error_page_path.empty() && server != NULL)
		error_page_path = server->getErrorPagePath(error);
	
	if (!error_page_path.empty())
	{
		string	root = getEffectiveRoot(loc, server);
		string full_path = buildFilePath(error_page_path, root, "");

		if (prepareFile(response, full_path))
		{
			response.addHeader("Content-Type", "text/html");
			response.setStatus(error);
			return (response);
		}
	}

	std::ostringstream	out;

	out << "<html><head><title>" 
		<< error << " " << getReasonPhrase(error) 
		<< "</title></head><body><center><h1>"
		<< error << " " << getReasonPhrase(error)
		<< "</h1></center></body></html>";

	string	content = out.str();
	std::vector<char>	body(content.begin(), content.end());

	response.setBody(body);
	response.addHeader("Content-Type", "text/html");
	response.setStatus(error);

	return (response);
}

RequestHandler::RequestHandler(const RequestHandler& other) { (void)other; }

RequestHandler& RequestHandler::operator=(const RequestHandler& other)
{
	(void)other;
	return (*this);
}

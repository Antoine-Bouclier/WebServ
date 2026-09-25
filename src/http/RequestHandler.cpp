#include <cerrno>
#include "http/RequestHandler.hpp"

using std::string;

RequestHandler::RequestHandler() {}

RequestHandler::~RequestHandler() {}

HttpStatusCode RequestHandler::fileError()
{
	if (errno == ENOENT || errno == ENOTDIR)
		return (NOT_FOUND);
	if (errno == EACCES || errno == EPERM || errno == ELOOP || errno == EROFS)
		return (FORBIDDEN);
	return (INTERNAL_SERVER_ERROR);
}

bool RequestHandler::prepareFile(HttpResponse& response, const string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0)
		return (false);
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
		return (false);
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

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	if (location)
	{
		const std::vector<string>& methods = location->getMethods();
		if (std::find(methods.begin(), methods.end(), request.getMethod()) == methods.end())
		{
			HttpResponse response = buildErrorResponse(METHOD_NOT_ALLOWED, location, server);
			string allowed;
			for (size_t i = 0; i < methods.size(); ++i)
				allowed += (i ? ", " : "") + methods[i];
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

	HttpResponse response;
	if (location && resolveCgi(request.getPath(), *location, response))
	{
		if (response.getStatus() != OK)
			return (buildErrorResponse(response.getStatus(), location, server));
		return (response);
	}

	if (request.getMethod() == "GET")
		return (handleGet(request, location, server));
	if (request.getMethod() == "DELETE")
		return (handleDelete(request, location, server));
	if (request.getMethod() == "POST")
		return (handlePost(request, location, server));
	return (buildErrorResponse(NOT_IMPLEMENTED, location, server));
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

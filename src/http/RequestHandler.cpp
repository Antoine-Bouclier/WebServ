#include "http/RequestHandler.hpp"

RequestHandler::RequestHandler()
{
}

RequestHandler::~RequestHandler()
{
}

std::string RequestHandler::buildFilePath(const std::string& uri, const std::string& root)
{
	std::string	path = root;

	if (uri == "/")
		return (path);
	if (!path.empty() && path[path.length() - 1] == '/' && uri[0] == '/')
		path.erase(path.length() - 1);
	return (path + uri);
}

bool RequestHandler::isDirectory(const std::string& path)
{
	struct stat	info;

	if (stat(path.c_str(), &info) != 0)
		return (false);
	return (S_ISDIR(info.st_mode));
}

bool RequestHandler::isRegularFile(const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISREG(info.st_mode);
}

std::string	RequestHandler::getMimeType(const std::string& path)
{
	size_t dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dotPos);

	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".png") return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif") return "image/gif";
	if (ext == ".ico") return "image/x-icon";
	if (ext == ".txt") return "text/plain";
	if (ext == ".json") return "application/json";

	return "application/octet-stream";
}

std::string	RequestHandler::getEffectiveRoot(const ConfigLocation* location, const ConfigServer* server)
{
	std::string root = "";
	if (location && !location->getRoot().empty())
		root = location->getRoot();
	else if (server && !server->getRoot().empty())
		root = server->getRoot();
	else
		root = "./www";
	return (root);
}

HttpResponse RequestHandler::generateAutoindex(const std::string& uri, const std::string& target_path, const ConfigLocation* location, const ConfigServer* server)
{
	std::ostringstream	out;
	HttpResponse		response;

	out << "<html>\n"
		<< "<head><title>Index of " << uri << "</title></head>\n"
		<< "<body>\n"
		<< "<h1>Index of " << uri << "</h1>\n"
		<< "<ul>\n";
	
	DIR*	dir = opendir(target_path.c_str());
	if (dir == NULL)
		return (buildErrorResponse(INTERNAL_SERVER_ERROR, location, server));

	std::string href_base = uri;
	if (!href_base.empty() && href_base[href_base.length() - 1] != '/')
		href_base += "/";

	struct dirent	*ent;
	while ((ent = readdir(dir)) != NULL)
	{
		if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
			continue;

		out << "<li><a href=\"" << href_base << ent->d_name
			<< "\">" << ent->d_name << "</a></li>";
	}
	closedir(dir);
	out << "</ul></body></html>";

	std::string content = out.str();
	std::vector<char> body(content.begin(), content.end());

	response.setBody(body);
	response.setStatus(OK);
	response.addHeader("Content-Type", "text/html");

	return (response);
}

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	HttpResponse	response;

	if (location != NULL && !location->getMethods().empty())
	{
		const std::vector<std::string>& allowed = location->getMethods();
		if (std::find(allowed.begin(), allowed.end(), request.getMethod()) == allowed.end())
		{
			return buildErrorResponse(METHOD_NOT_ALLOWED, location, server);
		}
	}

	if (request.getMethod() == "GET")
	{
		std::string	root = getEffectiveRoot(location, server);
		std::string target_path = buildFilePath(request.getUri(), root);
	
		if (isDirectory(target_path))
		{
			std::vector<std::string>	index_list;
			if (location && !location->getIndex().empty())
				index_list = location->getIndex();
			else if (server && !server->getIndex().empty())
				index_list = server->getIndex();
			
			bool	index_found = false;
	
			for (std::vector<std::string>::const_iterator it = index_list.begin(); it != index_list.end(); ++it)
			{
				std::string	index_path = target_path;
				if (!index_path.empty() && index_path[index_path.length() - 1] != '/')
					index_path += "/";
				index_path += *it;
				if (isRegularFile(index_path))
				{
					target_path = index_path;
					response.setStatus(OK);
					index_found = true;
					break;
				}
			}
	
			if (!index_found)
			{
				if (location != NULL && location->getAutoindex())
					return (generateAutoindex(request.getUri(), target_path, location, server));
				else
					return (buildErrorResponse(FORBIDDEN, location, server));
			}
		}
		if (isRegularFile(target_path))
		{
			std::ifstream file(target_path.c_str(), std::ios::binary);
	
			if (!file.is_open())
				return (buildErrorResponse(FORBIDDEN, location, server));
			else
			{
				file.seekg(0, std::ios::end);
				std::streamsize fileSize = file.tellg();
				file.seekg(0, std::ios::beg);
	
				std::vector<char> buffer(fileSize);
				if (file.read(&buffer[0], fileSize))
				{
					response.setBody(buffer);
					response.setStatus(OK);
	
					std::ostringstream ss;
					ss << fileSize;
					
					response.addHeader("Content-Type", getMimeType(target_path));
				}
				else
					response.setStatus(INTERNAL_SERVER_ERROR);
			}
		}
		else
			return (buildErrorResponse(NOT_FOUND, location, server));
	}


	return response;
}

HttpResponse	RequestHandler::buildErrorResponse(HttpStatusCode error, const ConfigLocation* loc, const ConfigServer* server)
{
	HttpResponse	response;
	std::string		error_page_path = "";

	if (loc != NULL)
		error_page_path = loc->getErrorPagePath(error);
	
	if (error_page_path.empty() && server != NULL)
		error_page_path = server->getErrorPagePath(error);
	
	if (!error_page_path.empty())
	{
		std::string	root = getEffectiveRoot(loc, server);
		std::string full_path = buildFilePath(error_page_path, root);

		std::ifstream	file(full_path.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open())
		{
			std::vector<char> body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			
			response.setBody(body);
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

	std::string	content = out.str();
	std::vector<char>	body(content.begin(), content.end());

	response.setBody(body);
	response.addHeader("Content-Type", "text/html");
	response.setStatus(error);

	return (response);
}

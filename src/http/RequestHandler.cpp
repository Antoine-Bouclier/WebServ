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

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	HttpResponse	response;

	std::string root = "";
	if (location && !location->getRoot().empty())
		root = location->getRoot();
	else if (server && !server->getRoot().empty())
		root = server->getRoot();
	else
		root = "./www";

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
			response.setStatus(FORBIDDEN);
	}
	if (isRegularFile(target_path))
	{
		std::ifstream file(target_path.c_str(), std::ios::binary);

		if (!file.is_open())
			response.setStatus(FORBIDDEN);
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
		response.setStatus(NOT_FOUND);

	return response;
}

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

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{

}

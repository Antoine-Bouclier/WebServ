#include <cerrno>
#include <cstdio>
#include "http/RequestHandler.hpp"

#define MAX_AUTOINDEX_SIZE (1024 * 1024)

RequestHandler::RequestHandler() {}

RequestHandler::~RequestHandler() {}

static std::string escapeHtml(const std::string& text)
{
	std::string result;
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '&') result += "&amp;";
		else if (text[i] == '<') result += "&lt;";
		else if (text[i] == '>') result += "&gt;";
		else if (text[i] == '"') result += "&quot;";
		else if (text[i] == '\'') result += "&#39;";
		else result += text[i];
	}
	return (result);
}

static std::string encodePath(const std::string& path)
{
	std::string result;
	for (size_t i = 0; i < path.size(); ++i)
	{
		unsigned char c = path[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
			result += c;
		else
		{
			result += '%';
			result += "0123456789ABCDEF"[c / 16];
			result += "0123456789ABCDEF"[c % 16];
		}
	}
	return (result);
}

static HttpStatusCode fileError()
{
	if (errno == ENOENT || errno == ENOTDIR) return (NOT_FOUND);
	if (errno == EACCES || errno == EPERM || errno == ELOOP) return (FORBIDDEN);
	return (INTERNAL_SERVER_ERROR);
}

static bool prepareFile(HttpResponse& response, const std::string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0)
		return (false);
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file) return (false);
	response.setFile(path, info.st_size);
	return (true);
}

std::string RequestHandler::buildFilePath(const std::string& path, const std::string& root, const std::string& prefix)
{
	if (path.compare(0, prefix.size(), prefix) != 0)
		throw std::runtime_error("Invalid location prefix");
	std::string suffix = path.substr(prefix.size());
	std::string result = root;

	if (suffix.empty())
		return (result);
	if (!result.empty() && result[result.size() - 1] != '/' && suffix[0] != '/')
		result += '/';
	else if (!result.empty() && result[result.size() - 1] == '/' && suffix[0] == '/')
		suffix.erase(0, 1);
	return (result + suffix);
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
	for (size_t i = 0; i < ext.size(); ++i)
		ext[i] = std::tolower(static_cast<unsigned char>(ext[i]));
	if (ext == ".svg") return "image/svg+xml";
	if (ext == ".pdf") return "application/pdf";
	if (ext == ".woff2") return "font/woff2";
	if (ext == ".webp") return "image/webp";

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
	DIR* dir = opendir(target_path.c_str());
	if (!dir) return (buildErrorResponse(fileError(), location, server));
	try
	{
		std::string content = "<html><head><title>Index of " + escapeHtml(uri) + "</title></head><body><h1>Index of " + escapeHtml(uri) + "</h1><ul>";
		std::string base = uri;
		if (base.empty() || base[base.size() - 1] != '/') base += '/';
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name == "." || name == "..") continue;
			if (isDirectory(buildFilePath(name, target_path, ""))) name += '/';
			content += "<li><a href=\"" + encodePath(base + name) + "\">" + escapeHtml(name) + "</a></li>";
			if (content.size() > MAX_AUTOINDEX_SIZE)
			{
				closedir(dir);
				dir = NULL;
				return (buildErrorResponse(INTERNAL_SERVER_ERROR, location, server));
			}
		}
		closedir(dir);
		dir = NULL;
		content += "</ul></body></html>";
		HttpResponse response;
		response.setBody(std::vector<char>(content.begin(), content.end()));
		response.addHeader("Content-Type", "text/html; charset=utf-8");
		return (response);
	}
	catch (...)
	{
		if (dir) closedir(dir);
		throw;
	}
}

HttpResponse RequestHandler::handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	if (location)
	{
		const std::vector<std::string>& methods = location->getMethods();
		if (std::find(methods.begin(), methods.end(), request.getMethod()) == methods.end())
		{
			HttpResponse response = buildErrorResponse(METHOD_NOT_ALLOWED, location, server);
			std::string allowed;
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
	if (request.getMethod() == "GET")
		return (handleGet(request, location, server));
	if (request.getMethod() == "DELETE")
		return (handleDelete(request, location, server));
	if (request.getMethod() == "POST")
    	return (handlePost(request, location, server));
	return (buildErrorResponse(NOT_IMPLEMENTED, location, server));
}

HttpResponse RequestHandler::handleDelete(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	std::string path = buildFilePath(request.getPath(), getEffectiveRoot(location, server), location ? location->getPath() : "");
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return (buildErrorResponse(fileError(), location, server));
	if (!S_ISREG(info.st_mode))
		return (buildErrorResponse(FORBIDDEN, location, server));

	if (std::remove(path.c_str()) != 0)
	{
		HttpStatusCode status = fileError();

		if (errno == EROFS)
			status = FORBIDDEN;
		return (buildErrorResponse(status, location, server));
	}

	HttpResponse response;
	response.setStatus(NO_CONTENT);
	return (response);
}

HttpResponse RequestHandler::handleGet(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	std::string path = buildFilePath(request.getPath(), getEffectiveRoot(location, server), location ? location->getPath() : "");
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return (buildErrorResponse(fileError(), location, server));
	if (S_ISDIR(info.st_mode))
	{
		if (request.getPath()[request.getPath().size() - 1] != '/')
		{
			HttpResponse response;
			response.setStatus(MOVED_PERMANENTLY);
			std::string target = encodePath(request.getPath()) + "/";
			if (!request.getQuery().empty()) target += "?" + request.getQuery();
			response.addHeader("Location", target);
			return (response);
		}
		std::vector<std::string> indexes;
		if (location) indexes = location->getIndex();
		else if (server) indexes = server->getIndex();
		bool found = false;
		for (size_t i = 0; i < indexes.size(); ++i)
		{
			std::string candidate = buildFilePath(indexes[i], path, "");
			if (stat(candidate.c_str(), &info) != 0)
			{
				HttpStatusCode error = fileError();
				if (error != NOT_FOUND) return (buildErrorResponse(error, location, server));
				continue;
			}
			if (S_ISREG(info.st_mode)) { path = candidate; found = true; break; }
		}
		if (!found)
		{
			if (location && location->getAutoindex()) return (generateAutoindex(request.getPath(), path, location, server));
			return (buildErrorResponse(FORBIDDEN, location, server));
		}
	}
	else if (!S_ISREG(info.st_mode))
		return (buildErrorResponse(FORBIDDEN, location, server));
	HttpResponse response;
	if (!prepareFile(response, path)) return (buildErrorResponse(FORBIDDEN, location, server));
	response.addHeader("Content-Type", getMimeType(path));
	return (response);
}

HttpResponse RequestHandler::handlePost(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
    (void)request;

    if (!location || location->getUploadPath().empty())
        return (buildErrorResponse(FORBIDDEN, location, server));

    const std::string& destination = location->getUploadPath();
    struct stat info;

    if (stat(destination.c_str(), &info) != 0)
        return (buildErrorResponse(fileError(), location, server));
    if (!S_ISDIR(info.st_mode))
        return (buildErrorResponse(INTERNAL_SERVER_ERROR, location, server));

    return (buildErrorResponse(NOT_IMPLEMENTED, location, server));
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
		std::string full_path = buildFilePath(error_page_path, root, "");

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

	std::string	content = out.str();
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

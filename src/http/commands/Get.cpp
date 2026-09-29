#include "http/RequestHandler.hpp"
#include "utils/StringUtils.hpp"

#define MAX_AUTOINDEX_SIZE (1024 * 1024)

using std::string;

static bool isDirectory(const string& path);
static string escapeHtml(const string& text);
static string encodePath(const string& path);
static string getMimeType(const string& path);
static HttpResponse redirectDirectory(const HttpRequest& request);

HttpResponse RequestHandler::handleGet(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	string path = buildFilePath(request.getPath(), getEffectiveRoot(location, server), location ? location->getPath() : "");
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return (buildErrorResponse(fileError(), location, server));
	if (S_ISDIR(info.st_mode))
	{
		if (request.getPath()[request.getPath().size() - 1] != '/')
			return (redirectDirectory(request));

		std::vector<string> indexes;

		if (location)
			indexes = location->getIndex();
		else if (server)
			indexes = server->getIndex();

		HttpStatusCode status = findIndexFile(path, indexes);
		if (status == NOT_FOUND)
		{
			if (location && location->getAutoindex())
				return (generateAutoindex(request.getPath(), path, location, server));
			return (buildErrorResponse(FORBIDDEN, location, server));
		}
		if (status != OK)
			return (buildErrorResponse(status, location, server));
	}
	else if (!S_ISREG(info.st_mode))
		return (buildErrorResponse(FORBIDDEN, location, server));

	HttpResponse response;
	if (!prepareFile(response, path))
		return (buildErrorResponse(FORBIDDEN, location, server));
	response.addHeader("Content-Type", getMimeType(path));
	return (response);
}

HttpStatusCode RequestHandler::findIndexFile(string& path, const std::vector<string>& indexes)
{
	for (size_t i = 0; i < indexes.size(); ++i)
	{
		string candidate = buildFilePath(indexes[i], path, "");
		struct stat info;
		if (stat(candidate.c_str(), &info) != 0)
		{
			HttpStatusCode error = fileError();
			if (error != NOT_FOUND)
				return (error);
			continue;
		}
		if (S_ISREG(info.st_mode))
		{
			path = candidate;
			return (OK);
		}
	}
	return (NOT_FOUND);
}

HttpResponse RequestHandler::generateAutoindex(const string& uri, const string& target_path, const ConfigLocation* location, const ConfigServer* server)
{
	DIR* dir = opendir(target_path.c_str());
	if (!dir)
		return (buildErrorResponse(fileError(), location, server));
	try
	{
		string content = "<html><head><title>Index of " + escapeHtml(uri) + "</title></head><body><h1>Index of " + escapeHtml(uri) + "</h1><ul>";
		string base = uri;
		if (base.empty() || base[base.size() - 1] != '/')
			base += '/';
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL)
		{
			string name = entry->d_name;
			if (name == "." || name == "..")
				continue;
			if (isDirectory(buildFilePath(name, target_path, "")))
				name += '/';
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
		if (dir)
			closedir(dir);
		throw;
	}
}

static HttpResponse redirectDirectory(const HttpRequest& request)
{
	string target = encodePath(request.getPath()) + "/";
	if (!request.getQuery().empty())
		target += "?" + request.getQuery();

	HttpResponse response;
	response.setStatus(MOVED_PERMANENTLY);
	response.addHeader("Location", target);
	return (response);
}

static bool isDirectory(const string& path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return (false);
	return (S_ISDIR(info.st_mode));
}

static string getMimeType(const string& path)
{
	size_t dotPos = path.find_last_of('.');

	if (dotPos == string::npos)
		return ("application/octet-stream");

	string ext = lowercase(path.substr(dotPos));
	if (ext == ".svg") return ("image/svg+xml");
	if (ext == ".pdf") return ("application/pdf");
	if (ext == ".woff2") return ("font/woff2");
	if (ext == ".webp") return ("image/webp");
	if (ext == ".html" || ext == ".htm") return ("text/html");
	if (ext == ".css") return ("text/css");
	if (ext == ".js") return ("application/javascript");
	if (ext == ".png") return ("image/png");
	if (ext == ".jpg" || ext == ".jpeg") return ("image/jpeg");
	if (ext == ".gif") return ("image/gif");
	if (ext == ".ico") return ("image/x-icon");
	if (ext == ".txt") return ("text/plain");
	if (ext == ".json") return ("application/json");

	return ("application/octet-stream");
}

static string escapeHtml(const string& text)
{
	string result;
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '&')
			result += "&amp;";
		else if (text[i] == '<')
			result += "&lt;";
		else if (text[i] == '>')
			result += "&gt;";
		else if (text[i] == '"')
			result += "&quot;";
		else if (text[i] == '\'')
			result += "&#39;";
		else
			result += text[i];
	}
	return (result);
}

static string encodePath(const string& path)
{
	string result;
	for (size_t i = 0; i < path.size(); ++i)
	{
		unsigned char c = path[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
			|| c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
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

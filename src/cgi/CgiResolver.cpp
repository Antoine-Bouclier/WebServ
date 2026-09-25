#include <cerrno>
#include <unistd.h>
#include "http/RequestHandler.hpp"

using std::string;
using std::vector;

static bool hasCgiExtension(const string& path, const ConfigLocation& location);

bool RequestHandler::resolveCgi(const string& uri, const ConfigLocation& location, HttpResponse& response)
{
	if (location.getCgi().empty())
		return (false);

	string scriptUri = findCgiScriptUri(uri, location);
	if (scriptUri.empty())
		return (false);

	string scriptPath = buildFilePath(scriptUri, location.getRoot(), location.getPath());
	string extension = scriptPath.substr(scriptPath.find_last_of('.'));
	const string& interpreter = location.getCgi().find(extension)->second;
	HttpStatusCode status = validateCgiTarget(scriptPath, interpreter);

	if (status != OK)
	{
		response.setStatus(status);
		return (true);
	}

	string pathInfo = scriptUri.size() < uri.size() ? uri.substr(scriptUri.size()) : "";
	response.setCgi(scriptPath, interpreter, scriptUri, pathInfo);
	return (true);
}

string RequestHandler::findCgiScriptUri(const string& uri, const ConfigLocation& location)
{
	size_t position = location.getPath().size();
	while (position <= uri.size())
	{
		size_t end = uri.find('/', position);
		if (end == string::npos)
			end = uri.size();

		string scriptUri = uri.substr(0, end);
		string scriptPath = buildFilePath(scriptUri, location.getRoot(), location.getPath());
		if (hasCgiExtension(scriptPath, location))
			return (scriptUri);
		position = end + 1;
	}
	return (findCgiIndexUri(uri, location));
}

string RequestHandler::findCgiIndexUri(const string& uri, const ConfigLocation& location)
{
	if (uri.empty() || uri[uri.size() - 1] != '/')
		return ("");

	string directory = buildFilePath(uri, location.getRoot(), location.getPath());
	struct stat fileInfo;
	if (stat(directory.c_str(), &fileInfo) != 0 || !S_ISDIR(fileInfo.st_mode))
		return ("");

	const vector<string>& indexes = location.getIndex();
	for (size_t i = 0; i < indexes.size(); ++i)
	{
		string indexPath = buildFilePath(indexes[i], directory, "");
		if (stat(indexPath.c_str(), &fileInfo) != 0)
		{
			if (errno == ENOENT || errno == ENOTDIR)
				continue;
			return ("");
		}
		if (S_ISREG(fileInfo.st_mode))
			return (hasCgiExtension(indexPath, location) ? uri + indexes[i] : "");
	}
	return ("");
}

HttpStatusCode RequestHandler::validateCgiTarget(const string& scriptPath, const string& interpreter)
{
	struct stat fileInfo;
	if (stat(scriptPath.c_str(), &fileInfo) != 0)
		return (fileError());
	if (!S_ISREG(fileInfo.st_mode) || access(scriptPath.c_str(), R_OK) != 0)
		return (FORBIDDEN);
	if (interpreter.empty() || interpreter[0] != '/' || access(interpreter.c_str(), X_OK) != 0)
		return (INTERNAL_SERVER_ERROR);
	return (OK);
}

static bool hasCgiExtension(const string& path, const ConfigLocation& location)
{
	size_t dot = path.find_last_of('.');
	return (dot != string::npos && location.getCgi().count(path.substr(dot)) != 0);
}

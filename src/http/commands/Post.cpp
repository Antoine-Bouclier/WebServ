#include "http/RequestHandler.hpp"
#include "utils/StringUtils.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#define UPLOAD_WRITE_SIZE 16384

using std::map;
using std::string;

typedef map<string, string> Parameters;
typedef map<string, std::pair<size_t, size_t> > Uploads;

static bool parseParameters(const string& text, string& type, Parameters& parameters)
{
	size_t pos = text.find(';');
	type = lowercase(trim(text.substr(0, pos)));
	while (pos != string::npos)
	{
		size_t start = ++pos;
		bool quoted = false;
		for (; pos < text.size(); ++pos)
		{
			if (quoted && text[pos] == '\\' && pos + 1 < text.size())
				++pos;
			else if (text[pos] == '"')
				quoted = !quoted;
			else if (!quoted && text[pos] == ';')
				break;
		}
		string name, value;
		if (quoted || !splitOnce(text.substr(start, pos - start), '=', name, value))
			return (false);
		name = lowercase(trim(name));
		value = trim(value);
		if (!isToken(name) || value.empty() || parameters.count(name))
			return (false);
		if (value[0] == '"')
		{
			if (value.size() < 2 || value[value.size() - 1] != '"')
				return (false);
			string decoded;
			for (size_t i = 1; i + 1 < value.size(); ++i)
			{
				if (value[i] == '\\')
				{
					if (++i >= value.size() - 1)
						return (false);
				}
				else if (value[i] == '"')
					return (false);
				unsigned char c = value[i];
				if (c < 32 || c == 127)
					return (false);
				decoded += value[i];
			}
			value = decoded;
		}
		else if (!isToken(value))
			return (false);
		parameters[name] = value;
		if (pos == text.size())
			break;
	}
	return (!type.empty());
}

static bool validBoundary(const string& boundary)
{
	if (boundary.empty() || boundary.size() > 70 || boundary[boundary.size() - 1] == ' ')
		return (false);
	for (size_t i = 0; i < boundary.size(); ++i)
	{
		unsigned char c = boundary[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || string("'()+_,-./:=? ").find(c) != string::npos))
			return (false);
	}
	return (true);
}

static bool validFilename(const string& name)
{
	if (name == "." || name == ".." || name.find_first_of("/\\") != string::npos)
		return (false);
	for (size_t i = 0; i < name.size(); ++i)
		if (static_cast<unsigned char>(name[i]) < 32 || name[i] == 127)
			return (false);
	return (!name.empty());
}

static bool partFilename(const string& headers, string& filename)
{
	Parameters fields;
	for (size_t pos = 0; pos < headers.size();)
	{
		size_t end = headers.find("\r\n", pos);
		if (end == string::npos)
			end = headers.size();
		string name, value;
		if (!splitOnce(headers.substr(pos, end - pos), ':', name, value) || !isToken(name))
			return (false);
		name = lowercase(name);
		value = trim(value);
		for (size_t i = 0; i < value.size(); ++i)
			if ((static_cast<unsigned char>(value[i]) < 32 && value[i] != '\t') || value[i] == 127)
				return (false);
		if (fields.count(name))
			return (false);
		fields[name] = value;
		pos = end == headers.size() ? end : end + 2;
	}
	Parameters parameters;
	string type;
	if (!parseParameters(fields["content-disposition"], type, parameters) || type != "form-data" || !parameters.count("name"))
		return (false);
	if (fields.count("content-transfer-encoding") && lowercase(fields["content-transfer-encoding"]) != "binary")
		return (false);
	filename = parameters.count("filename") ? parameters["filename"] : "";
	return (filename.empty() || validFilename(filename));
}

static bool delimiter(const string& body, size_t pos, const string& marker, size_t& next, bool& last)
{
	if (body.compare(pos, marker.size(), marker) != 0)
		return (false);
	next = pos + marker.size();
	last = body.compare(next, 2, "--") == 0;
	if (last)
		next += 2;
	while (next < body.size() && (body[next] == ' ' || body[next] == '\t'))
		++next;
	if (last && next == body.size())
		return (true);
	if (body.compare(next, 2, "\r\n") != 0)
		return (false);
	next += 2;
	return (true);
}

static bool parseUploads(const string& body, const string& boundary, Uploads& files)
{
	string marker = "--" + boundary;
	size_t pos = 0;
	bool last = false;
	if (!delimiter(body, 0, marker, pos, last))
		return (false);
	while (!last)
	{
		size_t end = body.find("\r\n\r\n", pos);
		if (end == string::npos || end - pos > MAX_HEADER_SIZE)
			return (false);
		string filename;
		if (!partFilename(body.substr(pos, end - pos), filename))
			return (false);
		size_t start = end + 4;
		end = start;
		do
		{
			end = body.find("\r\n" + marker, end);
			if (end == string::npos)
				return (false);
			if (delimiter(body, end + 2, marker, pos, last))
				break;
			end += 2;
		} while (true);
		if (!filename.empty())
		{
			if (files.count(filename))
				return (false);
			files[filename] = std::make_pair(start, end - start);
		}
	}
	return (!files.empty());
}

static HttpStatusCode saveUploads(const string& body, const string& destination, const Uploads& files)
{
	std::vector<string> paths;
	for (Uploads::const_iterator it = files.begin(); it != files.end(); ++it)
		paths.push_back(destination + (destination[destination.size() - 1] == '/' ? "" : "/") + it->first);
	size_t created = 0;
	HttpStatusCode status = CREATED;
	for (Uploads::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		int fd = open(paths[created].c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
		if (fd < 0)
		{
			status = errno == EEXIST ? CONFLICT : INTERNAL_SERVER_ERROR;
			if (errno == EACCES || errno == EPERM || errno == EROFS)
				status = FORBIDDEN;
			break;
		}
		++created;
		size_t written = 0;
		while (written < it->second.second)
		{
			size_t count = std::min(it->second.second - written, static_cast<size_t>(UPLOAD_WRITE_SIZE));
			ssize_t result = write(fd, body.data() + it->second.first + written, count);
			if (result <= 0)
			{
				status = INTERNAL_SERVER_ERROR;
				break;
			}
			written += static_cast<size_t>(result);
		}
		if (close(fd) != 0)
			status = INTERNAL_SERVER_ERROR;
		if (status != CREATED)
			break;
	}
	if (status != CREATED)
		for (size_t i = 0; i < created; ++i)
			std::remove(paths[i].c_str());
	return (status);
}

HttpResponse RequestHandler::handlePost(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	if (!location || location->getUploadPath().empty())
		return (buildErrorResponse(FORBIDDEN, location, server));
	const string& destination = location->getUploadPath();
	struct stat info;
	if (stat(destination.c_str(), &info) != 0)
		return (buildErrorResponse(fileError(), location, server));
	if (!S_ISDIR(info.st_mode))
		return (buildErrorResponse(INTERNAL_SERVER_ERROR, location, server));

	const Parameters& headers = request.getheaders();
	Parameters::const_iterator it = headers.find("content-type");
	if (it == headers.end())
		return (buildErrorResponse(UNSUPPORTED_MEDIA_TYPE, location, server));
	if (lowercase(trim(it->second.substr(0, it->second.find(';')))) != "multipart/form-data")
		return (buildErrorResponse(UNSUPPORTED_MEDIA_TYPE, location, server));
	string type;
	Parameters parameters;
	if (!parseParameters(it->second, type, parameters) || !validBoundary(parameters["boundary"]))
		return (buildErrorResponse(BAD_REQUEST, location, server));

	string body(request.getBody().begin(), request.getBody().end());
	Uploads files;
	if (!parseUploads(body, parameters["boundary"], files))
		return (buildErrorResponse(BAD_REQUEST, location, server));
	HttpStatusCode status = saveUploads(body, destination, files);
	if (status != CREATED)
		return (buildErrorResponse(status, location, server));
	HttpResponse response;
	response.setStatus(CREATED);
	return (response);
}

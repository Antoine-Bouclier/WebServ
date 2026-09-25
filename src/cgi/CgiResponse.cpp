#include <sstream>
#include <stdexcept>
#include "cgi/Cgi.hpp"
#include "utils/StringUtils.hpp"

using std::map;
using std::string;
using std::vector;

static size_t parseLength(const string& value)
{
	if (value.empty())
		throw std::runtime_error("Empty CGI length");
	for (size_t i = 0; i < value.size(); ++i)
		if (value[i] < '0' || value[i] > '9')
			throw std::runtime_error("Invalid CGI length");
	std::istringstream input(value);
	size_t length;
	input >> length;
	if (input.fail() || !input.eof())
		throw std::runtime_error("Invalid CGI length");
	return (length);
}

HttpResponse Cgi::response() const
{
	size_t end = _data.find("\r\n\r\n");
	size_t separator = 4;
	size_t lf = _data.find("\n\n");
	if (lf != string::npos && (end == string::npos || lf < end))
	{
		end = lf;
		separator = 2;
	}
	if (end == string::npos || end > CGI_HEADER_LIMIT)
		throw std::runtime_error("Missing CGI headers");

	map<string, string> headers;
	std::istringstream lines(_data.substr(0, end));
	string line;
	while (std::getline(lines, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		string key;
		string value;
		if (!splitOnce(line, ':', key, value) || !isToken(key))
			throw std::runtime_error("Invalid CGI header");
		key = lowercase(key);
		value = trim(value);
		for (size_t i = 0; i < value.size(); ++i)
		{
			unsigned char c = value[i];
			if ((c < 32 && c != '\t') || c == 127)
				throw std::runtime_error("Invalid CGI header value");
		}
		if (!headers.insert(std::make_pair(key, value)).second)
			throw std::runtime_error("Duplicate CGI header");
	}

	HttpResponse result;
	if (headers.count("status"))
	{
		const string& status = headers["status"];
		if (status.size() < 3 || (status.size() > 3 && status[3] != ' '))
			throw std::runtime_error("Invalid CGI status");
		size_t code = parseLength(status.substr(0, 3));
		if (code < 200 || code > 599)
			throw std::runtime_error("Invalid CGI status");
		result.setStatus(static_cast<HttpStatusCode>(code));
		if (status.size() > 4)
			result.setReason(status.substr(4));
	}
	else if (headers.count("location"))
		result.setStatus(FOUND);

	size_t bodySize = _data.size() - end - separator;
	if (headers.count("transfer-encoding"))
		throw std::runtime_error("Encoded CGI output");
	if (headers.count("content-length") && parseLength(headers["content-length"]) != bodySize)
		throw std::runtime_error("Truncated CGI body");
	if (result.getStatus() != NO_CONTENT && result.getStatus() != NOT_MODIFIED && !headers.count("location") && (!headers.count("content-type") || headers["content-type"].empty()))
		throw std::runtime_error("Missing CGI content type");
	if (headers.count("location") && headers["location"].empty())
		throw std::runtime_error("Empty CGI location");

	for (map<string, string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		if (it->first != "status" && it->first != "content-length" && it->first != "connection" && it->first != "keep-alive" && it->first != "trailer" && it->first != "upgrade" && it->first != "proxy-connection")
			result.addHeader(it->first, it->second);
	if (result.getStatus() != NO_CONTENT && result.getStatus() != NOT_MODIFIED)
		result.setBody(vector<char>(_data.begin() + end + separator, _data.end()));
	return (result);
}

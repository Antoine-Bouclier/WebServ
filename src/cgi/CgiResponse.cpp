#include <sstream>
#include <stdexcept>
#include "cgi/Cgi.hpp"
#include "utils/StringUtils.hpp"

using std::map;
using std::string;
using std::vector;

typedef map<string, string> Headers;

static size_t findHeaderEnd(const string& data, size_t& separator);
static Headers parseHeaders(const string& text);
static void applyStatus(Headers& headers, HttpResponse& result);
static void validateBody(Headers& headers, HttpStatusCode status, size_t bodySize);

HttpResponse Cgi::response() const
{
	size_t separator;
	size_t headerEnd = findHeaderEnd(_data, separator);
	size_t bodyStart = headerEnd + separator;
	Headers headers = parseHeaders(_data.substr(0, headerEnd));

	HttpResponse result;
	applyStatus(headers, result);
	validateBody(headers, result.getStatus(), _data.size() - bodyStart);

	for (Headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first != "status" &&
			it->first != "content-length" &&
			it->first != "connection" &&
			it->first != "keep-alive" &&
			it->first != "trailer" &&
			it->first != "upgrade" &&
			it->first != "proxy-connection")
			result.addHeader(it->first, it->second);
	}
	if (result.getStatus() != NO_CONTENT && result.getStatus() != NOT_MODIFIED)
		result.setBody(vector<char>(_data.begin() + bodyStart, _data.end()));
	return (result);
}

static size_t findHeaderEnd(const string& data, size_t& separator)
{
	size_t end = data.find("\r\n\r\n");
	separator = 4;
	size_t lf = data.find("\n\n");
	if (lf != string::npos && (end == string::npos || lf < end))
	{
		end = lf;
		separator = 2;
	}
	if (end == string::npos || end > CGI_HEADER_LIMIT)
		throw std::runtime_error("Missing CGI headers");

	return (end);
}

static Headers parseHeaders(const string& text)
{
	string line;
	Headers headers;
	std::istringstream lines(text);

	while (std::getline(lines, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		string key;
		string value;
		if (!parseHeaderLine(line, key, value))
			throw std::runtime_error("Invalid CGI header");
		if (!headers.insert(std::make_pair(key, value)).second)
			throw std::runtime_error("Duplicate CGI header");
	}

	return (headers);
}

static void applyStatus(Headers& headers, HttpResponse& result)
{
	if (headers.count("status"))
	{
		const string& status = headers["status"];
		if (status.size() < 3 || (status.size() > 3 && status[3] != ' '))
			throw std::runtime_error("Invalid CGI status");
		size_t code;
		if (!parseUnsignedNumber(status.substr(0, 3), code) || code < 200 || code > 599)
			throw std::runtime_error("Invalid CGI status");
		result.setStatus(static_cast<HttpStatusCode>(code));
		if (status.size() > 4)
			result.setReason(status.substr(4));
	}
	else if (headers.count("location"))
		result.setStatus(FOUND);
}

static void validateBody(Headers& headers, HttpStatusCode status, size_t bodySize)
{
	if (headers.count("transfer-encoding"))
		throw std::runtime_error("Encoded CGI output");
	if (headers.count("content-length"))
	{
		size_t length;
		if (!parseUnsignedNumber(headers["content-length"], length))
			throw std::runtime_error("Invalid CGI length");
		if (length != bodySize)
			throw std::runtime_error("Truncated CGI body");
	}
	if (status != NO_CONTENT && status != NOT_MODIFIED && !headers.count("location") && (!headers.count("content-type") || headers["content-type"].empty()))
		throw std::runtime_error("Missing CGI content type");
	if (headers.count("location") && headers["location"].empty())
		throw std::runtime_error("Empty CGI location");
}

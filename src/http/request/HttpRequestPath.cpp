#include "http/HttpRequest.hpp"
#include "utils/StringUtils.hpp"

using std::string;
static bool percentDecode(const string& input, string& output);
static bool normalizePath(const string& input, string& output);

void	HttpRequest::isValidURI()
{
	if (_uri.empty() || _uri[0] != '/')
		return (_state = STATE_ERROR, void());
	for (size_t i = 0; i < _uri.size(); ++i)
	{
		if (static_cast<unsigned char>(_uri[i]) <= ' ' || _uri[i] == 127)
			return (_state = STATE_ERROR, void());
	}
}

void HttpRequest::cleanUriToPath()
{
	if (_state == STATE_ERROR)
		return ;

	_path = _uri;
	_query.clear();
	splitOnce(_uri, '?', _path, _query);
	_path = _path.substr(0, _path.find('#'));
	_query = _query.substr(0, _query.find('#'));
}

void HttpRequest::parsePath()
{
	string decoded;

	if (!percentDecode(_path, decoded))
	{
		_status_code = BAD_REQUEST;
		_state = STATE_ERROR;
		return;
	}
	if (!normalizePath(decoded, _path))
	{
		_status_code = FORBIDDEN;
		_state = STATE_ERROR;
	}
}

static bool percentDecode(const string& input, string& output)
{
	output.clear();
	for (size_t i = 0; i < input.size(); ++i)
	{
		char c = input[i];
		if (c == '%')
		{
			if (i + 2 >= input.size())
				return (false);
			int high = hexValue(input[i + 1]);
			int low = hexValue(input[i + 2]);
			if (high < 0 || low < 0)
				return (false);
			c = static_cast<char>(high * 16 + low);
			i += 2;
		}
		if (c == '\0')
			return (false);
		output += c;
	}
	return (true);
}

static bool normalizePath(const string& input, string& output)
{
	string normalized = "/";
	size_t len = input.size();
	bool directory = (len > 0 && input[len - 1] == '/') || (len >= 2 && input.substr(len - 2) == "/.");

	for (size_t start = 0, end; start < input.size(); start = end + 1)
	{
		end = input.find('/', start);
		if (end == string::npos)
			end = input.size();

		string segment = input.substr(start, end - start);
		if (segment == "..")
			return (false);

		if (!segment.empty() && segment != ".")
			normalized += segment + '/';
	}

	if (normalized.size() > 1 && !directory)
		normalized.erase(normalized.size() - 1);

	output = normalized;
	return (true);
}

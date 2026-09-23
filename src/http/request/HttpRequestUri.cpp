#include "http/HttpRequest.hpp"
#include "utils/StringUtils.hpp"

using std::string;
using std::vector;
using std::map;

void	HttpRequest::isValidURI()
{
	for (size_t i = 0; i < _uri.size(); ++i)
	{
		if (static_cast<unsigned char>(_uri[i]) <= 32 || _uri[i] == 127)
		{
			_state = STATE_ERROR;
			return;
		}
	}
	if (_uri.empty() || _uri[0] != '/')
	{
		_state = STATE_ERROR;
		return ;
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

void HttpRequest::decodePath()
{
	string decoded;

	for (size_t i = 0; i < _path.size(); ++i)
	{
		char c = _path[i];
		if (c == '%')
		{
			if (i + 2 >= _path.size() || hexValue(_path[i + 1]) < 0 || hexValue(_path[i + 2]) < 0)
			{
				_state = STATE_ERROR;
				return;
			}
			c = static_cast<char>(hexValue(_path[i + 1]) * 16 + hexValue(_path[i + 2]));
			i += 2;
		}
		if (c == '\0')
		{
			_state = STATE_ERROR;
			return;
		}
		decoded += c;
	}
	_path = decoded;

	for (size_t start = 0; start < _path.size();)
	{
		size_t end = _path.find('/', start);
		if (end == string::npos)
			end = _path.size();
		if (_path.substr(start, end - start) == "..")
		{
			_status_code = FORBIDDEN;
			_state = STATE_ERROR;
			return;
		}
		start = end + 1;
	}
}

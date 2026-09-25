#include "http/HttpRequest.hpp"
#include "utils/StringUtils.hpp"

using std::string;
using std::vector;
using std::map;

bool HttpRequest::checkHeaderSize(size_t end)
{
    if (end <= MAX_HEADER_SIZE)
        return (true);
    _status_code = BAD_REQUEST;
    _state = STATE_ERROR;
    return (false);
}

void	HttpRequest::parseRequestLine()
{
	vector<char>::iterator	it;

	if (!searchEOL(it))
	{
		checkHeaderSize(_buffer.size());
		return;
	}
	if (!checkHeaderSize(static_cast<size_t>(it - _buffer.begin()) + 2))
		return;

	string	request_line(_buffer.begin() + _position_ptr, it);

	_position_ptr = it - _buffer.begin() + 2;
	if (std::count(request_line.begin(), request_line.end(), ' ') != 2)
	{
		_state = STATE_ERROR;
		return ;
	}

	std::stringstream	ss(request_line);
	string				extra;
	
	ss >> _method >> _uri >> _version;
	if (!ss || ss >> extra)
	{
		_state = STATE_ERROR;
		return ;
	}

	isValidURI();
	if (_state == STATE_ERROR)
		return ;

	cleanUriToPath();
	parsePath();
	if (_state == STATE_ERROR)
		return;

	_state = STATE_HEADERS;
}

void	HttpRequest::parseHeaders()
{
	vector<char>::iterator	it;
	if (!searchEOL(it))
	{
		checkHeaderSize(_buffer.size());
		return;
	}
	if (!checkHeaderSize(static_cast<size_t>(it - _buffer.begin()) + 2))
		return;

	string	header_line(_buffer.begin() + _position_ptr, it);
	if (header_line.empty())
	{
		_position_ptr += 2;
		_state = STATE_HEADERS_DONE;
		return ;
	}

	string key;
	string value;
	if (!splitOnce(header_line, ':', key, value))
	{
		_state = STATE_ERROR;
		return ;
	}
	if (!isToken(key)) { _state = STATE_ERROR; return; }
	for (size_t i = 0; i < value.size(); ++i)
	{
		unsigned char c = value[i];
		if ((c < 32 && c != '\t') || c == 127) { _state = STATE_ERROR; return; }
	}
	key = lowercase(key);
	value = trim(value);

	if ((key == "host" || key == "content-length" || key == "transfer-encoding") && _headers.count(key) > 0)
	{
		_state = STATE_ERROR;
		return ;
	}

	_headers.insert(std::make_pair(key, value));

	_position_ptr = it - _buffer.begin() + 2;

	if (key == "transfer-encoding" && value == "chunked")
		_is_chunked = true;
}

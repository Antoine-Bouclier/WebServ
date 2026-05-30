#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _state(STATE_REQUEST_LINE), _position_ptr(0), _content_length(0), _is_chunked(false)
{

}

HttpRequest::HttpRequest(const HttpRequest& src)
{
	*this = src;
}

HttpRequest& HttpRequest::operator=(const HttpRequest& src)
{
	if (this != &src)
	{
		/* -- Request Line-- */
		_method = src._method;
		_uri = src._uri;
		_version = src._version;

		/* -- Headers -- */
		_headers = src._headers;

		/* -- Body -- */
		_body = src._body;

		/* -- Stream Control & Internal State -- */
		_state = src._state;
		_buffer = src._buffer;
		_position_ptr = src._position_ptr;
		_content_length = src._content_length;
		_is_chunked = src._is_chunked;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{

}

void	HttpRequest::parseRequestLine()
{
	std::vector<char>::iterator	it;

	if (!searchEOL(it))
		return ;

	std::string	request_line(_buffer.begin() + _position_ptr, it);

	_position_ptr = it - _buffer.begin() + 2;
	if (std::count(request_line.begin(), request_line.end(), ' ') != 2)
	{
		_state = STATE_ERROR;
		return ;
	}

	std::stringstream	ss(request_line);
	std::string extra;
	
	ss >> _method >> _uri >> _version;
	if (!ss || ss >> extra)
	{
		_state = STATE_ERROR;
		return ;
	}

	isValidRequestLine();
	if (_state == STATE_ERROR)
		return ;

	_state = STATE_HEADERS;
}

void	HttpRequest::isValidRequestLine()
{
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
	{
		_state = STATE_ERROR;
		return ;
	}
	
	if (_uri.empty() || _uri[0] != '/')
	{
		_state = STATE_ERROR;
		return ;
	}

	if (_version != "HTTP/1.1")
	{
		_state = STATE_ERROR;
		return ;
	}
}

void	HttpRequest::parseHeaders()
{
	std::vector<char>::iterator	it;
	if (!searchEOL(it))
		return ;

	std::string	header_line(_buffer.begin() + _position_ptr, it);
	if (header_line.empty())
	{
		_position_ptr += 2;
		return ;
	}

	size_t	found = header_line.find(':');
	if (found == std::string::npos)
	{
		_state = STATE_ERROR;
		return ;
	}
	
	std::pair<std::string, std::string>	header(header_line.substr(0, found), header_line.substr(found + 1));
	_headers.insert(header);

	_position_ptr += header_line.size();
}

bool	HttpRequest::searchEOL(std::vector<char>::iterator& it)
{
	std::string	eol = "\r\n";
	it = std::search(_buffer.begin() + _position_ptr, _buffer.end(), eol.begin(), eol.end());

	if (it == _buffer.end())
		return (false);

	return (true);
}

void	HttpRequest::parseBody()
{

}


void	HttpRequest::feed(const char* raw_bytes, size_t bytes_count)
{
	_buffer.insert(_buffer.end(), raw_bytes, raw_bytes + bytes_count);
	while (_state != STATE_READY)
	{
		size_t			old_position = _position_ptr;
		HttpParseState	old_state = _state;

		if (_state == STATE_REQUEST_LINE)
			parseRequestLine();
		else if (_state == STATE_HEADERS)
			parseHeaders();
		else if (_state == STATE_BODY)
			parseBody();

		if (_state == STATE_ERROR || (_state == old_state && _position_ptr == old_position))
			break;
	}
}
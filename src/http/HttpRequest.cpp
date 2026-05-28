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

}

void	HttpRequest::parseHeaders()
{

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
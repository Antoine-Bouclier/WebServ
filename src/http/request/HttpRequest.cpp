#include "http/HttpRequest.hpp"
#include "http/RequestValidator.hpp"

using std::string;
using std::vector;
using std::map;

HttpRequest::HttpRequest()
	:	_state(STATE_REQUEST_LINE),
		_position_ptr(0), _content_length(0),
		_current_chunk_size(0),
		_max_body_size(0),
		_status_code(BAD_REQUEST),
		_is_chunked(false),
		_reading_chunk_headers(true),
		_reading_trailers(false){}

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
		_query = src._query;
		_path = src._path;

		/* -- Headers -- */
		_headers = src._headers;

		/* -- Body -- */
		_body = src._body;

		/* -- Stream Control & Internal State -- */
		_state = src._state;
		_buffer = src._buffer;
		_position_ptr = src._position_ptr;
		_content_length = src._content_length;
		_current_chunk_size = src._current_chunk_size;
		_max_body_size = src._max_body_size;
		_status_code = src._status_code;
		_is_chunked = src._is_chunked;
		_reading_chunk_headers = src._reading_chunk_headers;
		_reading_trailers = src._reading_trailers;
	}
	return (*this);
}

HttpRequest::~HttpRequest(){}

const HttpParseState&	HttpRequest::getState() const{ return (_state); }

HttpStatusCode HttpRequest::getStatusCode() const { return (_status_code); }

const string&		HttpRequest::getMethod() const{ return (_method); }

const string&		HttpRequest::getUri() const{ return (_uri); }

const string&		HttpRequest::getVersion() const{ return (_version); }

const string&		HttpRequest::getPath() const{ return (_path); }

const string&		HttpRequest::getQuery() const{ return (_query); }

const vector<char>&	HttpRequest::getBody() const{ return (_body);}

const map<string, string>&	HttpRequest::getheaders() const{ return (_headers); }

void HttpRequest::feed(const char* data, size_t size)
{
	if (_state == STATE_READY || _state == STATE_ERROR)
		return;
	_buffer.insert(_buffer.end(), data, data + size);
	parse();
}

void HttpRequest::startBody(const AConfig& config)
{
	RequestValidator validator;
	HttpStatusCode status = validator.validate(*this, config);

	if (status != OK)
	{
		_status_code = status;
		_state = STATE_ERROR;
		return;
	}
	_max_body_size = config.getClientMaxBody();
	if (_headers.count("content-length"))
	{
		std::istringstream length(_headers["content-length"]);
		length >> _content_length;
	}
	_state = (_is_chunked || _headers.count("content-length")) ? STATE_BODY : STATE_READY;
	parse();
}

void HttpRequest::parse()
{
	while (_state != STATE_READY && _state != STATE_ERROR && _state != STATE_HEADERS_DONE)
	{
		size_t old_position = _position_ptr;
		HttpParseState old_state = _state;

		if (_state == STATE_REQUEST_LINE)
			parseRequestLine();
		else if (_state == STATE_HEADERS)
			parseHeaders();
		else if (_state == STATE_BODY)
			parseBody();

		if (_state == old_state && _position_ptr == old_position)
			break;
	}
	if (_state == STATE_BODY && _position_ptr)
	{
		_buffer.erase(_buffer.begin(), _buffer.begin() + _position_ptr);
		_position_ptr = 0;
	}
	else if (_state == STATE_READY || _state == STATE_ERROR)
	{
		vector<char>().swap(_buffer);
		_position_ptr = 0;
	}
}

bool	HttpRequest::searchEOL(vector<char>::iterator& it)
{
	string	eol = "\r\n";
	it = std::search(_buffer.begin() + _position_ptr, _buffer.end(), eol.begin(), eol.end());

	return (it != _buffer.end());
}

void HttpRequest::releaseBody()
{
	vector<char>().swap(_body);
	vector<char>().swap(_buffer);
	_position_ptr = 0;
}

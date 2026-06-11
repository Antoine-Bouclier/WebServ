#include "http/HttpRequest.hpp"

HttpRequest::HttpRequest() : _state(STATE_REQUEST_LINE), _position_ptr(0), _content_length(0), _current_chunk_size(0), _is_chunked(false), _reading_chunk_headers(false)
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
		_current_chunk_size = src._current_chunk_size;
		_is_chunked = src._is_chunked;
		_reading_chunk_headers = src._reading_chunk_headers;
	}
	return (*this);
}

HttpRequest::~HttpRequest()
{

}

/* -- Getters -- */
const HttpParseState&	HttpRequest::getState() const{ return (_state); }
const std::string&		HttpRequest::getMethod() const{ return (_method); }
const std::string&		HttpRequest::getUri() const{ return (_uri); }
const std::string&		HttpRequest::getVersion() const{ return (_version); }

/* -- Utils Function -- */

/**
 * @brief Validates the syntactical conformity of the HTTP Request-Line.
 * 
 * This method checks three mandatory requirements according to the HTTP protocol:
 * 1. The method must be either GET, POST, or DELETE.
 * 2. The URI must not be empty and must start with a forward slash ('/').
 * 3. The HTTP version must strictly be "HTTP/1.1".
 * 
 * If any check fails, the internal state transitions to `STATE_ERROR`.
 */
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

/**
 * @brief Searches for the next End-Of-Line (EOL) sequence in the data buffer.
 * 
 * Scans the internal buffer starting from the current read pointer (`_position_ptr`)
 * to find the standard HTTP CRLF (`"\r\n"`) sequence.
 * 
 * @param[out] it An iterator that will store the position of the first '\\r' character if found.
 * @return true If a complete CRLF sequence was found.
 * @return false If the EOL sequence is missing (indicating incomplete data in the buffer).
 */
bool	HttpRequest::searchEOL(std::vector<char>::iterator& it)
{
	std::string	eol = "\r\n";
	it = std::search(_buffer.begin() + _position_ptr, _buffer.end(), eol.begin(), eol.end());

	if (it == _buffer.end())
		return (false);

	return (true);
}

bool	HttpRequest::skipEOL()
{
	if (_buffer.size() - _position_ptr < 2)
		return (false);
	if (_buffer[_position_ptr] != '\r' || _buffer[_position_ptr + 1] != '\n')
	{
		_state = STATE_ERROR;
		return (false);
	}
	_position_ptr += 2;
	return (true);
}

void	HttpRequest::parseBodyContentLength()
{
	std::istringstream	iss(_headers["content-length"]);
	iss >> _content_length;

	size_t	to_copy = std::min(_content_length - _body.size(), _buffer.size() - _position_ptr);

	std::vector<char>::iterator	it = _buffer.begin() + _position_ptr;
	_body.insert(_body.end(), it, it + to_copy);

	_position_ptr += to_copy;

	if (_body.size() == _content_length)
		_state = STATE_READY;
}

void	HttpRequest::parseBodyTransferEncoding()
{
	if (_reading_chunk_headers)
	{
		std::vector<char>::iterator	it;
	
		if (!searchEOL(it))
			return ;
		
		std::string			hex(_buffer.begin() + _position_ptr, it);
		std::istringstream	iss(hex);
	
		iss >> std::hex >> _current_chunk_size;
	
		if (_current_chunk_size == 0)
		{
			if (_buffer.end() - it < 4)
				return ;
			
			if (*(it + 2) != '\r' || *(it + 3) != '\n')
			{
				_state = STATE_ERROR;
				return ;
			}
			_position_ptr = (it - _buffer.begin()) + 4;
			_state = STATE_READY;
			return ;
		}
		_position_ptr = it - _buffer.begin() + 2;
		_reading_chunk_headers = false;
	}
	else
	{
		size_t	to_copy = std::min(_buffer.size() - _position_ptr, _current_chunk_size);

		std::vector<char>::iterator	it = _buffer.begin() + _position_ptr;
		_body.insert(_body.end(), it, it + to_copy);

		_position_ptr += to_copy;
		_current_chunk_size -= to_copy;

		if (_current_chunk_size == 0)
		{
			if (!skipEOL())
				return ;
			_reading_chunk_headers = true;
		}
	}
}


/* -- Parsing Sub-routine -- */

/**
 * @brief Extracts and parses the HTTP Request-Line.
 * 
 * Tokenizes the first line received to extract the HTTP method, URI, and version.
 * It strictly ensures that exactly two spaces act as delimiters. If valid, the read 
 * pointer advances past the line, and the state machine transitions to `STATE_HEADERS`.
 * 
 * @note If the line is incomplete, the execution halts to wait for more data.
 *       Any structural anomaly will force the state to `STATE_ERROR`.
 */
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

/**
 * @brief Extracts, normalizes, and stores a single HTTP Header line.
 * 
 * Processes a single line from the buffer:
 * - If the line is empty, it marks the end of the header section (CRLF line). 
 *   The parser then determines whether to transition to `STATE_BODY` or `STATE_READY` 
 *   based on the presence of payload-related headers.
 * - If a key-value pair is found, it converts the key to lowercase (for HTTP 
 *   case-insensitivity), trims leading whitespaces from the value, and inserts the 
 *   pair into the `_headers` map.
 */
void	HttpRequest::parseHeaders()
{
	std::vector<char>::iterator	it;
	if (!searchEOL(it))
		return ;

	std::string	header_line(_buffer.begin() + _position_ptr, it);
	if (header_line.empty())
	{
		_position_ptr += 2;
		if (_headers.find("content-length") != _headers.end() || _headers.find("transfer-encoding") != _headers.end())
			_state = STATE_BODY;
		else
			_state = STATE_READY;
		return ;
	}

	size_t	found = header_line.find(':');
	if (found == std::string::npos)
	{
		_state = STATE_ERROR;
		return ;
	}
	std::string key = header_line.substr(0, found);
	std::string value = header_line.substr(found + 1);

	for (size_t i = 0; i < key.length(); ++i)
		key[i] = std::tolower(key[i]);

	while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
		value.erase(0, 1);

	std::pair<std::string, std::string> header(key, value);
	_headers.insert(header);

	_position_ptr = it - _buffer.begin() + 2;

	if (_headers.find("transfer-encoding") != _headers.end() && _headers["transfer-encoding"] == "chunked")
		_is_chunked = true;
}

void	HttpRequest::parseBody()
{
	if (_is_chunked)
		parseBodyTransferEncoding();
	else if (_headers.find("content-length") != _headers.end())
		parseBodyContentLength();
	else
		_state = STATE_READY;
}

/* -- Main Function -- */

/**
 * @brief Feeds raw byte chunks into the parser and triggers the state machine.
 * 
 * Appends the incoming bytes to the internal stream buffer, then processes the data through 
 * consecutive parsing stages (Request-Line -> Headers -> Body) as long as progress is being made 
 * and no errors are encountered.
 * 
 * @param[in] raw_bytes Pointer to the array of raw bytes read from the network socket.
 * @param[in] bytes_count Total number of bytes to append and process.
 */
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
#include "http/HttpRequest.hpp"

static int hexValue(char c);

HttpRequest::HttpRequest()
	:	_state(STATE_REQUEST_LINE),
		_position_ptr(0), _content_length(0),
		_current_chunk_size(0),
		_max_body_size(0),
		_status_code(BAD_REQUEST),
		_has_duplicate_host(false),
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
		_has_duplicate_host = src._has_duplicate_host;
		_is_chunked = src._is_chunked;
		_reading_chunk_headers = src._reading_chunk_headers;
		_reading_trailers = src._reading_trailers;
	}
	return (*this);
}

HttpRequest::~HttpRequest(){}

/* ------------- */
/* -- GETTERS -- */
/* ------------- */

const HttpParseState&	HttpRequest::getState() const{ return (_state); }

HttpStatusCode HttpRequest::getStatusCode() const { return (_status_code); }

/* -- Request Line Getters -- */
const std::string&		HttpRequest::getMethod() const{ return (_method); }
const std::string&		HttpRequest::getUri() const{ return (_uri); }
const std::string&		HttpRequest::getVersion() const{ return (_version); }

const std::string&		HttpRequest::getPath() const{ return (_path); }
const std::string&		HttpRequest::getQuery() const{ return (_query); }

/* -- Body Getter -- */
const std::vector<char>&	HttpRequest::getBody() const{ return (_body);}

/* -- Headers Getter -- */
const std::map<std::string, std::string>&	HttpRequest::getheaders() const{ return (_headers); }

/* -------------------- */
/* -- UTILS METHODS -- */
/* -------------------- */

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
void	HttpRequest::isValidURI()
{
	if (_uri.empty() || _uri[0] != '/')
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

void HttpRequest::parseBodyTransferEncoding(size_t max_body_size)
{
	if (_reading_chunk_headers || _reading_trailers)
	{
		std::vector<char>::iterator it;
		if (!searchEOL(it))
			return;
		std::string line(_buffer.begin() + _position_ptr, it);

		if (_reading_trailers)
		{
			if (!line.empty())
			{
				_state = STATE_ERROR;
				return;
			}
			_state = STATE_READY;
		}
		else
		{
			size_t size = 0;
			if (line.empty())
			{
				_state = STATE_ERROR;
				return;
			}
			for (size_t i = 0; i < line.size(); ++i)
			{
				int digit = hexValue(line[i]);
				if (digit < 0 || size > (static_cast<size_t>(-1) - digit) / 16)
				{
					_state = STATE_ERROR;
					return;
				}
				size = size * 16 + digit;
			}
			if (max_body_size > 0 && (_body.size() > max_body_size || size > max_body_size - _body.size()))
			{
				_status_code = PAYLOAD_TOO_LARGE;
				_state = STATE_ERROR;
				return;
			}
			_current_chunk_size = size;
			_reading_chunk_headers = false;
			_reading_trailers = (size == 0);
		}
		_position_ptr = it - _buffer.begin() + 2;
		return;
	}

	size_t count = std::min(_buffer.size() - _position_ptr, _current_chunk_size);
	std::vector<char>::iterator it = _buffer.begin() + _position_ptr;
	_body.insert(_body.end(), it, it + count);
	_position_ptr += count;
	_current_chunk_size -= count;
	if (_current_chunk_size == 0 && skipEOL())
		_reading_chunk_headers = true;
}

void	HttpRequest::resumeParsing()
{
	if (_headers.find("content-length") != _headers.end() || _is_chunked)
		_state = STATE_BODY;
	else
		_state = STATE_READY;
}

static int hexValue(char c)
{
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
	return (-1);
}

void HttpRequest::decodePath()
{
	std::string decoded;

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
		if (end == std::string::npos)
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

void HttpRequest::cleanUriToPath()
{
	if (_state == STATE_ERROR)
		return ;

	_path = _uri;

	std::size_t q_pos = _uri.find('?');
	if (q_pos != std::string::npos)
	{
		_path = _uri.substr(0, q_pos);
		
		std::size_t hash_pos = _uri.find('#', q_pos);
		if (hash_pos != std::string::npos)
			_query = _uri.substr(q_pos + 1, hash_pos - (q_pos + 1));
		else
			_query = _uri.substr(q_pos + 1);
	}

	std::size_t hash_pos = _path.find('#');
	if (hash_pos != std::string::npos)
	{
		_path = _path.substr(0, hash_pos);
	}
}

/* ------------------------- */
/* -- PARSING SUB-ROUTINE -- */
/* ------------------------- */

/**
 * @brief Extracts and parses the HTTP Request-Line.
 * 
 * Tokenizes the first line received to extract the HTTP method, URI, and version.
 * It strictly ensures that exactly two spaces act as delimiters. If valid, the read 
 * pointer advances past the line, and the state machine transitions to `STATE_HEADERS`.
 * 
 * @note If the line is incomplete, the execution halts to wait for more data.
 *	   Any structural anomaly will force the state to `STATE_ERROR`.
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

	isValidURI();
	if (_state == STATE_ERROR)
		return ;

	cleanUriToPath();
	decodePath();
	if (_state == STATE_ERROR)
		return;

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
		_state = STATE_HEADERS_DONE;
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

	if ((key == "host" || key == "content-length") && _headers.count(key) > 0)
	{
		_state = STATE_ERROR;
		return ;
	}

	std::pair<std::string, std::string> header(key, value);
	_headers.insert(header);

	_position_ptr = it - _buffer.begin() + 2;

	if (_headers.find("transfer-encoding") != _headers.end() && _headers["transfer-encoding"] == "chunked")
		_is_chunked = true;
}

void	HttpRequest::parseBody(size_t max_body_size)
{
	if (_is_chunked)
		parseBodyTransferEncoding(max_body_size);
	else if (_headers.find("content-length") != _headers.end())
		parseBodyContentLength();
	else
		_state = STATE_READY;
}
/* ------------------ */
/* -- MAIN METHOD -- */
/* ------------------ */

void HttpRequest::feed(const char* data, size_t size)
{
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
	resumeParsing();
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
			parseBody(_max_body_size);

		if (_state == old_state && _position_ptr == old_position)
			break;
	}
}

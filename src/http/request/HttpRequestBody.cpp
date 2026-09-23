#include "http/HttpRequest.hpp"
#include "utils/StringUtils.hpp"

using std::string;
using std::vector;
using std::map;

void	HttpRequest::parseBody()
{
	if (_is_chunked)
		parseBodyTransferEncoding();
	else
		parseBodyContentLength();
}

void	HttpRequest::parseBodyContentLength()
{
	size_t	to_copy = std::min(_content_length - _body.size(), _buffer.size() - _position_ptr);

	vector<char>::iterator	it = _buffer.begin() + _position_ptr;
	_body.insert(_body.end(), it, it + to_copy);

	_position_ptr += to_copy;

	if (_body.size() == _content_length)
		_state = STATE_READY;
}

void HttpRequest::parseBodyTransferEncoding()
{
	if (_reading_chunk_headers || _reading_trailers)
	{
		vector<char>::iterator it;
		if (!searchEOL(it))
		{
			if (_buffer.size() - _position_ptr > MAX_HEADER_SIZE)
				_state = STATE_ERROR;
			return;
		}
		if (static_cast<size_t>(it - _buffer.begin()) - _position_ptr > MAX_HEADER_SIZE)
		{
			_state = STATE_ERROR;
			return;
		}
		string line(_buffer.begin() + _position_ptr, it);

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
			if (_max_body_size > 0 && (_body.size() > _max_body_size || size > _max_body_size - _body.size()))
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
	vector<char>::iterator it = _buffer.begin() + _position_ptr;
	_body.insert(_body.end(), it, it + count);
	_position_ptr += count;
	_current_chunk_size -= count;
	if (_current_chunk_size == 0 && skipEOL())
		_reading_chunk_headers = true;
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

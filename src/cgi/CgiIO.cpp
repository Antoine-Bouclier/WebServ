#include <unistd.h>
#include <algorithm>
#include "cgi/Cgi.hpp"

using std::string;
using std::vector;

void Cgi::writeInput()
{
	size_t count = std::min(_body.size() - _offset, static_cast<size_t>(CGI_BUFFER_SIZE));
	ssize_t written = write(_input, &_body[_offset], count);
	if (written <= 0)
	{
		closeInput();
		return;
	}

	_offset += written;
	if (_offset == _body.size())
		closeInput();
}

void Cgi::readOutput()
{
	char buffer[CGI_BUFFER_SIZE];
	ssize_t count = read(_output, buffer, sizeof(buffer));
	if (count < 0)
	{
		cancel(BAD_GATEWAY);
		return;
	}
	if (!count)
	{
		closeOutput();
		return;
	}
	if (static_cast<size_t>(count) > CGI_OUTPUT_LIMIT - _data.size())
	{
		cancel(BAD_GATEWAY);
		return;
	}

	_data.append(buffer, count);
	checkOutputHeaders();
}

void Cgi::checkOutputHeaders()
{
	if (_headersDone)
		return;

	size_t end = _data.find("\r\n\r\n");
	size_t lf = _data.find("\n\n");
	if (lf != string::npos && (end == string::npos || lf < end))
		end = lf;

	_headersDone = end != string::npos;
	size_t headerSize = _headersDone ? end : _data.size();
	if (headerSize > CGI_HEADER_LIMIT)
		cancel(BAD_GATEWAY);
}

void Cgi::closeInput()
{
	if (_input >= 0)
		close(_input);
	_input = -1;
	vector<char>().swap(_body);
}

void Cgi::closeOutput()
{
	if (_output >= 0)
		close(_output);
	_output = -1;
	_eof = true;
}

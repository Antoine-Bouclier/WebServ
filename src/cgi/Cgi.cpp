#include <cerrno>
#include <stdexcept>
#include <sys/wait.h>
#include "cgi/Cgi.hpp"

Cgi::Cgi()
	: _client(-1),
	_input(-1),
	_output(-1),
	_pid(-1),
	_status(0),
	_eof(false),
	_headersDone(false),
	_error(OK),
	_started(std::time(NULL)),
	_offset(0)
{}

Cgi::Cgi(const Cgi& other)
	: _client(-1),
	_input(-1),
	_output(-1),
	_pid(-1),
	_status(0),
	_eof(false),
	_headersDone(false),
	_error(OK),
	_started(std::time(NULL)),
	_offset(0)
{
	*this = other;
}

Cgi& Cgi::operator=(const Cgi& other)
{
	if (this == &other)
		return (*this);
	if (_client >= 0 || _input >= 0 || _output >= 0 || _pid >= 0)
		throw std::logic_error("Cannot assign to an active CGI");
	if (other._client >= 0 || other._input >= 0 || other._output >= 0 || other._pid >= 0)
		throw std::logic_error("Cannot copy an active CGI");

	std::vector<char> body(other._body);
	std::string data(other._data);
	_body.swap(body);
	_data.swap(data);
	_status = other._status;
	_eof = other._eof;
	_headersDone = other._headersDone;
	_error = other._error;
	_started = other._started;
	_offset = other._offset;
	return (*this);
}

Cgi::~Cgi()
{
	cancel(_error);
	if (_pid > 0)
	{
		while (waitpid(_pid, &_status, 0) < 0)
		{
			if (errno != EINTR)
				break;
		}
	}
}

int Cgi::client() const
{
	return (_client);
}

int Cgi::input() const
{
	return (_input);
}

int Cgi::output() const
{
	return (_output);
}

HttpStatusCode Cgi::error() const
{
	return (_error);
}

bool Cgi::finished() const
{
	return (_error != OK || (_eof && _pid < 0));
}

void Cgi::detach()
{
	_client = -1;
}

pid_t Cgi::pid() const
{
	return (_pid);
}

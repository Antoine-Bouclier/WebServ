#include "server/Client.hpp"

using std::string;

Client::Client() :
	_fd(-1),
	_listener_fd(-1),
	_last_activity(std::time(NULL)),
	_request_started(_last_activity),
	_file_offset(0),
	_file_remaining(0)
{}

Client::Client(int clientFd, int listenerFd) :
	_fd(clientFd),
	_listener_fd(listenerFd),
	_last_activity(std::time(NULL)),
	_request_started(_last_activity),
	_file_offset(0),
	_file_remaining(0)
{}

Client::Client(const Client& other) :
	_fd(other._fd),
	_request(other._request),
	_listener_fd(other._listener_fd),
	_writeBuffer(other._writeBuffer),
	_last_activity(other._last_activity),
	_request_started(other._request_started),
	_file_offset(0),
	_file_remaining(0)
{
	*this = other;
}

Client::~Client() {}

Client&	Client::operator=(const Client& other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_request = other._request;
		_listener_fd = other._listener_fd;
		_writeBuffer = other._writeBuffer;
		_last_activity = other._last_activity;
		_request_started = other._request_started;
		_file.close();
		_file.clear();
		_file_path = other._file_path;
		_file_offset = other._file_offset;
		_file_remaining = other._file_remaining;
		if (_file_remaining)
		{
			_file.open(_file_path.c_str(), std::ios::binary);
			_file.seekg(_file_offset);
			if (!_file) throw std::runtime_error("Cannot copy file response");
		}
	}
	return (*this);
}

int					Client::getFd() const { return (_fd); }
int					Client::getListenerFd() const { return (_listener_fd); }
HttpRequest&		Client::getRequest() { return (_request); }
const HttpRequest&	Client::getRequest() const { return (_request); }

const string&		Client::getWriteBuffer() const { return (_writeBuffer); }
void				Client::appendWriteBuffer(const std::string& data) { _writeBuffer.append(data); }
void				Client::clearWriteBuffer() { _writeBuffer.clear(); }
bool				Client::hasPendingWrite() const { return (!_writeBuffer.empty() || _file_remaining > 0); }
void				Client::consumeWriteBuffer(size_t bytes) { _writeBuffer.erase(0, bytes); }

void Client::touch() { _last_activity = std::time(NULL); }

bool Client::hasTimedOut(std::time_t now) const { return (idleSeconds(now) >= CLIENT_IDLE_TIMEOUT); }

bool Client::requestTimedOut(std::time_t now) const
{
    HttpParseState state = _request.getState();

    return (state != STATE_READY && state != STATE_ERROR && std::difftime(now, _request_started) >= CLIENT_REQUEST_TIMEOUT);
}

double Client::idleSeconds(std::time_t now) const { return (std::difftime(now, _last_activity)); }

void Client::setResponse(const HttpResponse& response)
{
	_file.close();
	_file.clear();
	_file_path = response.getFilePath();
	_file_offset = 0;
	_file_remaining = response.getFileSize();
	if (!_file_path.empty())
	{
		_file.open(_file_path.c_str(), std::ios::binary);
		if (!_file) throw std::runtime_error("Cannot open response file");
	}
	_writeBuffer = response.serialize();
	_request.releaseBody();
}

bool Client::fillWriteBuffer()
{
	if (!_writeBuffer.empty() || !_file_remaining)
		return (true);
	char data[FILE_BUFFER_SIZE];
	std::streamsize count = static_cast<std::streamsize>(std::min(_file_remaining, static_cast<std::streamoff>(sizeof(data))));
	if (!_file.read(data, count))
		return (false);
	_writeBuffer.assign(data, count);
	_file_offset += count;
	_file_remaining -= count;
	return (true);
}

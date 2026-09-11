#include "server/Client.hpp"

using std::string;

Client::Client() :
	_fd(-1),
	_listener_fd(-1),
	_last_activity(std::time(NULL)),
	_request_started(_last_activity)
{}

Client::Client(int clientFd, int listenerFd) :
	_fd(clientFd),
	_listener_fd(listenerFd),
	_last_activity(std::time(NULL)),
	_request_started(_last_activity)
{}

Client::Client(const Client& other) :
	_fd(other._fd),
	_request(other._request),
	_listener_fd(other._listener_fd),
	_writeBuffer(other._writeBuffer),
	_last_activity(other._last_activity),
	_request_started(other._request_started)
{}

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
bool				Client::hasPendingWrite() const { return (!_writeBuffer.empty()); }
void				Client::consumeWriteBuffer(size_t bytes) { _writeBuffer.erase(0, bytes); }

void Client::touch() { _last_activity = std::time(NULL); }

bool Client::hasTimedOut(std::time_t now) const { return (idleSeconds(now) >= 30); }

bool Client::requestTimedOut(std::time_t now) const
{
    HttpParseState state = _request.getState();

    return (state != STATE_READY && state != STATE_ERROR && std::difftime(now, _request_started) >= 120);
}

double Client::idleSeconds(std::time_t now) const { return (std::difftime(now, _last_activity)); }

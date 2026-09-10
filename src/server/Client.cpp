#include "server/Client.hpp"

using std::string;

Client::Client() :
	_fd(-1),
	_listener_fd(-1)
{}

Client::Client(int clientFd, int listenerFd) : _fd(clientFd), _listener_fd(listenerFd) {}

Client::Client(const Client& other) :
	_fd(other._fd),
	_listener_fd(other._listener_fd),
	_request(other._request),
	_writeBuffer(other._writeBuffer)
{}

Client::~Client() {}

Client&	Client::operator=(const Client& other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_listener_fd = other._listener_fd;
		_request = other._request;
		_writeBuffer = other._writeBuffer;
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

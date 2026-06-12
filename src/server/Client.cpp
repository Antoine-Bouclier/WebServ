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
	_readBuffer(other._readBuffer),
	_writeBuffer(other._writeBuffer)
{}

Client::~Client() {}

Client&	Client::operator=(const Client& other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_listener_fd = other._listener_fd;
		_readBuffer = other._readBuffer;
		_writeBuffer = other._writeBuffer;
	}
	return (*this);
}

void	Client::appendReadBuffer(const char* data, size_t size)
{
	_readBuffer.append(data, size);
}

const string&	Client::getReadBuffer() const { return (_readBuffer); }

bool Client::hasCompleteHeader() const
{
	return (_readBuffer.find("\r\n\r\n") != string::npos);
}

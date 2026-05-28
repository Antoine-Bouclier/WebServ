#include "server/Client.hpp"

Client::Client() :
	_fd(-1),
	_listener_fd(-1)
{}

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
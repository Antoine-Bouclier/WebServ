#include "server/Listener.hpp"

using std::string;

/***************************
 *                         *
 * -- CLASS DECLARATION -- *
 *                         *
 ***************************/

Listener::Listener() :
	_fd(-1),
	_port(-1),
	_server(NULL)
{}

Listener::Listener(const Listener& other) :
	_fd(other._fd),
	_port(other._port),
	_host(other._host),
	_server(other._server)
{}

Listener::Listener(const ConfigServer& server) :
	_fd(-1),
	_port(server.getPort()),
	_host(server.getHost()),
	_server(&server)
{}


Listener::~Listener() {}

Listener&	Listener::operator=(const Listener& other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_port = other._port;
		_host = other._host;
		_server = other._server;
	}
	return (*this);
}

void	Listener::setFd(int fd) { _fd = fd; }

int		Listener::getFd() const { return (_fd); }
int		Listener::getPort() const { return (_port); }
string	Listener::getHost() const { return (_host); }

const ConfigServer& Listener::getServer() const { return (*_server); }

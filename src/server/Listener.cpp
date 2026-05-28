#include "server/Listener.hpp"

using std::vector;
using std::string;

Listener*	getListener(string& host, int& port, vector<Listener>& list)
{
	vector<Listener>::iterator current = list.begin();
	for (; current != list.end(); current++)
	{
		if (host == current->getHost() && port == current->getPort())
			return (&(*current));
	}
	return (NULL);
}

/***************************
 *                         *
 * -- CLASS DECLARATION -- *
 *                         *
 ***************************/

Listener::Listener() :
	_fd(-1),
	_port(-1)
{}

Listener::Listener(const Listener& other) :
	_fd(other._fd),
	_port(other._port),
	_host(other._host),
	_servers(vector<const ConfigServer*>(other._servers))
{}

Listener::Listener(const ConfigServer& server) :
	_port(server.getPort()),
	_host(server.getHost())
{
	_servers.push_back(&server);
}


Listener::~Listener() {}

Listener&	Listener::operator=(const Listener& other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_port = other._port;
		_host = other._host;
		_servers = other._servers;
	}
	return (*this);
}

int		Listener::getPort() const { return (_port); }
string	Listener::getHost() const { return (_host); }

void	Listener::addServer(ConfigServer& server)
{
	_servers.push_back(&server);
}

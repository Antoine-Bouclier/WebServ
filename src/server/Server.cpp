#include "server/Server.hpp"

using std::map;
using std::vector;
using std::string;

static void	setupListeningSockets(vector<ConfigServer>& servers, vector<Listener>& listeners)
{
	vector<ConfigServer>::iterator it = servers.begin();
	for (; it != servers.end(); it++)
	{
		int		port = it->getPort();
		string	host = it->getHost();

		Listener* exists = getListener(host, port, listeners);
		if (exists)
		{
			exists->addServer(*it);
			continue;
		}

		Listener	newListener = Listener(*it);
		listeners.push_back(newListener);
	}
}

// bool	run(Server& ServerContext)
// {

// }

/***************************
 *                         *
 * -- CLASS DECLARATION -- *
 *                         *
 ***************************/

bool	Server::setupServer()
{
	setupListeningSockets(_servers, _listeners);
	return (true);
}

Server::Server() {}

Server::Server(const vector<ConfigServer>& servers) :
	_servers(servers)
{}

Server::Server(const Server& other) :
	_clients(other._clients),
	_servers(other._servers),
	_poll_fds(other._poll_fds),
	_listeners(other._listeners)
{}

Server::~Server() {}

Server&	Server::operator=(const Server& other)
{
	if (this != &other)
	{
		_clients = other._clients;
		_servers = other._servers;
		_poll_fds = other._poll_fds;
		_listeners = other._listeners;
	}

	return (*this);
}
#include <unistd.h>
#include <iostream>
#include "server/Socket.hpp"
#include "server/Server.hpp"

using std::map;
using std::vector;
using std::string;

static int createListeningSocket(const std::string& host, int port)
{
	int fd = createSocket();
	if (fd == -1)
		throw std::runtime_error("socket failed");

	try
	{
		setReuseAddr(fd);
		setNonBlocking(fd);

		if (bindSocket(fd, host, port) == -1)
			throw std::runtime_error("bind failed on " + host);

		if (listen(fd, LISTEN_BACKLOG) == -1)
			throw std::runtime_error("listen failed");
	}
	catch (...)
	{
		close(fd);
		throw;
	}

	return (fd);
}

/***********************
 *                     *
 * -- CLASS METHODS -- *
 *                     *
 ***********************/

void	Server::setupServer()
{
	vector<ConfigServer>::iterator it = _servers.begin();
	for (; it != _servers.end(); it++)
	{
		int		port = it->getPort();
		string	host = it->getHost();

		Listener* exists = getListener(host, port, _listeners);
		if (exists)
		{
			exists->addServer(*it);
			continue;
		}

		Listener	newListener = Listener(*it);

		int fd = createListeningSocket(host, port);
		if (fd <= 0)
			throw std::runtime_error("Failed to create listening socket fd");
		newListener.setFd(fd);
		if (!addPollFd(fd, POLLIN))
			throw std::runtime_error("Failed to register listening fd into poll");
		_listeners.push_back(newListener);
	}
}

bool	Server::addPollFd(int fd, short events)
{
	if (fd <= 0)
		return (false);

	pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_poll_fds.push_back(pfd);

	return (true);
}

/***************************
 *                         *
 * -- CLASS DECLARATION -- *
 *                         *
 ***************************/

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

Server::~Server()
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
		if (_poll_fds[i].fd >= 0)
			close(_poll_fds[i].fd);
	}
	_poll_fds.clear();
	_clients.clear();
	_listeners.clear();
}

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

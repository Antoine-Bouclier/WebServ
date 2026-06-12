#include <errno.h>
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

bool	Server::isListenerFd(int fd) const
{
	vector<Listener>::const_iterator	it = _listeners.begin();
	for (; it != _listeners.end(); it++)
	{
		if (fd == it->getFd())
			return (true);
	}
	return (false);
}

void Server::handleClientConnection(int listenerFd)
{
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);

    int clientFd = accept(listenerFd, reinterpret_cast<sockaddr*>(&addr), &addrLen);
    if (clientFd == -1)
        return;

    setNonBlocking(clientFd);

    Client client(clientFd, listenerFd);
    _clients[clientFd] = client;

    addPollFd(clientFd, POLLIN);
}

void Server::closeClient(int fd)
{
	close(fd);
    _clients.erase(fd);

	vector<pollfd>::iterator it = _poll_fds.begin();
    for (; it != _poll_fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _poll_fds.erase(it);
            break;
        }
    }
}

void Server::handleClientRead(int clientFd)
{
    char buffer[4096];

    ssize_t bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes == 0)
    {
        std::cout << "Client closed connection: " << clientFd << std::endl;
        closeClient(clientFd);
        return;
    }

    if (bytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        std::cout << "recv failed on client: " << clientFd << std::endl;
        closeClient(clientFd);
        return;
    }
    buffer[bytes] = '\0';

	_clients[clientFd].appendReadBuffer(buffer, bytes);

	if (_clients[clientFd].hasCompleteHeader())
	{
		std::cout << "Complete HTTP headers from client " << clientFd << ":\n";
		std::cout << _clients[clientFd].getReadBuffer() << std::endl;

		closeClient(clientFd);
	}
}

void	Server::run()
{
	if (_poll_fds.empty())
		throw std::runtime_error("No file descriptor had been added to poll");

	_isAlive = true;
	while (_isAlive)
	{
		int ready = poll(&_poll_fds[0], _poll_fds.size(), -1);

		if (ready == -1)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("Error while getting fds with poll");
		}

		for (size_t i = 0; i < _poll_fds.size(); i++)
		{
			if (_poll_fds[i].revents == 0) // Le fd n'est pas prêt
				continue;
			else if (_poll_fds[i].revents & POLLIN)
			{
				int fd = _poll_fds[i].fd;

				if (isListenerFd(fd))
					handleClientConnection(fd);
				else
					handleClientRead(fd);
			}
		}
	}
}

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

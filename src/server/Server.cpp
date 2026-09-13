#include <errno.h>
#include <unistd.h>
#include <iostream>
#include "server/Socket.hpp"
#include "server/Server.hpp"

using std::map;
using std::cout;
using std::vector;
using std::string;

static int createListeningSocket(const std::string& host, int port)
{
	int fd = createSocket();
	if (fd == -1)
		throw std::runtime_error("socket failed");

	try
	{
		if (!setReuseAddr(fd))
			throw std::runtime_error("setsockopt failed");
		if (!setNonBlocking(fd))
			throw std::runtime_error("fcntl failed");

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
 *					   *
 * -- CLASS METHODS -- *
 *					   *
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

	if (_clients.size() >= MAX_CLIENTS || !setNonBlocking(clientFd))
	{
		close(clientFd);
		return;
	}

	try
	{
		_clients[clientFd] = Client(clientFd, listenerFd);
		if (!addPollFd(clientFd, POLLIN))
			closeClient(clientFd);
	}
	catch (...)
	{
		closeClient(clientFd);
	}
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

void Server::closeTimedOutClients()
{
    std::time_t now = std::time(NULL);
    map<int, Client>::iterator it = _clients.begin();

    while (it != _clients.end())
    {
        int fd = it->first;
        bool requestExpired = it->second.requestTimedOut(now);
        bool idleExpired = it->second.hasTimedOut(now);
        ++it;

        if (requestExpired || idleExpired)
        {
            closeClient(fd);
        }
    }
}

void Server::handleClientRead(int clientFd)
{
	char buffer[4096];

	ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytes == 0)
	{
		closeClient(clientFd);
		return;
	}

	if (bytes < 0)
	{
		closeClient(clientFd);
		return;
	}

	_clients[clientFd].touch();

	processClientRequest(clientFd, buffer, bytes);
}

void Server::handleClientWrite(int clientFd)
{
	Client& client = _clients[clientFd];

	if (!client.hasPendingWrite() || !client.fillWriteBuffer())
	{
		closeClient(clientFd);
		return;
	}

	const string& buffer = client.getWriteBuffer();
	ssize_t bytes = send(clientFd, buffer.c_str(), buffer.size(), 0);

	if (bytes <= 0)
	{
		closeClient(clientFd);
		return;
	}

	client.touch();

	client.consumeWriteBuffer(static_cast<size_t>(bytes));

	if (!client.hasPendingWrite())
		closeClient(clientFd);
}

void	Server::run()
{
	if (_poll_fds.empty())
		throw std::runtime_error("No file descriptor had been added to poll");

	_isAlive = true;
	while (_isAlive)
	{
		int ready = poll(&_poll_fds[0], _poll_fds.size(), 1000);

		if (ready == -1)
		{
			if (errno == EINTR)
			{
				closeTimedOutClients();
				continue;
			}
			throw std::runtime_error("Error while getting fds with poll");
		}

		for (int i = 0; i < static_cast<int>(_poll_fds.size()); ++i)
		{
			int fd = _poll_fds[i].fd;
			short revents = _poll_fds[i].revents;

			if (!revents)
				continue;

			if (isListenerFd(fd))
			{
				if (revents & (POLLERR | POLLHUP | POLLNVAL))
					throw std::runtime_error("Listening socket failed");
				if (revents & POLLIN)
					handleClientConnection(fd);
				continue;
			}

			try
			{
				if (revents & (POLLERR | POLLNVAL)) closeClient(fd);
				else if (revents & POLLOUT) handleClientWrite(fd);
				else if (revents & POLLIN) handleClientRead(fd);
				else if (revents & POLLHUP) closeClient(fd);
			}
			catch (...)
			{
				closeClient(fd);
			}

			if (_clients.find(fd) == _clients.end())
				--i;
		}

		closeTimedOutClients();
	}
}

void	Server::setupServer()
{
	vector<ConfigServer>::iterator it = _servers.begin();
	for (; it != _servers.end(); it++)
	{
		int		port = it->getPort();
		string	host = it->getHost();

		Listener	newListener = Listener(*it);

		int fd = createListeningSocket(host, port);
		try
		{
			newListener.setFd(fd);
			if (!addPollFd(fd, POLLIN))
				throw std::runtime_error("Failed to register listening fd into poll");
			_listeners.push_back(newListener);
		}
		catch (...)
		{
			closeClient(fd);
			throw;
		}
	}
}

bool	Server::addPollFd(int fd, short events)
{
	if (fd < 0)
		return (false);

	pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	_poll_fds.push_back(pfd);

	return (true);
}

bool Server::setPollEvents(int fd, short events)
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
		if (_poll_fds[i].fd == fd)
		{
			_poll_fds[i].events = events;
			return (true);
		}
	}
	return (false);
}

const ConfigServer& Server::getServerConfig(int listenerFd) const
{
	for (size_t i = 0; i < _listeners.size(); ++i)
	{
		if (_listeners[i].getFd() == listenerFd)
			return (_listeners[i].getServer());
	}
	throw std::runtime_error("Unknown listener fd");
}

void Server::processClientRequest(int clientFd, const char* buffer, ssize_t bytes)
{
	Client&				client = _clients[clientFd];
	HttpRequest&		request = client.getRequest();
	const ConfigServer&	config = getServerConfig(client.getListenerFd());

	request.feed(buffer, bytes);
	if (request.getState() == STATE_HEADERS_DONE)
	{
		Router					router;
		const AConfig*			effectiveConfig = &config;
		const ConfigLocation*	location = router.matchLocation(config, request.getPath());

		if (location)
			effectiveConfig = location;
		request.startBody(*effectiveConfig);
	}

	if (request.getState() == STATE_ERROR)
	{
		
		HttpResponse	response = RequestHandler::buildErrorResponse(request.getStatusCode(), Router().matchLocation(config, request.getPath()), &config);

		client.setResponse(response);
		setPollEvents(clientFd, POLLOUT);
		return;
	}

	if (request.getState() == STATE_READY)
	{

		Router					router;
		const ConfigLocation*	matchedLocation = router.matchLocation(config, request.getPath());


		HttpResponse response = RequestHandler::handle(request, matchedLocation, &config);

		client.setResponse(response);
		
		setPollEvents(clientFd, POLLOUT);
	}
}

/***************************
 *						 *
 * -- CLASS DECLARATION -- *
 *						 *
 ***************************/

Server::Server() : _isAlive(false) {}

Server::Server(const vector<ConfigServer>& servers) : _servers(servers), _isAlive(false) {}

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

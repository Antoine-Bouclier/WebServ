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

	ssize_t bytes = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytes == 0)
	{
		std::cout << "[Server] Client disconnected: " << clientFd << std::endl;
		closeClient(clientFd);
		return;
	}

	if (bytes < 0)
	{
		std::cout << "[Server] Client disconnected or recv failed: " << clientFd << std::endl;
		closeClient(clientFd);
		return;
	}

	processClientRequest(clientFd, buffer, bytes);
}

void Server::handleClientWrite(int clientFd)
{
	Client& client = _clients[clientFd];

	if (!client.hasPendingWrite())
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
		int ready = poll(&_poll_fds[0], _poll_fds.size(), -1);

		if (ready == -1)
		{
			if (errno == EINTR)
				continue;
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
				if (revents & POLLIN)
					handleClientConnection(fd);
				continue;
			}

			if (revents & (POLLERR | POLLHUP | POLLNVAL))
				closeClient(fd);
			else if (revents & POLLOUT)
				handleClientWrite(fd);
			else if (revents & POLLIN)
				handleClientRead(fd);

			if (_clients.find(fd) == _clients.end())
				--i;
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

void Server::processClientRequest(int clientFd, const char* buffer, ssize_t bytes)
{
	Client&				client = _clients[clientFd];
	HttpRequest&		request = client.getRequest();
	const ConfigServer&	defaultConfig = _servers[0]; 

	request.feed(buffer, bytes, defaultConfig, defaultConfig.getClientMaxBody());

	if (request.getState() == STATE_ERROR)
	{
		std::cout << "[Server] HTTP Parsing Error on client " << clientFd << "!\n";
		
		HttpResponse	response;
		string			body = "400 Bad Request";

		response.setStatus(BAD_REQUEST);
		response.addHeader("Content-Type", "text/plain");
		response.setBody(vector<char>(body.begin(), body.end()));

		client.appendWriteBuffer(response.serialize());
		setPollEvents(clientFd, POLLOUT);
		return;
	}

	if (request.getState() == STATE_READY)
	{
		std::cout << "[Server] Request successfully received from client " << clientFd << "!\n";
		std::cout << "Method: " << request.getMethod() 
				  << " | Path: " << request.getPath() 
				  << " | Version: " << request.getVersion() << std::endl;

		Router router;
		const ConfigServer& matchedServer = router.matchServer(_servers, request);
		const ConfigLocation* matchedLocation = router.matchLocation(matchedServer, request.getPath());

		std::cout << "Matched Server Port: " << matchedServer.getPort() << std::endl;
		if (matchedLocation)
			std::cout << "Matched Location: " << matchedLocation->getPath() << std::endl;

		HttpResponse response = RequestHandler::handle(request, matchedLocation, &matchedServer);

		client.appendWriteBuffer(response.serialize());
		
		setPollEvents(clientFd, POLLOUT);
	}
}

/***************************
 *						 *
 * -- CLASS DECLARATION -- *
 *						 *
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

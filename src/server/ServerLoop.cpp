#include <cerrno>
#include <new>
#include "server/Server.hpp"

void Server::run()
{
	if (_poll_fds.empty())
		throw std::runtime_error("No file descriptor had been added to poll");

	_isAlive = true;
	std::vector<pollfd> polledFds;

	while (_isAlive)
	{
		int ready = poll(&_poll_fds[0], _poll_fds.size(), 1000);
		if (ready < 0)
			handlePollError(errno);

		if (ready > 0 && copyPolledFds(polledFds))
		{
			for (size_t i = 0; i < polledFds.size(); ++i)
				handleEvent(polledFds[i]);
		}
		updateCgi();
		closeTimedOutClients();
	}
}

void Server::handlePollError(int error)
{
	switch (error)
	{
		case EINTR:			// A signal interrupted the wait.
			return;
		case ENOMEM:		// The kernel could not allocate memory for poll.
			if (!_clients.empty())
				closeClient(_clients.begin()->first);
			return;
		case EINVAL:		// The descriptor count exceeds RLIMIT_NOFILE on Linux.
			throw std::runtime_error("poll: too many descriptors");
		case EFAULT:		// The descriptor array points to inaccessible memory.
			throw std::runtime_error("poll: invalid descriptor array");
		default:
			throw std::runtime_error("poll failed");
	}
}

bool Server::copyPolledFds(std::vector<pollfd>& polledFds)
{
	try
	{
		polledFds = _poll_fds;
		return (true);
	}
	catch (const std::bad_alloc&)
	{
		if (!_clients.empty())
			closeClient(_clients.begin()->first);
		return (false);
	}
}

void Server::handleEvent(const pollfd& event)
{
	int		fd = event.fd;
	short	revents = event.revents;

	if (!revents || !hasPollEvent(fd, revents))
		return;
	if (isListenerFd(fd))
	{
		if (revents & (POLLERR | POLLHUP | POLLNVAL))
			throw std::runtime_error("Listening socket failed");
		if (revents & POLLIN)
			handleClientConnection(fd);
		return;
	}
	if (handleCgiEvent(fd, revents))
		return;
	if (_clients.find(fd) == _clients.end())
		return;

	try
	{
		if (revents & (POLLERR | POLLNVAL))
			closeClient(fd);
		else if (revents & POLLOUT)
			handleClientWrite(fd);
		else if (revents & POLLIN)
			handleClientRead(fd);
		else if (revents & POLLHUP)
			closeClient(fd);
	}
	catch (...)
	{
		closeClient(fd);
	}
}

bool Server::hasPollEvent(int fd, short events) const
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
		if (_poll_fds[i].fd == fd)
			return (_poll_fds[i].revents == events);
	}
	return (false);
}

void Server::removePollFd(int fd)
{
	for (size_t i = 0; i < _poll_fds.size(); ++i)
	{
		if (_poll_fds[i].fd != fd)
			continue;
		_poll_fds.erase(_poll_fds.begin() + i);
		return;
	}
}

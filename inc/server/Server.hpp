#ifndef SERVER_HPP
# define SERVER_HPP

/* -- Includes -- */
#include <map>
#include <vector>
#include <poll.h>
#include <netinet/in.h>

#include "server/Client.hpp"
#include "server/Listener.hpp"
#include "config/ConfigServer.hpp"

/* -- Config -- */
#define LISTEN_BACKLOG 128

/* -- Class -- */
class	Server
{
public:
	Server();
	Server(const Server&);
	Server(const std::vector<ConfigServer>& servers);

	~Server();

	Server&	operator=(const Server&);

	void	run(void);
	void	setupServer(void);
	bool	isListenerFd(int fd) const;
	bool	addPollFd(int fd, short events);

	void	closeClient(int fd);
	void	handleClientRead(int clientFd);
	void	handleClientConnection(int listenerFd);

private:
	std::map<int, Client>		_clients;		
	std::vector<ConfigServer>	_servers;
	std::vector<pollfd>			_poll_fds;
	std::vector<Listener>		_listeners;

	bool						_isAlive;
};

#endif
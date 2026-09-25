#ifndef SERVER_HPP
# define SERVER_HPP

/* -- Includes -- */
#include <map>
#include "cgi/Cgi.hpp"
#include <vector>
#include <poll.h>
#include <netinet/in.h>

#include "http/Router.hpp"
#include "server/Client.hpp"
#include "server/Listener.hpp"
#include "http/HttpResponse.hpp"
#include "config/ConfigServer.hpp"
#include "http/RequestHandler.hpp"

/* -- Config -- */
#define LISTEN_BACKLOG 128
#define MAX_CLIENTS 256

/* -- Class -- */
class	Server
{
public:
	Server();
	Server(const std::vector<ConfigServer>& servers);

	~Server();


	void	run(void);
	void	setupServer(void);
	bool	isListenerFd(int fd) const;
	bool	addPollFd(int fd, short events);
	bool	setPollEvents(int fd, short events);

	void	closeClient(int fd);
	void	closeTimedOutClients();
	void	handleClientRead(int clientFd);
	void	handleClientWrite(int clientFd);
	void	handleClientConnection(int listenerFd);

private:
	Server(const Server&);
	Server& operator=(const Server&);
	const ConfigServer& getServerConfig(int listenerFd) const;
	std::map<int, Client>		_clients;		
	std::vector<ConfigServer>	_servers;
	std::vector<pollfd>			_poll_fds;
	std::vector<Listener>		_listeners;

	bool						_isAlive;
	std::vector<Cgi*> _cgis;

	bool hasCgi(int client) const;
	void syncCgi(Cgi& cgi, int input, int output);
	void stopCgi(int client);
	void startCgi(int clientFd, const HttpResponse& target, const ConfigServer& config);
	bool handleCgiEvent(int fd, short events);
	void updateCgi();
	void handlePollError(int error);
	bool copyPolledFds(std::vector<pollfd>& polledFds);
	void handleEvent(const pollfd& event);
	bool hasPollEvent(int fd, short events) const;
	void removePollFd(int fd);
	void processClientRequest(int clientFd, const char* buffer, ssize_t bytes);
};

#endif
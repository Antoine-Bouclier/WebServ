#ifndef LISTENER_HPP
# define LISTENER_HPP

/* -- Includes -- */
#include <string>
#include <vector>
#include "config/ConfigServer.hpp"

/* -- Class -- */
class	Listener
{
public:
	Listener();
	Listener(const Listener&);
	Listener(const ConfigServer& server);

	~Listener();

	Listener&	operator=(const Listener&);

	void		setFd(int fd);

	int			getFd(void) const;
	int			getPort(void) const;
	std::string	getHost(void) const;

	void		addServer(const ConfigServer& server);

private:
	int									_fd;
	int									_port;
	std::string							_host;
	std::vector<const ConfigServer*>	_servers;
};

/* -- Methods -- */
Listener*	getListener(const std::string& host, const int& port, std::vector<Listener>& list);

#endif
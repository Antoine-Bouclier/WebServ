#ifndef LISTENER_HPP
# define LISTENER_HPP

/* -- Includes -- */
#include <string>
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

	const ConfigServer& getServer() const;

private:
	int									_fd;
	int									_port;
	std::string							_host;
	const ConfigServer* _server;
};


#endif
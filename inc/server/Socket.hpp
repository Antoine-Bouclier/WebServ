#ifndef SOCKET_HPP
# define SOCKET_HPP

#include <string>
#include <netinet/in.h>	// in_addr_t

int			createSocket(void);
void		setReuseAddr(int fd);
void		setNonBlocking(int fd);
in_addr_t	resolveHost(const std::string& host);
int			bindSocket(int fd, const std::string& host, int port);

#endif
#include <string>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>		// getaddrinfo
#include <cstring>		// std::memset
#include <unistd.h>
#include <iostream>
#include <stdexcept>	// std::runtime_error
#include <sys/socket.h>

in_addr_t resolveHost(const std::string& host)
{
	struct addrinfo		hints;
	struct addrinfo*	res = NULL;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int ret = getaddrinfo(host.c_str(), NULL, &hints, &res);
	if (ret != 0)
		throw std::runtime_error(gai_strerror(ret)); // Error output for getaddrinfo

	in_addr_t addr = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;

	freeaddrinfo(res);
	return (addr);
}

int	bindSocket(int fd, const std::string& host, int port)
{
	sockaddr_in addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = resolveHost(host);
	
	if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
		return (-1);
	return (0);
}

/*
fcntl = file descriptor control

Params:
	fd: file descriptor to configure
	command: operation to perform on the file descriptor
	...: optional argument required by some commands

F_GETFL: retrieve the current file status flags
F_SETFL: update the file status flags
O_NONBLOCK: make I/O operations non-blocking

Used to prevent accept(), recv() and send() from blocking the server.
*/
void	setNonBlocking(int fd)
{
	// int current_fd_flags = fcntl(fd, F_GETFL, 0);
	// fcntl(fd, F_SETFL, current_fd_flags | O_NONBLOCK);
	fcntl(fd, F_SETFL, O_NONBLOCK);
}

/*
setsockopt = set socket option

Params:
	fd: socket file descriptor
	level: specifies which protocol layer handles the option
	optname: option to configure
	optval: value assigned to the option
	optlen: size of optval in bytes

SOL_SOCKET: option is handled at the socket level
SO_REUSEADDR: allow reusing a local address/port immediately after restarting the server
*/
void	setReuseAddr(int fd)
{
	if (fd <= 0)
		return;
	int	active = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &active, sizeof(active));
}

/*
socket = create a communication endpoint

Params:
	domain: communication domain (address family)
	type: socket type
	protocol: protocol to use (0 = default for the given type)

AF_INET: IPv4 address family
SOCK_STREAM: TCP socket
0: use the default protocol for SOCK_STREAM (TCP)
*/
int createSocket()
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		std::cerr << "socket creation failed: " << strerror(errno) << "\n";
		return (-1);
	}

	return (socket_fd);
}



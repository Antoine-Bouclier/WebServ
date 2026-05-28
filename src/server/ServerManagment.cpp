#include <netdb.h>		// getaddrinfo
#include <cstring>		// std::memset
#include <unistd.h>
#include <iostream>
#include <stdexcept>	// std::runtime_error
#include <sys/socket.h>

#include "server/Server.hpp"

using std::vector;

in_addr_t resolveHost(const std::string& host)
{
	struct addrinfo hints;
	struct addrinfo* res = NULL;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int ret = getaddrinfo(host.c_str(), NULL, &hints, &res);
	if (ret != 0)
		throw std::runtime_error(gai_strerror(ret)); // Error output for getaddrinfo

	in_addr_t addr = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;

	freeaddrinfo(res);
	return addr;
}

/*
sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server.getPort());
	try {
		addr.sin_addr.s_addr = resolveHost(server.getHost());
	} catch (const std::exception& e) {
		std::cout << e.what() << "\n";
		close(socket_fd);
		return (-1);
	}
*/

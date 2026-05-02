#ifndef SERVER_HPP
# define SERVER_HPP

/* -- Includes -- */
#include <netdb.h>			// getaddrinfo
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "WebServ.hpp"
#include "ConfigParser.hpp"

int start_server(const ConfigServer& server);
in_addr_t resolveHost(const std::string& host);


#endif
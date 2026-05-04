#ifndef SERVER_HPP
# define SERVER_HPP

/* -- Includes -- */
#include <netinet/in.h>

#include "config/ConfigServer.hpp"

int start_server(const ConfigServer& server);
in_addr_t resolveHost(const std::string& host);


#endif
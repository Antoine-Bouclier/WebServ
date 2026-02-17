#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h> // sockadrr_in
#include <sys/socket.h> // socket(), bind(), accept(), listen()
#include <unistd.h> // close()

#include <iostream>
#include <cstring> // memset
#include <map>
#include <poll.h>
#include <vector>

#include "Client.hpp"

class Server
{
	private:
		int			_server_fd;
		unsigned	int	_max_size_client;
		
		std::vector<pollfd>		_pollFds;
		std::map<int, Client*>	_clients;

		void	addNewPollfd(int fd);
		void	handleMessage(size_t *index);
	public:
		Server();
		~Server();
		void	run();
};

#endif
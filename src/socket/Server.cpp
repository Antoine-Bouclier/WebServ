#include "Server.hpp"

Server::Server()
{
	
}

void	Server::addNewPollfd(int fd)
{
	pollfd	newPoll;

	newPoll.fd = fd;
	newPoll.events = POLLIN;
	_pollFds.push_back(newPoll);
}

void	Server::handleMessage(size_t *index)
{
	char	buffer[1024];

	memset(buffer, 0, sizeof(buffer));
	int	bytes = recv(_pollFds[*index].fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
	{
		close(_pollFds[*index].fd);
		_pollFds.erase(_pollFds.begin() + *index);
		*index--;
	}
	else
		std::cout << "Message: " << buffer << std::endl;
}

void	Server::run()
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in	server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8080);

	bind(_server_fd, (struct sockaddr*)&server_address, sizeof(server_address));
	listen(_server_fd, 5);
	addNewPollfd(_server_fd);

	while (1)
	{
		poll(_pollFds.data(), _pollFds.size(), -1);
		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			if (_pollFds[i].revents & _server_fd)
				addNewPollfd(accept(_server_fd, NULL, NULL));
			else
				handleMessage(&i);
		}
	}
	close(_server_fd);
}

Server::~Server()
{

}
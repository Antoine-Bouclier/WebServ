#ifndef CLIENT_HPP
# define CLIENT_HPP

/* -- Includes -- */
#include <string>

/* -- Class -- */
class	Client
{
public:
	Client();
	Client(const Client&);

	~Client();

	Client&	operator=(const Client&);

private:
	int			_fd;
	int			_listener_fd;
	std::string	_readBuffer;
	std::string	_writeBuffer;
};

#endif
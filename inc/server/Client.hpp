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
	Client(int clientFd, int listenerFd);

	~Client();

	Client&	operator=(const Client&);

	const std::string&	getReadBuffer(void) const;
	bool				hasCompleteHeader(void) const;
	void				appendReadBuffer(const char* data, size_t size);

private:
	int			_fd;
	int			_listener_fd;
	std::string	_readBuffer;
	std::string	_writeBuffer;
};

#endif
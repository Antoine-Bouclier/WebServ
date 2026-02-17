#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

enum State
{
	READING,
	ZRITING,
	DONE
};

class Client
{
	private:
		int			_fd;
		std::string	_readBuffer;
		std::string	_writeBuffer;
		bool		_requestComplete;
	public:
		Client(int fd);
		~Client();
};


#endif
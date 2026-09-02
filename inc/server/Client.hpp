#ifndef CLIENT_HPP
# define CLIENT_HPP

/* -- Includes -- */
#include <string>
#include "http/HttpRequest.hpp"

/* -- Class -- */
class	Client
{
public:
	Client();
	Client(const Client&);
	Client(int clientFd, int listenerFd);

	~Client();

	Client&	operator=(const Client&);

	/*  -- Getters -- */
	int					getFd() const;
	int					getListenerFd() const;
	HttpRequest&		getRequest();
	const HttpRequest&	getRequest() const;

	const std::string&	getWriteBuffer() const;
	void				appendWriteBuffer(const std::string& data);
	void				clearWriteBuffer();
	bool				hasPendingWrite() const;
	void				consumeWriteBuffer(size_t bytes);

private:
	int			_fd;
	int			_listener_fd;
	HttpRequest	_request;
	std::string	_writeBuffer;
};

#endif
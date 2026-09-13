#ifndef CLIENT_HPP
# define CLIENT_HPP

/* -- Includes -- */
#include <ctime>
#include <fstream>
#include "http/HttpResponse.hpp"

#define FILE_BUFFER_SIZE 16384
#define CLIENT_IDLE_TIMEOUT 30
#define CLIENT_REQUEST_TIMEOUT 120
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

	int					getFd() const;
	int					getListenerFd() const;
	HttpRequest&		getRequest();
	const HttpRequest&	getRequest() const;

	const std::string&	getWriteBuffer() const;
	void				appendWriteBuffer(const std::string& data);
	void				clearWriteBuffer();
	bool				hasPendingWrite() const;
	void				consumeWriteBuffer(size_t bytes);

	void setResponse(const HttpResponse& response);
	bool fillWriteBuffer();
	void				touch();
	double				idleSeconds(std::time_t now) const;
	bool				hasTimedOut(std::time_t now) const;
	bool				requestTimedOut(std::time_t now) const;

private:
	int			_fd;
	HttpRequest	_request;
	int			_listener_fd;
	std::string	_writeBuffer;
	std::time_t _last_activity;
	std::time_t	_request_started;
	std::ifstream _file;
	std::string _file_path;
	std::streamoff _file_offset;
	std::streamoff _file_remaining;

};

#endif
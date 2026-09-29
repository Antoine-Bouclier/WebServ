#ifndef CGI_HPP
#define CGI_HPP

#include <ctime>
#include <poll.h>
#include <sys/types.h>
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "config/ConfigServer.hpp"

#define CGI_TIMEOUT 10
#define CGI_BUFFER_SIZE 16384
#define CGI_HEADER_LIMIT 16384
#define CGI_OUTPUT_LIMIT (64 * 1024 * 1024)

class Client;

class Cgi
{
public:
	Cgi();
	Cgi(const Cgi& other);
	Cgi& operator=(const Cgi& other);
	~Cgi();
	void start(const Client&, const HttpResponse&, const ConfigServer&, const std::vector<pollfd>&);
	void writeInput();
	void closeInput();
	void readOutput();
	void update();
	void cancel(HttpStatusCode error);
	void detach();
	int client() const;
	int input() const;
	int output() const;
	bool finished() const;
	pid_t pid() const;
	HttpStatusCode error() const;
	HttpResponse response() const;

private:
	int _client;
	int _input;
	int _output;
	pid_t _pid;
	int _status;
	bool _eof;
	bool _headersDone;
	HttpStatusCode _error;
	std::time_t _started;
	size_t _offset;
	std::vector<char> _body;
	std::string _data;
	std::vector<std::string> buildEnv(const HttpRequest& request, const HttpResponse& target, const ConfigServer& server, const std::string& remote) const;
	void launchProcess(const HttpResponse& target, std::vector<std::string>& environment, const std::vector<pollfd>& descriptors);
	bool createPipes(int bodyPipe[2], int responsePipe[2]);
	void checkOutputHeaders();
	void collectChildStatus();
	void closeOutput();
};

#endif

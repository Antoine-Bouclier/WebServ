#include <cerrno>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include "cgi/Cgi.hpp"
#include "server/Client.hpp"
#include "server/Socket.hpp"

using std::map;
using std::string;
using std::vector;

static vector<char*> environmentPointers(vector<string>& environment);
static void connectChildPipes(int bodyPipe[2], int responsePipe[2], const vector<pollfd>& fds);

void Cgi::start(const Client& client, const HttpResponse& response, const ConfigServer& server, const vector<pollfd>& fds)
{
	_client = client.getFd();
	_body = client.getRequest().getBody();

	vector<string> environment = buildEnv(client.getRequest(), response, server, client.getRemoteAddress());
	launchProcess(response, environment, fds);
}

void Cgi::launchProcess(const HttpResponse& response, vector<string>& environment, const vector<pollfd>& fds)
{
	vector<char*> envp = environmentPointers(environment);
	const string& interpreter = response.getCgiBinary();
	const string& scriptPath = response.getFilePath();

	size_t slash = scriptPath.find_last_of('/');
	string directory = slash == string::npos ? "." : scriptPath.substr(0, slash + 1);
	string scriptName = slash == string::npos ? scriptPath : scriptPath.substr(slash + 1);

	char* arguments[] = {const_cast<char*>(interpreter.c_str()), const_cast<char*>(scriptName.c_str()), NULL};

	int bodyPipe[2];
	int responsePipe[2];
	if (!createPipes(bodyPipe, responsePipe))
		return;

	_pid = fork();
	if (_pid == 0)
	{
		connectChildPipes(bodyPipe, responsePipe, fds);
		if (chdir(directory.c_str()) == 0)
			execve(arguments[0], arguments, &envp[0]);
		std::exit(EXIT_FAILURE);
	}

	close(bodyPipe[0]);
	close(responsePipe[1]);
	if (_pid < 0)
		cancel(INTERNAL_SERVER_ERROR);
	else if (_body.empty())
		closeInput();
}

bool Cgi::createPipes(int bodyPipe[2], int responsePipe[2])
{
	if (pipe(bodyPipe) < 0)
	{
		cancel(INTERNAL_SERVER_ERROR);
		return (false);
	}
	if (pipe(responsePipe) < 0)
	{
		close(bodyPipe[0]);
		close(bodyPipe[1]);
		cancel(INTERNAL_SERVER_ERROR);
		return (false);
	}

	_input = bodyPipe[1];
	_output = responsePipe[0];
	if (setNonBlocking(_input) && setNonBlocking(_output))
		return (true);

	close(bodyPipe[0]);
	close(responsePipe[1]);
	cancel(INTERNAL_SERVER_ERROR);
	return (false);
}

void Cgi::update()
{
	collectChildStatus();
	if (_error == OK && !finished() && std::difftime(std::time(NULL), _started) >= CGI_TIMEOUT)
		cancel(GATEWAY_TIMEOUT);
}

void Cgi::collectChildStatus()
{
	if (_pid <= 0)
		return;

	pid_t result = waitpid(_pid, &_status, WNOHANG);
	if (result == _pid)
	{
		_pid = -1;
		closeInput();
		if (_error == OK && (!WIFEXITED(_status) || WEXITSTATUS(_status)))
			cancel(BAD_GATEWAY);
	}
	else if (result < 0 && errno != EINTR)
	{
		_pid = -1;
		cancel(BAD_GATEWAY);
	}
}

void Cgi::cancel(HttpStatusCode error)
{
	_error = error;
	closeInput();
	closeOutput();
	if (_pid > 0)
		kill(_pid, SIGKILL);
}

vector<string> Cgi::buildEnv(const HttpRequest& request, const HttpResponse& target, const ConfigServer& server, const string& remote) const
{
	vector<string> env;
	const map<string, string>& headers = request.getheaders();
	map<string, string>::const_iterator it = headers.find("host");
	string host = it == headers.end() ? server.getHost() : it->second;
	std::ostringstream length;
	std::ostringstream port;
	length << request.getBody().size();
	port << server.getPort();

	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("SERVER_PROTOCOL=" + request.getVersion());
	env.push_back("SERVER_NAME=" + host.substr(0, host.find(':')));
	env.push_back("SERVER_PORT=" + port.str());
	env.push_back("REMOTE_ADDR=" + remote);
	env.push_back("REQUEST_METHOD=" + request.getMethod());
	env.push_back("REQUEST_URI=" + request.getUri());
	env.push_back("SCRIPT_NAME=" + target.getCgiName());
	env.push_back("SCRIPT_FILENAME=" + target.getFilePath().substr(target.getFilePath().find_last_of('/') + 1));
	env.push_back("PATH_INFO=" + target.getCgiPathInfo());
	env.push_back("QUERY_STRING=" + request.getQuery());
	env.push_back("CONTENT_LENGTH=" + length.str());
	env.push_back("REDIRECT_STATUS=200");

	it = headers.find("content-type");
	if (it != headers.end())
		env.push_back("CONTENT_TYPE=" + it->second);

	for (it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "content-length" || it->first == "content-type" || it->first == "transfer-encoding" || it->first == "connection" || it->first == "proxy")
			continue;
		string name = "HTTP_";
		for (size_t i = 0; i < it->first.size(); ++i)
			name += it->first[i] == '-' ? '_' : std::toupper(static_cast<unsigned char>(it->first[i]));
		env.push_back(name + "=" + it->second);
	}
	return (env);
}

static vector<char*> environmentPointers(vector<string>& environment)
{
	vector<char*> pointers(environment.size() + 1, NULL);
	for (size_t i = 0; i < environment.size(); ++i)
		pointers[i] = const_cast<char*>(environment[i].c_str());
	return (pointers);
}

static void connectChildPipes(int bodyPipe[2], int responsePipe[2], const vector<pollfd>& descriptors)
{
	if (dup2(bodyPipe[0], STDIN_FILENO) < 0 || dup2(responsePipe[1], STDOUT_FILENO) < 0)
		std::exit(EXIT_FAILURE);

	close(bodyPipe[0]);
	close(bodyPipe[1]);
	close(responsePipe[0]);
	close(responsePipe[1]);
	for (size_t i = 0; i < descriptors.size(); ++i)
		if (descriptors[i].fd > STDERR_FILENO)
			close(descriptors[i].fd);

	std::signal(SIGPIPE, SIG_DFL);
}

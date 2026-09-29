#include <csignal>
#include <iostream>
#include "server/Server.hpp"
#include "parser/ConfigParser.hpp"
#include "config/ConfigNormalizer.hpp"

volatile std::sig_atomic_t g_stopRequested = 0;

static void handleStop(int signal);

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv [configuration file]\n";
		return (1);
	}

	std::signal(SIGPIPE, SIG_IGN);
	std::signal(SIGINT, handleStop);
	std::signal(SIGTERM, handleStop);

	try
	{
		ConfigParser parser;
		parser.parseConfig(argv[1]);
		check_required(parser);
		Server server(parser.getServer());
		server.setupServer();
		server.run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		return (1);
	}
	return (0);
}

static void handleStop(int signal)
{
	(void)signal;
	g_stopRequested = 1;
}

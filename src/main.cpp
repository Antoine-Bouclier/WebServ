#include <csignal>
#include <iostream>

#include "server/Server.hpp"
#include "parser/ConfigParser.hpp"
#include "config/ConfigNormalizer.hpp"

volatile sig_atomic_t isAlive = 0;

void sigint_handler(int signal)
{
	(void)signal;
	isAlive = 0;
}

int	main(int argc, char **argv)
{
	if (argc == 2)
	{
		ConfigParser	parser;

		try
		{
			parser.parseConfig(argv[1]);
			check_required(parser);

			Server server(parser.getServer());
			server.setupServer();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (1);
		}
	}
	return (0);
}
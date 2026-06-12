#include <iostream>

#include "server/Server.hpp"
#include "parser/ConfigParser.hpp"
#include "config/ConfigNormalizer.hpp"

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
			server.run();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (1);
		}
	}
	return (0);
}
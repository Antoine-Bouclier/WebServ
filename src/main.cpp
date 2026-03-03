#include "WebServ.hpp"
#include "config/ConfigParser.hpp"

int	main(int argc, char **argv)
{
	if (argc == 2)
	{
		ConfigParser	parser;

		try
		{
			parser.parseConfig(argv[1]);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		
	}
	return (0);
}
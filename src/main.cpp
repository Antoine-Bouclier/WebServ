#include "WebServ.hpp"
#include "ConfigFile.hpp"

int	main(int argc, char **argv)
{
	if (argc == 2 || argc == 1)
	{
		std::string	file_name = argc == 2 ? argv[1] : "default";
		try
		{
			ConfigFile	config(file_name);
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		
	}
	return (0);
}
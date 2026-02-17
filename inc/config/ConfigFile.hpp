#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include "WebServ.hpp"
#include "ConfigServer.hpp"
#include "ConfigParser.hpp"

class ConfigFile
{
	private:
		std::vector<ConfigServer>	_server;
	public:
		/* -- Constructors -- */
		ConfigFile();
		ConfigFile(std::string file_name);
		ConfigFile(const ConfigFile &copy);
		ConfigFile	&operator=(const ConfigFile &copy);
		~ConfigFile();
};

#endif
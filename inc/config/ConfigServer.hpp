#ifndef CONFIGSERVER_HPP
#define CONFIGSERVER_HPP

#include "WebServ.hpp"
#include "ConfigLocation.hpp"

class ConfigServer
{
	private:
		uint16_t					_port;
		std::string					_server_name;
		std::string					_root;
		std::string					_index;
		std::string					_error_page;
		unsigned long				_client_max_body_size;
		std::vector<ConfigLocation>	_location;
	public:
		ConfigServer();
		~ConfigServer();
};


#endif
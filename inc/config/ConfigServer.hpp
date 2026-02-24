#ifndef CONFIGSERVER_HPP
#define CONFIGSERVER_HPP

#include "WebServ.hpp"
#include "ConfigLocation.hpp"

class ConfigServer
{
	private:
		uint16_t					_port;
		std::string					_host;
		std::string					_server_name;
		std::string					_root;
		std::string					_index;
		std::map<int, std::string>	_error_pages;
		unsigned long				_client_max_body_size;
		std::vector<ConfigLocation>	_location;
	public:
		ConfigServer();
		~ConfigServer();

		/* Setter */
		uint16_t	setPort() const;
		std::string	setHost() const;
		std::string	setServerName() const;
		std::string	setRoot() const;
		std::string	setIndex() const;
		void	parseServer();
};


#endif
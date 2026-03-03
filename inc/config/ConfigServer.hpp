#ifndef CONFIGSERVER_HPP
#define CONFIGSERVER_HPP

#include "WebServ.hpp"
#include "ConfigLocation.hpp"
#include "AConfig.hpp"

class ConfigServer : public AConfig
{
	private:
		uint16_t					_port;
		std::string					_host;
		std::pair<int, std::string>	_redirection;
		std::vector<std::string>	_server_names;
		std::vector<ConfigLocation>	_locations;
	public:
		ConfigServer();
		~ConfigServer();

		/* -- Setter -- */
		void	setPort(uint16_t port);
		void	setHost(const std::string& host);
		void	addServerName(const std::string& name);
		void	addLocation(const ConfigLocation& loc);

		/* -- Getters -- */
		uint16_t							getPort() const;
		std::string							getHost() const;
		const std::vector<std::string>&		getServerNames() const;
		const std::vector<ConfigLocation>&	getLocations() const;
};


#endif
#ifndef CONFIGSERVER_HPP
#define CONFIGSERVER_HPP

#include "WebServ.hpp"
#include "AConfig.hpp"
#include "ConfigLocation.hpp"

class ConfigServer : public AConfig
{
	private:
		int							_port;
		std::string					_host;
		std::vector<ConfigLocation>	_locations;
		std::pair<int, std::string>	_redirection;
		std::vector<std::string>	_server_names;

	public:
		ConfigServer();
		~ConfigServer();

		/* -- Setter -- */
		void	setPort(int port);
		void	setHost(const std::string& host);
		void	addServerName(const std::string& name);
		void	addLocation(const ConfigLocation& location);

		/* -- Getters -- */
		int									getPort(void) const;
		std::string							getHost(void) const;
		const std::vector<std::string>&		getServerNames(void) const;
		const std::vector<ConfigLocation>&	getLocations(void) const;

		/* -- Methods -- */
		void	applyInheritanceToLocations(void);
};

#endif

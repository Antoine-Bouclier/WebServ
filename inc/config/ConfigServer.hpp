#ifndef CONFIGSERVER_HPP
#define CONFIGSERVER_HPP

#include "config/ConfigLocation.hpp"

class ConfigServer : public AConfig
{
	private:
		int							_port;
		std::string					_host;
		std::vector<ConfigLocation>	_locations;
		std::pair<int, std::string>	_redirection;
		std::vector<std::string>	_server_names;

	public:
		/* -- Canonical Form -- */
		ConfigServer();
		~ConfigServer();
		ConfigServer(const ConfigServer& src);
		ConfigServer&	operator=(const ConfigServer& src);

		/* -- Setters -- */
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

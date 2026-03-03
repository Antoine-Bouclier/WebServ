#include "config/ConfigServer.hpp"

ConfigServer::ConfigServer()
{}

ConfigServer::~ConfigServer()
{}

/* -- Setter -- */
void	ConfigServer::setPort(uint16_t port)					{ _port = port; }
void	ConfigServer::setHost(const std::string& host)			{ _host = host; }
void	ConfigServer::addServerName(const std::string& name)	{ _server_names.push_back(name); }

void	ConfigServer::addLocation(const ConfigLocation& loc)
{
	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].getPath() == loc.getPath())
			throw ErrorException("Duplicate location path: " + loc.getPath());
	}
	_locations.push_back(loc);
}


/* -- Getters -- */
uint16_t							ConfigServer::getPort() const			{ return (_port); }
std::string							ConfigServer::getHost() const			{ return (_host); }
const std::vector<std::string>&		ConfigServer::getServerNames() const	{ return (_server_names); }
const std::vector<ConfigLocation>&	ConfigServer::getLocations() const		{ return (_locations); }

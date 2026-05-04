#include "core/Exception.hpp"
#include "config/ConfigServer.hpp"

ConfigServer::ConfigServer() : _port(-1) {}

ConfigServer::~ConfigServer() {}

/* -- Setter -- */
void	ConfigServer::setPort(int port)							{ _port = port; }
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
int									ConfigServer::getPort() const			{ return (_port); }
std::string							ConfigServer::getHost() const			{ return (_host); }
const std::vector<ConfigLocation>&	ConfigServer::getLocations() const		{ return (_locations); }
const std::vector<std::string>&		ConfigServer::getServerNames() const	{ return (_server_names); }

/* -- Methods -- */
void ConfigServer::applyInheritanceToLocations()
{
	std::vector<ConfigLocation>::iterator location = _locations.begin();
	for (; location != _locations.end(); location++)
	{
		if (!location->isClientMaxBodySizeAssigned())
			location->setClientMaxBody(_client_max_body_size);

		if (location->getRoot().empty())
		{
			if (_root.empty())
				throw ErrorException("Cannot inherit root from any server block");
			location->setRoot(_root);
		}

		if (location->getIndex().empty())
		{
			if (!_index.empty())
				location->setIndex(_index);
		}

		std::map<int, std::string>::const_iterator it = _error_pages.begin();

		for (; it != _error_pages.end(); ++it)
		{
			if (location->getErrorPage().find(it->first) == location->getErrorPage().end())
				location->addErrorPage(it->first, it->second);
		}
	}
}

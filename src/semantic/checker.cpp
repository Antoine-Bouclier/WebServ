#include <vector>
#include <algorithm>
#include "ConfigParser.hpp"

using std::map;
using std::string;
using std::vector;

static void	check_server_block(const ConfigServer& server)
{
	if (server.getPort() <= 0)
		throw ErrorException("Wrong or uninitialized port in a server block");

	if (not server.getServerNames().empty())
	{
		vector<string>::const_iterator it = server.getServerNames().begin();
		vector<string>::const_iterator end = server.getServerNames().end();

		for (; it != end; it++)
		{
			if (std::find(it + 1, end, *it) != end)
				throw ErrorException("Duplicate found in server names");
		}
	}

	if (server.getRoot().empty())
		throw ErrorException("No root found in server block");
}

static void	check_location_block(const ConfigLocation& location)
{
	if (not location.getCgi().empty())
	{
		map<string, string>::const_iterator it = location.getCgi().begin();
		for (; it != location.getCgi().end(); it++)
		{
			if (it->first.empty() || it->second.empty())
				throw ErrorException("Empty CGI key or value");
			if (it->first[0] != '.')
				throw ErrorException("CGI key does not starts with a dot");
		}
	}
}

bool	check_required(const ConfigParser& parsed)
{
	vector<ConfigServer>::const_iterator serv_it = parsed.getServer().begin();

	for (; serv_it != parsed.getServer().end(); serv_it++)
	{

		check_server_block(*serv_it);

		vector<ConfigLocation>::const_iterator locs_it = serv_it->getLocations().begin();

		for (; locs_it != serv_it->getLocations().end(); locs_it++)
			check_location_block(*locs_it);
	}
	return (true);
}
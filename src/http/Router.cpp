#include "http/Router.hpp"
#include <stdexcept>

Router::Router()
{
}

Router::Router(const Router& src)
{
	(void)src;
}

Router::~Router()
{
}

Router& Router::operator=(const Router& rhs)
{
	(void)rhs;
	return (*this);
}

static std::string extractHostName(const std::string& raw_host)
{
	std::size_t pos = raw_host.find(':');
	if (pos != std::string::npos)
		return raw_host.substr(0, pos);
	return raw_host;
}

const ConfigServer& Router::matchServer(const std::vector<ConfigServer>& servers, 
										const HttpRequest& request) const
{
	if (servers.empty())
		throw std::runtime_error("No servers configured for this port");

	std::string host_header;
	const std::map<std::string, std::string>& headers = request.getheaders();

	std::map<std::string, std::string>::const_iterator it = headers.find("host");
	if (it != headers.end())
		host_header = it->second;

	if (host_header.empty())
		return servers[0];

	std::string target_host = extractHostName(host_header);

	for (std::vector<ConfigServer>::const_iterator s_it = servers.begin(); s_it != servers.end(); ++s_it)
	{
		const std::vector<std::string>& names = s_it->getServerNames();

		for (std::vector<std::string>::const_iterator n_it = names.begin(); 
			n_it != names.end(); ++n_it)
		{
		if (*n_it == target_host)
			return *s_it;
		}
	}
	return servers[0];
}

const ConfigLocation* Router::matchLocation(const ConfigServer& server, const std::string& path) const
{
	const ConfigLocation* best_match = NULL;
	std::size_t max_len = 0;

	const std::vector<ConfigLocation>& locations = server.getLocations();

	for (std::vector<ConfigLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string& loc_path = it->getPath();

		if (path.find(loc_path) == 0)
		{
			bool is_valid_match = false;

			if (path.length() == loc_path.length())
				is_valid_match = true;
			else if (loc_path.size() > 0 && loc_path[loc_path.size() - 1] == '/')
				is_valid_match = true;
			else if (path.length() > loc_path.length() && path[loc_path.length()] == '/')
				is_valid_match = true;

			if (is_valid_match && loc_path.length() > max_len)
			{
				max_len = loc_path.length();
				best_match = &(*it);
			}
		}
	}

	return best_match;
}

RouteResult Router::route(const std::vector<ConfigServer>& candidate_servers, const HttpRequest& request) const
{
	RouteResult result;

	const ConfigServer& server = matchServer(candidate_servers, request);
	result.server = &server;

	const ConfigLocation* location = matchLocation(server, request.getPath());

	if (!location)
	{
		result.status_code = NOT_FOUND;
		return result;
	}

	result.location = location;
	result.status_code = OK;

	return result;
}
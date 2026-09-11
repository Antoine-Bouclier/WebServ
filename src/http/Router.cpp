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

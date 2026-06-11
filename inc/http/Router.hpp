#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "config/ConfigServer.hpp"

struct RouteResult
{
	const ConfigServer*		server;
	const ConfigLocation*	locations;
	int						status_code;
};

class Router
{
	private:
		
	public:
		Router();
		~Router();
};

#endif
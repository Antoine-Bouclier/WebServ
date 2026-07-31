#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "config/ConfigServer.hpp"
#include "http/httpStatusCode.hpp"
#include "http/HttpRequest.hpp"

#include <vector>

struct RouteResult
{
	const ConfigServer*		server;
	const ConfigLocation*	location;
	HttpStatusCode			status_code;

	RouteResult() : server(NULL), location(NULL), status_code(OK) {}
};

class Router
{
	private:
		RouteResult				route(const std::vector<ConfigServer>& candidate_servers, const HttpRequest& request) const;

	public:
		Router();
		Router(const Router& src);
		~Router();
		Router&	operator=(const Router& rhs);

		/* -- Main Method -- */
		const ConfigServer&		matchServer(const std::vector<ConfigServer>& servers, const HttpRequest& request) const;
		const ConfigLocation*	matchLocation(const ConfigServer& server, const std::string& uri) const;
};

#endif
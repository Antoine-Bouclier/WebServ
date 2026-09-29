#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "config/ConfigServer.hpp"
#include "http/HttpStatusCode.hpp"
#include "http/HttpRequest.hpp"

#include <vector>

class Router
{
	public:
		Router();
		Router(const Router& src);
		~Router();
		Router&	operator=(const Router& rhs);

		static const ConfigLocation* matchLocation(const ConfigServer& server, const std::string& uri);
};

#endif
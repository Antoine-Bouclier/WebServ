#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ConfigLocation.hpp"
#include "ConfigServer.hpp"
#include <string>
#include <sys/stat.h>

class RequestHandler {
	public:
		static HttpResponse handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server);

	private:
		RequestHandler();
		~RequestHandler();

		static std::string buildFilePath(const std::string& uri, const std::string& root);
		static bool isDirectory(const std::string& path);
		static bool isRegularFile(const std::string& path);
};

#endif
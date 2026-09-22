#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "config/ConfigServer.hpp"
#include "config/ConfigLocation.hpp"

class RequestHandler {
	public:
		static HttpResponse	handle(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server);
		static HttpResponse	buildErrorResponse(HttpStatusCode error, const ConfigLocation* loc, const ConfigServer* server);

	private:
		RequestHandler();
		RequestHandler(const RequestHandler& other);
		RequestHandler& operator=(const RequestHandler& other);
		~RequestHandler();

		static HttpResponse handleGet(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server);
		static HttpResponse handlePost(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server);
		static HttpResponse handleDelete(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server);
		static HttpResponse handleCgi(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server, const std::string& script_path, const std::string& cgi_bin);

		static bool			isDirectory(const std::string& path);
		static bool			isRegularFile(const std::string& path);
		static std::string	getMimeType(const std::string& path);
		static std::string	getEffectiveRoot(const ConfigLocation* location, const ConfigServer* server);
		static std::string	buildFilePath(const std::string& path, const std::string& root, const std::string& prefix);
		static HttpResponse generateAutoindex(const std::string& uri, const std::string& target_path, const ConfigLocation* location, const ConfigServer* server);
		static std::vector<std::string> buildCgiEnv(const HttpRequest& request, const std::string& script_path);
};

#endif
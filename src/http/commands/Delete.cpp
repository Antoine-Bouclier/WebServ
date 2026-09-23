#include "http/RequestHandler.hpp"
#include <cstdio>

using std::string;

HttpResponse RequestHandler::handleDelete(const HttpRequest& request, const ConfigLocation* location, const ConfigServer* server)
{
	string path = buildFilePath(request.getPath(), getEffectiveRoot(location, server), location ? location->getPath() : "");
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return (buildErrorResponse(fileError(), location, server));
	if (!S_ISREG(info.st_mode))
		return (buildErrorResponse(FORBIDDEN, location, server));

	if (std::remove(path.c_str()) != 0)
		return (buildErrorResponse(fileError(), location, server));

	HttpResponse response;
	response.setStatus(NO_CONTENT);
	return (response);
}


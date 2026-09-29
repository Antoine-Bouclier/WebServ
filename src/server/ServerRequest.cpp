#include "server/Server.hpp"

void Server::processClientRequest(int clientFd, const char* buffer, ssize_t bytes)
{
	Client& client = _clients[clientFd];
	HttpRequest& request = client.getRequest();
	const ConfigServer& config = getServerConfig(client.getListenerFd());

	request.feed(buffer, bytes);

	if (request.getState() == STATE_HEADERS_DONE)
	{
		const ConfigLocation* location = Router::matchLocation(config, request.getPath());
		request.startBody(location ? static_cast<const AConfig&>(*location) : config);
	}

	if (request.getState() != STATE_ERROR && request.getState() != STATE_READY)
		return;
	const ConfigLocation* location = Router::matchLocation(config, request.getPath());

	HttpResponse response = (request.getState() == STATE_ERROR) ?
		RequestHandler::buildErrorResponse(request.getStatusCode(), location, &config)
		: RequestHandler::handle(request, location, &config);

	if (!response.getCgiBinary().empty())
	{
		startCgi(clientFd, response, config);
		return;
	}
	client.setResponse(response);
	setPollEvents(clientFd, POLLOUT);
}

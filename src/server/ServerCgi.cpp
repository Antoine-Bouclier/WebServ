#include "server/Server.hpp"

bool Server::hasCgi(int client) const
{
	for (size_t i = 0; i < _cgis.size(); ++i)
		if (_cgis[i]->client() == client)
			return (true);
	return (false);
}

void Server::syncCgi(Cgi& cgi, int input, int output)
{
	if (input >= 0 && input != cgi.input())
		removePollFd(input);
	if (output >= 0 && output != cgi.output())
		removePollFd(output);
}

void Server::stopCgi(int client)
{
	for (size_t i = 0; i < _cgis.size(); ++i)
	{
		Cgi& cgi = *_cgis[i];
		if (cgi.client() != client)
			continue;

		int input = cgi.input();
		int output = cgi.output();
		cgi.detach();
		cgi.cancel(BAD_GATEWAY);
		syncCgi(cgi, input, output);
	}
}

void Server::startCgi(int clientFd, const HttpResponse& target, const ConfigServer& config)
{
	if (_cgis.size() >= MAX_CLIENTS)
		throw std::runtime_error("Too many CGI processes");

	Cgi* cgi = new Cgi;
	try
	{
		_cgis.push_back(cgi);
	}
	catch (...)
	{
		delete cgi;
		throw;
	}

	Client& client = _clients[clientFd];
	try
	{
		cgi->start(client, target, config, _poll_fds);
		if (cgi->input() >= 0)
			addPollFd(cgi->input(), POLLOUT);
		if (cgi->output() >= 0)
			addPollFd(cgi->output(), POLLIN);

		client.getRequest().releaseBody();
		setPollEvents(clientFd, 0);
	}
	catch (...)
	{
		int input = cgi->input();
		int output = cgi->output();
		cgi->cancel(INTERNAL_SERVER_ERROR);
		syncCgi(*cgi, input, output);
	}
}

bool Server::handleCgiEvent(int fd, short events)
{
	for (size_t i = 0; i < _cgis.size(); ++i)
	{
		Cgi& cgi = *_cgis[i];
		if (fd != cgi.input() && fd != cgi.output())
			continue;

		int input = cgi.input();
		int output = cgi.output();
		try
		{
			if (events & POLLNVAL)
				cgi.cancel(BAD_GATEWAY);
			else if (fd == input && (events & (POLLERR | POLLHUP)))
				cgi.closeInput();
			else if (events & POLLERR)
				cgi.cancel(BAD_GATEWAY);
			else if (fd == input && (events & POLLOUT))
				cgi.writeInput();
			else if (fd == output && (events & (POLLIN | POLLHUP)))
				cgi.readOutput();
		}
		catch (...)
		{
			cgi.cancel(INTERNAL_SERVER_ERROR);
		}
		syncCgi(cgi, input, output);
		return (true);
	}
	return (false);
}

void Server::updateCgi()
{
	for (size_t i = 0; i < _cgis.size();)
	{
		Cgi& cgi = *_cgis[i];

		int input = cgi.input();
		int output = cgi.output();
		cgi.update();
		syncCgi(cgi, input, output);
		int fd = cgi.client();
		if (cgi.finished() && fd >= 0 && _clients.find(fd) != _clients.end())
		{
			try
			{
				Client& client = _clients[fd];
				const ConfigServer& config = getServerConfig(client.getListenerFd());
				const ConfigLocation* location = Router::matchLocation(config, client.getRequest().getPath());
				HttpResponse response;
				HttpStatusCode error = cgi.error();
				if (error == OK)
				{
					try
					{
						response = cgi.response();
					}
					catch (...)
					{
						error = BAD_GATEWAY;
					}
				}
				if (error != OK)
					response = RequestHandler::buildErrorResponse(error, location, &config);
				client.setResponse(response);
				client.touch();
				setPollEvents(fd, POLLOUT);
			}
			catch (...)
			{
				closeClient(fd);
			}
			cgi.detach();
		}

		if (cgi.finished() && cgi.pid() < 0 && cgi.client() < 0)
		{
			delete _cgis[i];
			_cgis.erase(_cgis.begin() + i);
		}
		else
			++i;
	}
}

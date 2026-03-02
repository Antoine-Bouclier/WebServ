#include "config/ConfigParser.hpp"

void	ConfigParser::handleAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("autoindex directive requires a value (on/off).");
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for autoindex directive.");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	if (it->value == "on")
		loc.setAutoindex(true);
	else if (it->value == "off")
		loc.setAutoindex(false);
	else
		throw ErrorException("Unknown value for autoindex: " + it->value + "Allowed value: (on/off).");

	++it;
}

void	ConfigParser::handlePath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	
	if (it == end, it->type != TOKEN_WORD)
	throw ErrorException("Path name required in location.");
	if (it->value[0] != '/')
	throw ErrorException("Missing '/' at the beginning of the path.");
	
	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.setPath(it->value);

	++it;
}

void	ConfigParser::handleUploadPath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("upload_path directive requires a value.");
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for upload_path directive.");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.setUploadPath(it->value);

	++it;
}

void	ConfigParser::handleMethods(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.clearMethods();

	while (it != end || it->type == TOKEN_SEMICOLON)
	{
		if (it->type == TOKEN_WORD)
		{
			if (it->value == "GET" || it->value == "POST" || it->value == "DELETE")
				loc.addMethod(it->value);
			else
				throw ErrorException("Unknown method: " + it->value + ". Valid: [GET POST DELETE].");
		}
		else 
			throw ErrorException("Unexpected token in allowed_methods.");
		++it;
	}
	
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing ';' after allowed_methods.");
}

void	ConfigParser::handleCgi(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("cgi directive requires an extension.");
	std::string	extension = it->value;

	++it;
	if (it == end || it->type != TOKEN_WORD)
	throw ErrorException("cgi directive requires a binary path for extension.");
	std::string	binary_path = it->value;

	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';'.");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.addCgi(extension, binary_path);
}

/* -- Handlers server -- */
void	ConfigParser::handleListen(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("listen directive requires a Host.");

	std::string	host;
	std::string	port_str;
	size_t	found = it->value.find(':');
	if (found != std::string::npos)
	{
		host = it->value.substr(0, found);
		port_str = it->value.substr(found + 1);
	}
	else
	{
		host = "0.0.0.0";
		port_str = "8080";
	}

	++it;
	if (it ==end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after listen directive.");

	if (port_str.empty() || port_str.find_first_not_of("0123456789") != std::string::npos)
		throw ErrorException("Invalid port: " + port_str);
	long	port = std::atol(port_str.c_str());
	if (port < 1 || port > 65535)
		throw ErrorException("Port out of range: " + port_str);

	ConfigServer& server = static_cast<ConfigServer&>(config);
	server.setHost(host);
	server.setPort(static_cast<uint16_t>(port));
}

void	ConfigParser::handleServerNames(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	ConfigServer& server = static_cast<ConfigServer&>(config);

	if (it == end || it->type == TOKEN_SEMICOLON)
		throw ErrorException("server_name directive requires at least one value.");
	while (it != end && it->type == TOKEN_WORD)
	{
		server.addServerName(it->value);
		++it;
	}
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after server_name directive.");
}

void	ConfigParser::handleLocation(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	ConfigServer& server = static_cast<ConfigServer&>(config);
	ConfigLocation	new_loc;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Location requires a path.");
	if (it->value[0] != '/')
		throw ErrorException("Location prefix must start with '/'");
	server.addLocation(new_loc);
}
/* -- Handlers AConfig -- */
void	ConfigParser::handleIndex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Index directive require at least one file.");
	while (it != end && it->type == TOKEN_WORD)
	{
		config.addIndex(it->value);
		++it;
	}
	if (it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after index directive.");
}

void	ConfigParser::handleClientMax(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("client_max_body_size directive require a value.");
	if (it->value.rend() == )
	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for client_max_body_size directive.");
}

void	ConfigParser::handleErrorPage(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{

}
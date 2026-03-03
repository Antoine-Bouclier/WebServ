#include "config/ConfigParser.hpp"

void	ConfigParser::handleAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("autoindex directive requires a value (on/off).", it->line);
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for autoindex directive.", it->line);

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	if (it->value == "on")
		loc.setAutoindex(true);
	else if (it->value == "off")
		loc.setAutoindex(false);
	else
		throw ErrorException("Unknown value for autoindex: " + it->value + " Allowed value: (on/off).", it->line);

	++it;
}

void	ConfigParser::handlePath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	
	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Path name required in location.", it->line);
	if (it->value[0] != '/')
		throw ErrorException("Missing '/' at the beginning of the path.", it->line);
	
	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.setPath(it->value);

	++it;
}

void	ConfigParser::handleUploadPath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("upload_path directive requires a value.", it->line);
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for upload_path directive.", it->line);

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.setUploadPath(it->value);

	++it;
}

void	ConfigParser::handleMethods(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.clearMethods();

	while (it != end && it->type != TOKEN_SEMICOLON)
	{
		if (it->type == TOKEN_WORD)
		{
			if (it->value == "GET" || it->value == "POST" || it->value == "DELETE")
				loc.addMethod(it->value);
			else
				throw ErrorException("Unknown method: " + it->value + ". Valid: [GET POST DELETE].", it->line);
		}
		else 
			throw ErrorException("Unexpected token in allowed_methods." + it->value, it->line);
		++it;
	}
	
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing ';' after allowed_methods.", it->line);
}

void	ConfigParser::handleCgi(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("cgi directive requires an extension.", it->line);
	std::string	extension = it->value;

	++it;
	if (it == end || it->type != TOKEN_WORD)
	throw ErrorException("cgi directive requires a binary path for extension.", it->line);
	std::string	binary_path = it->value;

	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';'.", it->line);

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.addCgi(extension, binary_path);
}

/* -- Handlers server -- */
void	ConfigParser::handleListen(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("listen directive requires a Host.", it->line);

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
		throw ErrorException("Missing semicolon ';' after listen directive.", it->line);

	if (port_str.empty() || port_str.find_first_not_of("0123456789") != std::string::npos)
		throw ErrorException("Invalid port: " + port_str, it->line);
	long	port = std::atol(port_str.c_str());
	if (port < 1 || port > 65535)
		throw ErrorException("Port out of range: " + port_str, it->line);

	ConfigServer& server = static_cast<ConfigServer&>(config);
	server.setHost(host);
	server.setPort(static_cast<uint16_t>(port));
}

void	ConfigParser::handleServerNames(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	ConfigServer& server = static_cast<ConfigServer&>(config);

	if (it == end || it->type == TOKEN_SEMICOLON)
		throw ErrorException("server_name directive requires at least one value.", it->line);
	while (it != end && it->type == TOKEN_WORD)
	{
		server.addServerName(it->value);
		++it;
	}
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after server_name directive.", it->line);
}

void	ConfigParser::handleLocation(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	ConfigServer& server = static_cast<ConfigServer&>(config);
	ConfigLocation	new_loc;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Location requires a path.", it->line);
	if (it->value[0] != '/')
		throw ErrorException("Location prefix must start with '/'", it->line);

	new_loc.setPath(it->value);

	++it;

	if (it == end || it->type != TOKEN_LBRACE)
		throw ErrorException("Expected '{' after location path.", it->line);
	++it;
	parseBlock(it, end, new_loc);
	server.addLocation(new_loc);
}

/* -- Handlers AConfig -- */
void	ConfigParser::handleIndex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Index directive require at least one file.", it->line);
	while (it != end && it->type == TOKEN_WORD)
	{
		config.addIndex(it->value);
		++it;
	}
	if (it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after index directive.", it->line);
}

void	ConfigParser::handleClientMax(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	long	multiplier = 1;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("client_max_body_size directive require a value.", it->line);

	std::string	value = it->value;
	char		last_char = value.at(value.size() - 1);

	if (!isdigit(last_char))
	{
		if (last_char == 'K' || last_char == 'k')
			multiplier = 1024;
		else if (last_char == 'M' || last_char == 'm')
			multiplier = 1024 * 1024;
		else if (last_char == 'G' || last_char == 'g')
			multiplier = 1024 * 1024 * 1024;
		else
			throw ErrorException("Invalid unit: " + value, it->line);
		value = value.substr(0, it->value.size() - 1);
	}

	if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
		throw ErrorException("Invalid numeric value in client_max_body_size", it->line);

	long		size = std::atol(value.c_str()) * multiplier;
	config.setClientMaxBody(static_cast<size_t>(size));

	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for client_max_body_size directive.", it->line);
}

bool	isNumber(const std::string& s)
{
	if (s.empty())
		return (false);
	for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
	{
		if (!isdigit(*it))
			return (false);
	}
	return (true);
}

void	ConfigParser::handleErrorPage(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	std::vector<int>	codes;

	while (it != end && it->type == TOKEN_WORD && isNumber(it->value))
	{
		int code = std::atoi(it->value.c_str());
		if (code < 300 || code > 599)
			throw ErrorException("Invalid error code: " + it->value, it->line);
		codes.push_back(code);
		++it;
	}

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("Missing argument in error_page directive.", it->line);

	std::string	path = it->value;

	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after error_page directive.", it->line);

	if (codes.empty())
		throw ErrorException("error_page directive requires at least one code", it->line);

	for (size_t i = 0; i < codes.size(); i++)
	{
		config.addErrorPage(codes[i],path);
	}
}

void	ConfigParser::handleRoot(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("root directive requires a path.", it->line);

	config.setRoot(it->value);

	++it;

	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';' after root directive.", it->line);
}
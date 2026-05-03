#include <algorithm>
#include "ConfigParser.hpp"

static bool				isNumber(const std::string& s);
static void				requireToken(iter it, iter end, TokenType type, const std::string& msg);

/* -- Location handlers -- */
void	ConfigParser::handleAutoindex(iter &it, iter end, AConfig &config)
{
	ConfigLocation& loc = require<ConfigLocation>(it, config, "autoindex");

	if (loc.isAutoIndexAssigned())
		throw ErrorException("Duplicate autoindex directive", it->line);
	loc.setAutoIndexAssigned();

	requireToken(it, end, TOKEN_WORD, "autoindex directive requires a value (on/off).");
	requireToken(it + 1, end, TOKEN_SEMICOLON, "Too many arguments for autoindex directive.");
	if (it->value != "on" && it->value != "off")
		throw ErrorException("Unknown value for autoindex: " + it->value + " Allowed value: (on/off).", it->line);

	loc.setAutoindex(it->value == "on");

	++it;
}

void	ConfigParser::handleUploadPath(iter &it, iter end, AConfig &config)
{
	ConfigLocation& loc = require<ConfigLocation>(it, config, "upload_path");

	if (!loc.getUploadPath().empty())
		throw ErrorException("Duplicate upload_path directive", it->line);

	requireToken(it, end, TOKEN_WORD, "upload_path directive requires a value.");
	requireToken(it + 1, end, TOKEN_SEMICOLON, "Too many arguments for upload_path directive.");

	loc.setUploadPath(it->value);

	++it;
}

void	ConfigParser::handleMethods(iter &it, iter end, AConfig &config)
{
	ConfigLocation& loc = require<ConfigLocation>(it, config, "allowed_methods");

	if (loc.isMethodsAssigned())
		throw ErrorException("Duplicate methods directive", it->line);
	loc.setMethodsAssigned();
	
	loc.clearMethods();

	while (it != end && it->type != TOKEN_SEMICOLON)
	{
		if (it->type == TOKEN_WORD)
		{
			if (it->value != "GET" && it->value != "POST" && it->value != "DELETE")
				throw ErrorException("Unknown method: " + it->value + ". Valid: [GET POST DELETE].", it->line);
			if (std::find(loc.getMethods().begin(), loc.getMethods().end(), it->value) != loc.getMethods().end())
				throw ErrorException("Duplicate method found in methods directive", it->line);
			loc.addMethod(it->value);
		}
		else 
			throw ErrorException("Unexpected token in allowed_methods: " + it->value, it->line);
		++it;
	}

	if (loc.getMethods().empty())
		throw (it != end ? ErrorException("methods directive cannot be empty", it->line) : ErrorException("methods directive cannot be empty"));
	
	requireToken(it, end, TOKEN_SEMICOLON, "Missing ';' after allowed_methods.");
}

void	ConfigParser::handleCgi(iter &it, iter end, AConfig &config)
{
	ConfigLocation& loc = require<ConfigLocation>(it, config, "cgi");

	requireToken(it, end, TOKEN_WORD, "cgi directive requires an extension.");
	requireToken(it + 1, end, TOKEN_WORD, "cgi directive requires a binary path for extension.");

	std::string	extension = (it++)->value;
	std::string	binary_path = (it++)->value;

	requireToken(it, end, TOKEN_SEMICOLON, "Missing ';' after cgi.");

	if (loc.getCgi().find(extension) != loc.getCgi().end())
		throw ErrorException("Duplicate extension found in cgi directive", it->line);

	loc.addCgi(extension, binary_path);
}

/* -- Server handlers -- */
void	ConfigParser::handleListen(iter &it, iter end, AConfig &config)
{
	requireToken(it, end, TOKEN_WORD, "listen directive requires a Host.");

	ConfigServer& server = require<ConfigServer>(it, config, "listen");

	if (server.getPort() != -1 || !server.getHost().empty())
		throw ErrorException("Duplicate listen directive", it->line);

	std::string	host = "0.0.0.0";
	std::string	port_str = "8080";

	size_t	found = it->value.find(':');
	if (found != std::string::npos)
	{
		host = it->value.substr(0, found);
		port_str = it->value.substr(found + 1);
	}
	else
		port_str = it->value;

	++it;
	requireToken(it, end, TOKEN_SEMICOLON, "Missing semicolon ';' after listen directive.");

	if (!isNumber(port_str) || port_str.size() > 5)
		throw ErrorException("Invalid port: " + port_str, it->line);

	int port = std::atoi(port_str.c_str());

	if (port < 1 || port > 65535)
		throw ErrorException("Port out of range: " + port_str, it->line);

	if (host.empty())
		throw ErrorException("Empty host name", it->line);
	else if (host == "*")
		host = "0.0.0.0";

	server.setHost(host);
	server.setPort(port);
}

void	ConfigParser::handleServerNames(iter &it, iter end, AConfig &config)
{
	ConfigServer& server = require<ConfigServer>(it, config, "server_names");

	if (!server.getServerNames().empty())
		throw ErrorException("Duplicate server_names directive", it->line);

	requireToken(it, end, TOKEN_WORD, "server_name directive requires at least one value.");
	while (it != end && it->type == TOKEN_WORD)
	{
		server.addServerName(it->value);
		++it;
	}
	requireToken(it, end, TOKEN_SEMICOLON, "Missing semicolon ';' after server_name directive.");
}

void	ConfigParser::handleLocation(iter &it, iter end, AConfig &config)
{
	ConfigLocation	new_loc;
	ConfigServer&	server = require<ConfigServer>(it, config, "locations");

	requireToken(it, end, TOKEN_WORD, "Location requires a path.");
	if (it->value[0] != '/')
		throw ErrorException("Location prefix must start with '/'", it->line);

	new_loc.setPath(it->value);

	parseBlock(it, end, new_loc);
	server.addLocation(new_loc);
}

/* -- AConfig handlers -- */
void	ConfigParser::handleIndex(iter &it, iter end, AConfig &config)
{
/* Uncomment to restrict index directive's duplicates */
	// if (!config.getIndex().empty())
	// 	throw ErrorException("Duplicate index directive", it->line);

	requireToken(it, end, TOKEN_WORD, "Index directive require at least one file.");

	while (it != end && it->type == TOKEN_WORD)
		config.addIndex((it++)->value);

	requireToken(it, end, TOKEN_SEMICOLON, "Missing semicolon ';' after index directive.");
}

void	ConfigParser::handleClientMax(iter &it, iter end, AConfig &config)
{
	if (config.isClientMaxBodySizeAssigned())
		throw ErrorException("Duplicate client_max_body_size directive", it->line);
	config.setAssignedClientMaxBodySize();

	requireToken(it, end, TOKEN_WORD, "client_max_body_size directive require a value.");
	if (it->value.empty())
		throw ErrorException("Empty client_max_body_size", it->line);

	long		multiplier = 1;
	std::string	value = it->value;
	char		last_char = value.at(value.size() - 1);

	if (!isdigit(last_char))
	{
		switch (toupper(last_char))
		{
			case 'K': multiplier = 1024L; break;
			case 'M': multiplier = 1024L * 1024L; break;
			case 'G': multiplier = 1024L * 1024L * 1024L; break;
			default:
				throw ErrorException("Invalid unit: " + value, it->line);
		}
		value.erase(value.size() - 1);
	}

	if (!isNumber(value))
		throw ErrorException("Invalid numeric value in client_max_body_size", it->line);

	size_t	number = 0;
	for (size_t i = 0; i < value.size(); i++)
	{
		size_t	digit = value[i] - '0';
		if (number > (static_cast<size_t>(-1) - digit) / 10)
			throw ErrorException("client_max_body_size doesn't fit in a size_t", it->line);
		number = number * 10 + digit;
	}
	
	if (number > static_cast<size_t>(-1) / multiplier)
		throw ErrorException("client_max_body_size doesn't fit in a size_t", it->line);

	config.setClientMaxBody(number * multiplier);

	++it;
	requireToken(it, end, TOKEN_SEMICOLON, "Too many arguments for client_max_body_size directive.");
}

void	ConfigParser::handleErrorPage(iter &it, iter end, AConfig &config)
{
	std::vector<int>	codes;

	while (it != end && it->type == TOKEN_WORD && isNumber(it->value))
	{
		int code = std::atoi(it->value.c_str());

		if (code < 300 || code > 599)
			throw ErrorException("Invalid error code: " + it->value, it->line);

		if (std::count(codes.begin(), codes.end(), code) > 0
			|| config.getErrorPage().find(code) != config.getErrorPage().end())
			throw ErrorException("Duplicate code found in error_code directive", it->line);
		
		codes.push_back(code);
		++it;
	}

	requireToken(it, end, TOKEN_WORD, "Missing path in error_page directive.");

	std::string	path = it->value;
	if (path[0] != '/' || path.size() <= 1)
		throw ErrorException("Invalid error_page path", it->line);

	++it;
	requireToken(it, end, TOKEN_SEMICOLON, "Missing semicolon ';' after error_page directive.");

	if (codes.empty())
		throw ErrorException("error_page directive requires at least one code", it->line);

	for (size_t i = 0; i < codes.size(); i++)
	{
		config.addErrorPage(codes[i], path);
	}
}

void	ConfigParser::handleRoot(iter &it, iter end, AConfig &config)
{
	if (!config.getRoot().empty())
		throw ErrorException("Duplicate root directive", it->line);

	requireToken(it, end, TOKEN_WORD, "root directive requires a path.");

	config.setRoot(it->value);

	++it;

	requireToken(it, end, TOKEN_SEMICOLON, "Missing semicolon ';' after root directive.");
}

/* -- Wrappers (utils) -- */
static bool isNumber(const std::string& s)
{
	return !(s.empty() || s.find_first_not_of("0123456789") != std::string::npos);
}

static void requireToken(iter it, iter end, TokenType type, const std::string& msg)
{
	if (it == end || it->type != type)
		throw (it != end ? ErrorException(msg, it->line) : ErrorException(msg));
}

/* -- Template specialisation definitions -- */
const char* const ConfigParser::ConfigName<ConfigServer>::name = "server";
const char* const ConfigParser::ConfigName<ConfigLocation>::name = "location";

#include "config/ConfigParser.hpp"

void	ConfigParser::handleAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("autoindex directive requires a value (on/off)");
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for autoindex directive");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	if (it->value == "on")
		loc.setAutoindex(true);
	else if (it->value == "off")
		loc.setAutoindex(false);
	else
		throw ErrorException("Unknown value for autoindex: " + it->value + "Allowed value: (on/off)");

	++it;
}

void	ConfigParser::handlePath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;
	
	if (it == end, it->type != TOKEN_WORD)
	throw ErrorException("Path name required in location");
	if (it->value[0] != '/')
	throw ErrorException("Missing '/' at the beginning of the path");
	
	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.setPath(it->value);

	++it;
}

void	ConfigParser::handleUploadPath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("upload_path directive requires a value");
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ErrorException("Too many arguments for upload_path directive");

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
				throw ErrorException("Unknown method: " + it->value + ". Valid: [GET POST DELETE]");
		}
		else 
			throw ErrorException("Unexpected token in allowed_methods");
		++it;
	}
	
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing ';' after allowed_methods");
}

void	ConfigParser::handleCgi(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type != TOKEN_WORD)
		throw ErrorException("cgi directive requires an extension");
	std::string	extension = it->value;

	++it;
	if (it == end || it->type != TOKEN_WORD)
	throw ErrorException("cgi directive requires a binary path for extension");
	std::string	binary_path = it->value;

	++it;
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw ErrorException("Missing semicolon ';'");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	loc.addCgi(extension, binary_path);
}

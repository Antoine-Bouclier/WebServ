#include "config/ConfigParser.hpp"

void	ConfigParser::handleAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	++it;

	if (it == end || it->type == TOKEN_SEMICOLON)
		throw ConfigParser::ErrorException("autoindex directive requires a value (on/off)");
	if ((it + 1) == end || (it + 1)->type != TOKEN_SEMICOLON)
		throw ConfigParser::ErrorException("Too many arguments for autoindex directive");

	ConfigLocation& loc = static_cast<ConfigLocation&>(config);
	if (it->value == "on")
		loc.setAutoindex(true);
	else if (it->value == "off")
		loc.setAutoindex(false);
	else
		throw ConfigParser::ErrorException("Unknown value for autoindex: " + it->value + "Allowed value: (on/off)");

	++it;
}

void	ConfigParser::handlePath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{

}

void	ConfigParser::handleUploadPath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{

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
			throw AConfig::ErrorException("Unexpected token in allowed_methods");
		++it;
	}
	
	if (it == end || it->type != TOKEN_SEMICOLON)
		throw AConfig::ErrorException("Missing ';' after allowed_methods");
}

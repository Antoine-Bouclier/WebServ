#include "ConfigParser.hpp"

std::string	readFile(const std::string &path)
{
	std::ifstream	file(path.c_str());
	if (!file.is_open())
		throw	ConfigParser::ErrorException("Cannot open config file");
	std::stringstream	buffer;
	buffer << file.rdbuf();
	return (buffer.str());
}

ConfigFile	parseConfig(const std::string &path)
{
	
}

/* -- Exception -- */
ConfigParser::ErrorException::ErrorException(std::string message)
{
	_message = "CONFIG ERROR: " + message;
}

const char *ConfigParser::ErrorException::what() const
{
	return (_message.c_str());
}

ConfigParser::ErrorException::~ErrorException()
{

}
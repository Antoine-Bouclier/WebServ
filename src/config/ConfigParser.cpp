#include "ConfigParser.hpp"




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
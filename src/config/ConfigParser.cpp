#include "ConfigParser.hpp"

std::string	ConfigParser::readFile(const char* path)
{
	struct stat buf;

	_path = path;
	if (stat(path, &buf) == -1)
		throw	ConfigParser::ErrorException(_path + ": No such file or directory");
	if (S_ISDIR(buf.st_mode)) 
		throw	ConfigParser::ErrorException(_path + " is a Directory");
	else if (S_ISREG(buf.st_mode))
	{
		std::ifstream	file(_path.c_str());
		if (!file.is_open())
			throw	ConfigParser::ErrorException("Cannot open config file");
		std::stringstream	buffer;
		buffer << file.rdbuf();
		return (buffer.str());
	}
	else
		throw	ConfigParser::ErrorException(_path + " is unknown");
}

void	ConfigParser::parseConfig(const char* path)
{
	_lexer.Tokenize(readFile(path));
	_lexer.PrintToken(); //! REMOVE
}

ConfigParser::ErrorException::ErrorException(std::string const& message) : _message("CONFIG ERROR: " + message) {}

ConfigParser::ErrorException::~ErrorException() throw() {}

const char* ConfigParser::ErrorException::what() const throw()
{
    return _message.c_str();
}

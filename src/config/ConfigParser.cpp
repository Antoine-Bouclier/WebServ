#include "config/ConfigParser.hpp"

std::string	ConfigParser::readFile(const char* path)
{
	struct stat buf;

	if (stat(path, &buf) == -1)
		throw	ErrorException(_path + ": No such file or directory");
	if (S_ISDIR(buf.st_mode)) 
		throw	ErrorException(_path + " is a Directory");
	else if (S_ISREG(buf.st_mode))
	{
		std::ifstream	file(_path.c_str());
		if (!file.is_open())
			throw	ErrorException("Cannot open config file");
		std::stringstream	buffer;
		buffer << file.rdbuf();
		return (buffer.str());
	}
	else
		throw	ErrorException(_path + " is unknown");
}

void	ConfigParser::checkExt()
{
	std::string	ext = ".conf";

	if (_path.size() < ext.size())
		throw	ErrorException("Extension must be <.conf>");
	std::string::reverse_iterator	it_path = _path.rbegin();
	for (std::string::reverse_iterator it = ext.rbegin(); it != ext.rend(); ++it)
	{
		if (*it != *it_path)
			throw	ErrorException("Extension must be <.conf>");
		++it_path;
	}
}

void	ConfigParser::parseServer(std::vector<Token>::iterator &it, std::vector<Token>::iterator end)
{
	ConfigServer	new_server;
	std::string		directives[7] = {
						"listen", "server_name", "root", "client_max_body_size", "error_page", "location", "index"};
	void			(ConfigParser::*ptr[])(std::vector<Token>::iterator&, std::vector<Token>::iterator, ConfigServer&) = {
						&ConfigParser::handleListen, &ConfigParser::handleName, &ConfigParser::handleRoot, &ConfigParser::handleClientMax,
						&ConfigParser::handleErrorPage, &ConfigParser::handleLocation, &ConfigParser::handleIndex};

	++it;
	if (it == end || it->type != TOKEN_LBRACE)
		throw	ErrorException("Missing left brace.");
	++it;

	while (it != end && it->type != TOKEN_RBRACE)
	{
		for (int i = 0; i < 7; i++)
		{
			bool	found = false;
			if (it->value == directives[i])
			{
				(this->*ptr[i])(it, end, new_server);
				found = true;
				break ;
			}
			if (!found)
				throw	ErrorException("Unknown directive: " + it->value);
			++it;
		}
	}
	if (it == end)
		throw	ErrorException("Missing right brace for server block.");
	_server.push_back(new_server);
}

void	ConfigParser::parseConfig(const char* path)
{
	std::vector<Token>	tokens;

	_path = path;
	checkExt();
	tokens = _lexer.tokenize(readFile(path));
	for (std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->value.compare("server") != 0)
			throw	ErrorException("Only server blocks are allowed in the main context.");
		else
			parseServer(it, tokens.end());
	}
}

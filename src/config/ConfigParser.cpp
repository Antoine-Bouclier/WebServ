#include "config/ConfigParser.hpp"
ConfigParser::ConfigParser()
{
	initHandlers();
}

ConfigParser::~ConfigParser()
{

}

void	ConfigParser::initHandlers()
{
	/* -- Handlers AConfig -- */
	_handlers["index"] = &ConfigParser::handleIndex;
	_handlers["client_max_body_size"] = &ConfigParser::handleClientMax;
	_handlers["error_page"] = &ConfigParser::handleErrorPage;
	_handlers["root"] = &ConfigParser::handleRoot;
		
	/* -- Handlers location -- */
	_handlers["autoindex"] = &ConfigParser::handleAutoindex;
	_handlers["upload_path"] = &ConfigParser::handleUploadPath;
	_handlers["allowed_methods"] = &ConfigParser::handleMethods;
	_handlers["cgi"] = &ConfigParser::handleCgi;

	/* -- Handlers server -- */
	_handlers["listen"] = &ConfigParser::handleListen;
	_handlers["server_name"] = &ConfigParser::handleServerNames;
	_handlers["location"] = &ConfigParser::handleLocation;
}

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

	++it;
	if (it == end || it->type != TOKEN_LBRACE)
		throw	ErrorException("Missing left brace.");
	++it;

	parseBlock(it, end, new_server);

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
		if (it->value.compare("server") != 0 && it->type == TOKEN_WORD)
			throw	ErrorException("Only server blocks are allowed in the main context.");
		else
			parseServer(it, tokens.end());
	}
}

void	ConfigParser::parseBlock(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config)
{
	while (it != end && it->type != TOKEN_RBRACE)
	{
		if (it->type == TOKEN_WORD)
		{
			std::map<std::string, Handler>::iterator h = _handlers.find(it->value);

			if (h != _handlers.end())
				(this->*(h->second))(it, end, config);
			else
				throw ErrorException("Unknown directive: " + it->value);
		}
		else
			throw ErrorException("Unexpected token in block.");
		++it;
	}
}

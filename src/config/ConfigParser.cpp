#include "ConfigParser.hpp"

using std::string;

void	ConfigParser::parseConfig(const char* path)
{
	std::vector<Token>	tokens;

	_path = path;
	if (_path.size() < 5 || _path.compare(_path.size() - 5, 5, ".conf"))
		throw	ErrorException("Extension must be <.conf>");

	tokens = _lexer.tokenize(readFile(path));
	_lexer.printToken();

	for (iter it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->value != "server" && it->type == TOKEN_WORD)
			throw	ErrorException("Only server blocks are allowed in the main context.", it->line);

		ConfigServer server_block;
		parseBlock(it, tokens.end(), server_block);
		_server.push_back(server_block);
	}
}

/* -- Private methods -- */

ConfigParser::ConfigParser()
{
	/* -- Handlers AConfig -- */
	_handlers["root"] = &ConfigParser::handleRoot;
	_handlers["index"] = &ConfigParser::handleIndex;
	_handlers["error_page"] = &ConfigParser::handleErrorPage;
	_handlers["client_max_body_size"] = &ConfigParser::handleClientMax;
		
	/* -- Handlers location -- */
	_handlers["cgi"] = &ConfigParser::handleCgi;
	_handlers["autoindex"] = &ConfigParser::handleAutoindex;
	_handlers["upload_path"] = &ConfigParser::handleUploadPath;
	_handlers["allowed_methods"] = &ConfigParser::handleMethods;

	/* -- Handlers server -- */
	_handlers["listen"] = &ConfigParser::handleListen;
	_handlers["location"] = &ConfigParser::handleLocation;
	_handlers["server_name"] = &ConfigParser::handleServerNames;
}

string	ConfigParser::readFile(const char* path)
{
	struct stat buf;

	if (stat(path, &buf) == -1)
		throw	ErrorException(_path + ": No such file or directory");
	if (S_ISDIR(buf.st_mode)) 
		throw	ErrorException(_path + " is a Directory");
	if (S_ISREG(buf.st_mode))
	{
		std::ifstream	file(path);

		if (!file.is_open())
			throw	ErrorException("Cannot open config file");

		std::stringstream	buffer;
		buffer << file.rdbuf();
		return (buffer.str());
	}
	else
		throw	ErrorException(_path + " is unknown");
}

void	ConfigParser::parseBlock(iter &it, iter end, AConfig &config)
{
	if (++it == end || it->type != TOKEN_LBRACE)
		throw	ErrorException("Missing left brace in block.", it->line);

	++it;

	while (it != end && it->type != TOKEN_RBRACE)
	{
		if (it->type == TOKEN_WORD)
		{
			std::map<string, Handler>::iterator h = _handlers.find(it->value);

			if (h != _handlers.end())
				(this->*(h->second))(++it, end, config);
			else
				throw ErrorException("Unknown directive: " + it->value, it->line);
		}
		else
			throw ErrorException("Unexpected token in block.", it->line);
		++it;
	}

	if (it == end || it->type != TOKEN_RBRACE)
		throw	ErrorException("Missing right brace in block.", it->line);
}

ConfigParser::~ConfigParser() {}

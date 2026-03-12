#include "Lexer.hpp"

Lexer::Lexer() {}

Lexer::Lexer(const Lexer&) {}

Lexer &Lexer::operator=(const Lexer&)
{
	return (*this);
}

Lexer::~Lexer() {}

TokenType	Lexer::getTokenType(char c)
{
	switch (c)
	{
		case '\0':	return (TOKEN_EOF);
		case '{':	return (TOKEN_LBRACE);
		case '}':	return (TOKEN_RBRACE);
		case ';':	return (TOKEN_SEMICOLON);

		default:	return (TOKEN_WORD);
	}
}

bool	Lexer::isSpecial(char c)
{
	return (c == '{' || c ==  '}' || c ==  ';' || c ==  '\0');
}

void Lexer::addToken(Token& token, const uint line, TokenType type)
{
	if (token.value.empty())
		return ;

	token.type = type;
	token.line = line;
	_tokens.push_back(token);
	token.value.clear();
}

std::vector<Token>	Lexer::tokenize(const std::string& config)
{
	uint line = 1;
	Token token;

	_tokens.clear();

	for (ulong i = 0; i < config.size(); i++)
	{

		if (config[i] == '\n')
		{
			addToken(token, line, TOKEN_WORD);
			line++;
			continue;
		}

		if (config[i] == '#')
		{
			addToken(token, line, TOKEN_WORD);
			while (i < config.size() && config[i] != '\n')
				i++;
			continue ;
		}

		if (isSpecial(config[i]) || isspace(config[i]))
		{
			addToken(token, line, TOKEN_WORD);
			if (isSpecial(config[i]))
			{
				token.value.push_back(config[i]);
				addToken(token, line, getTokenType(config[i]));
			}
		}
		else
			token.value.push_back(config[i]);
	}
	return (_tokens);
}

void	Lexer::printToken()
{
	std::string	token[5] = {"WORD", "LBRACE", "RBRACE", "SEMICOLON", "EOF"};
	for (std::vector<Token>::iterator it = _tokens.begin(); it != _tokens.end(); ++it)
	{
		std::cout << "Type: " << token[it->type] << "	| Value: " << it->value << '\n';
	}
}

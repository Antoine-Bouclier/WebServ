#include <iostream>
#include "parser/Lexer.hpp"

typedef unsigned int	uint;

/* -------------------- */
/* -- CANONICAL FORM -- */
/* -------------------- */

Lexer::Lexer() {}

Lexer::Lexer(const Lexer& other) { *this = other; }

Lexer &Lexer::operator=(const Lexer& other)
{
	if (this != &other)
		_tokens = other._tokens;
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

void Lexer::addToken(Token& token, const unsigned int line, TokenType type)
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
	Token	token;
	uint	line = 1;

	_tokens.clear();

	for (size_t i = 0; i < config.size(); i++)
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
			if (i < config.size())
				line++;
			continue ;
		}

		if (isSpecial(config[i]) || isspace(static_cast<unsigned char>(config[i])))
		{
			addToken(token, line, TOKEN_WORD);
			if (isSpecial(config[i]))
			{
				token.value.push_back(config[i]);
				addToken(token, line, getTokenType(config[i]));
			}
		}
		else
			token.value += config[i];
	}
	addToken(token, line, TOKEN_WORD);
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

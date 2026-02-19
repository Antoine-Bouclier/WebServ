#include "Lexer.hpp"

Lexer::Lexer()
{

}
Lexer::Lexer(const Lexer& copy)
{

}
Lexer &Lexer::operator=(const Lexer& copy)
{

}

Lexer::~Lexer()
{

}

void	Lexer::PrintToken()
{
	for (std::vector<Token>::iterator it = _tokens.begin(); it != _tokens.end(); ++it)
	{
		std::cout << "Type: " << it->type << "	|	Value: " << it->value << '\n';
	}
}

TokenType	Lexer::GetTokenType(char c)
{
	if (c == '{')
		return (TOKEN_LBRACE);
	else if (c == '}')
		return (TOKEN_RBRACE);
	else if (c == ';')
		return (TOKEN_SEMICOLON);
	else if (c == '\0')
		return (TOKEN_EOF);
	else
		return (TOKEN_WORD);
}

std::vector<Token>	Lexer::Tokenize(const std::string &path)
{
	Token	token;
	for (unsigned long i = 0; i < path.size(); i++)
	{
		token.type = GetTokenType(path[i]);
		if ((isspace(path[i]) && token.value.size() != 0) || token.type != TOKEN_WORD)
		{
			_tokens.push_back(token);
			token.value.erase(token.value.begin(), token.value.end());
		}
		if (!isspace(path[i]))
			token.value.push_back(path[i]);
		if (token.value.size() == 1 && token.type != TOKEN_WORD)
		{
			_tokens.push_back(token);
			token.value.erase(token.value.begin(), token.value.end());
		}
	}
	if (token.value.size() != 0)
	{
		_tokens.push_back(token);
		token.value.erase(token.value.begin(), token.value.end());
	}
	return (_tokens);
}
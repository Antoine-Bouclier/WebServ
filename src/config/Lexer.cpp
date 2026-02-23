#include "Lexer.hpp"

Lexer::Lexer()
{

}
Lexer::Lexer(const Lexer& copy)
{
	(void)copy;
}
Lexer &Lexer::operator=(const Lexer& copy)
{
	(void)copy;
	return (*this);
}

Lexer::~Lexer()
{

}

void	Lexer::PrintToken()
{
	std::string	token[5] = {"WORD", "LBRACE", "RBRACE", "SEMICOLON", "EOF"};
	for (std::vector<Token>::iterator it = _tokens.begin(); it != _tokens.end(); ++it)
	{
		std::cout << "Type: " << token[it->type] << "	| Value: " << it->value << '\n';
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

bool	Lexer::isSpecial(char c)
{
	if (c == '{' || c ==  '}' || c ==  ';' || c ==  '\0')
		return (true);
	else
		return (false);
}

std::vector<Token>	Lexer::Tokenize(const std::string &path)
{
	Token	current_token;
	for (unsigned long i = 0; i < path.size(); i++)
	{
		if (path[i] == '#')
		{
			if (!current_token.value.empty())
			{
				current_token.type = TOKEN_WORD;
				_tokens.push_back(current_token);
				current_token.value.erase(current_token.value.begin(), current_token.value.end());
			}
			while (i < path.size() && path[i] != '\n')
				i++;
			continue ;
		}
		if (isSpecial(path[i]) || isspace(path[i]))
		{
			if (!current_token.value.empty())
			{
				current_token.type = TOKEN_WORD;
				_tokens.push_back(current_token);
				current_token.value.erase(current_token.value.begin(), current_token.value.end());
			}
			if (isSpecial(path[i]))
			{
				current_token.type = GetTokenType(path[i]);
				current_token.value.push_back(path[i]);
				_tokens.push_back(current_token);
				current_token.value.erase(current_token.value.begin(), current_token.value.end());
			}
		}
		else
			current_token.value.push_back(path[i]);
	}
	return (_tokens);
}

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <vector>

#include <sys/types.h>

enum TokenType
{
	TOKEN_WORD,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_SEMICOLON,
	TOKEN_EOF
};

struct Token
{
	TokenType	type;
	std::string	value;
};

#endif
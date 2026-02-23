#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <vector>
#include <cstring> // strerror()

#include <sys/types.h>
#include <sys/stat.h> // stat()

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
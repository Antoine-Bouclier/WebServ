#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <map>			// std::map
#include <vector>		// std::vector
#include <utility>		// std::pair
#include <cstring>		// strerror()
#include <cstdlib>		// atoi, atol
#include <sstream>		// stringtream
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>	// stat()
#include <stdint.h>		// uint16_t

#include <Exception.hpp>

enum Context {
	MAIN_CONTEXT,
	SERVER_CONTEXT,
	LOCATION_CONTEXT
};

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
	TokenType		type;
	std::string		value;
	unsigned int	line;
};

#endif

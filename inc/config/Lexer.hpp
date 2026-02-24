#ifndef LEXER_HPP
#define LEXER_HPP

#include "WebServ.hpp"

class Lexer
{
	private:
		std::vector<Token>	_tokens;

		TokenType	getTokenType(char c);
		bool	isSpecial(char c);
	public:
		/* -- Constructors -- */
		Lexer();
		Lexer(const Lexer& copy);
		Lexer &operator=(const Lexer& copy);
		~Lexer();

		/* -- DEBUG -- */
		void				printToken();

		/* -- Member function -- */
		std::vector<Token>	tokenize(const std::string &path);
};

#endif
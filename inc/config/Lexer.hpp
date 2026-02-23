#ifndef LEXER_HPP
#define LEXER_HPP

#include "WebServ.hpp"

class Lexer
{
	private:
		std::vector<Token>	_tokens;

		TokenType	GetTokenType(char c);
		bool	isSpecial(char c);
		void	pushAndErase(Token	current_token);
	public:
		/* -- Constructors -- */
		Lexer();
		Lexer(const Lexer& copy);
		Lexer &operator=(const Lexer& copy);
		~Lexer();

		/* -- DEBUG -- */
		void				PrintToken();

		/* -- Member function -- */
		std::vector<Token>	Tokenize(const std::string &path);
};

#endif
#ifndef LEXER_HPP
#define LEXER_HPP

#include "WebServ.hpp"

class Lexer
{
public:
	/* -- Constructors -- */
	Lexer();
	Lexer(const Lexer&);
	Lexer &operator=(const Lexer&);
	~Lexer();

	/* -- DEBUG -- */
	void				printToken(void);

	/* -- Member function -- */
	std::vector<Token>	tokenize(const std::string&);

private:
		std::vector<Token>	_tokens;

		/* -- Private methods -- */
		bool		isSpecial(char);
		TokenType	getTokenType(char);
		void		addToken(Token&, const uint, TokenType);
};

#endif

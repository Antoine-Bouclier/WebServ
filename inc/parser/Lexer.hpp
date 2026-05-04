#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>

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
		void		addToken(Token&, const unsigned int, TokenType);
};

#endif

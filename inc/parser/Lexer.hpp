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
	Lexer();
	Lexer(const Lexer&);
	Lexer &operator=(const Lexer&);
	~Lexer();

	void				printToken(void);

	std::vector<Token>	tokenize(const std::string&);

private:
		std::vector<Token>	_tokens;

		bool		isSpecial(char);
		TokenType	getTokenType(char);
		void		addToken(Token&, const unsigned int, TokenType);
};

#endif

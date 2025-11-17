#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>

enum TokenType
{
	TK_IDENTIFIER, //server, root, error_page
	TK_NUMBER, // 8080, 404, 10240
	TK_STRING, //"/var/www/html"
	TK_LBRACE, //{
	TK_RBRACE,//}
	TK_SEMICOLON,
	TK_EOF,
};

struct Token
{
	TokenType type;
	std::string value;
	int	line;
	int	col;
};

class Tokenizer
{
public:
	Tokenizer(std::string& source);
	std::vector<Token> tokenize();
private:
	const std::string& _source;
	std::size_t _pos;
	int _line;
	int _col;

	char peek() const;
	char get();
	bool eof() const;

	void skipWhitespaceAndComments();
	Token nextToken();

	Token tokenizeIdentifier();
	Token tokenizeString();
	Token tokenizeSymbol(); //{}
	//Token tokenizeNumber();
	//error()?
};

#endif

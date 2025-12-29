#ifndef CONFIGTOKENIZER_HPP
#define CONFIGTOKENIZER_HPP

#include <string>
#include <vector>

/**
 * @brief 
 * 
 * 
 * 
 * 
 * 
 */
namespace config{
	enum TokenType
	{
		TK_IDENTIFIER,
		TK_NUMBER,
		TK_LBRACE,
		TK_RBRACE,
		TK_SEMICOLON,
		TK_EOF,
		TK_STRING,
	};

	struct Token
	{
		TokenType 	type;
		std::string value;
		int			line;
		int			col;
	};

	class Tokenizer
	{
	public:
		Tokenizer(std::string& source);
		std::vector<Token> tokenize();

	private:
		const std::string& _source;
		std::size_t _pos;
		int 		_line;
		int 		_col;

		char peek() const;
		char get();
		bool eof() const;
		void skipWhitespaceAndComments();
		Token nextToken();
		Token tokenizeIdentifier();
		Token tokenizeSymbol();
		Token tokenizeString();
	};
}

#endif

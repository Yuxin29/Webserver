#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <iostream>

/**
 * @file ConfigTokenizer.hpp
 * @brief Lexical analyzer for webserver configuration files.
 *
 * This module converts a raw configuration file into
 * a sequence of typed tokens that can be consumed
 * by the configuration parser.
 * 
 *  @note
 * Tokenizer  →  Parser  →  Builder
 * (Lexing)      (Syntax)    (Runtime config)
 * 
 *  @example input
 * server {
 *   listen 8080;
 *   root /var/www/html;
 * }
 *  @example output
 * IDENTIFIER("server")
 * LBRACE("{")
 *
 * IDENTIFIER("listen")
 * NUMBER("8080")
 * SEMICOLON(";")
 *
 * IDENTIFIER("root")
 * IDENTIFIER("/var/www/html")
 * SEMICOLON(";")
 *
 * RBRACE("}")
 * EOF
 */
namespace config{
	// All supported token types in the configuration language.
	enum TokenType
	{
		TK_IDENTIFIER,		///< Keywords, paths, and directive names
		TK_NUMBER,			///< Numeric values consisting only of digits
		TK_LBRACE,			///< '{' beginning of a block
		TK_RBRACE,			///< '}' end of a block
		TK_SEMICOLON,		///< ';' directive terminator
		TK_EOF,				///< End-of-file marker
		TK_STRING,		 	///< Double-quoted string literal
	};

	// Represents a single lexical token.
	struct Token
	{
		TokenType 	type;	///< Token category
		std::string value;	///< Raw token value
		int			line;	///< Line number in source file
		int			col;	///< Column number in source file
	};

	/**
	 * @class Tokenizer
	 * @brief Performs lexical analysis on configuration files.
	 *
	 * The Tokenizer scans the input character-by-character
	 * and groups them into identifiers, numbers, symbols,
	 * and string literals.
	 *
	 * @note This class does not perform syntax or semantic validation.
	 */
	class Tokenizer
	{
	private:
		const std::string& _source;	 		///< Reference to the source string.
		std::size_t 		_pos;			///< Current position in the source.
		int 				_line;			///< Current line number.
		int 				_col;			///< Current column number.

		char 	peek() const;
		char	get();
		bool	eof() const;
		void	skipWhitespaceAndComments();
		Token	nextToken();
		Token	tokenizeIdentifier();
		Token	tokenizeSymbol();
		Token	tokenizeString();

	public:
		Tokenizer(std::string& source);
		std::vector<Token> tokenize();
	};
}
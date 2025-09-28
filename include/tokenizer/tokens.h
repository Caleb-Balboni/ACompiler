#ifndef TOKENS
#define TOKENS

#include <stdio.h>
#include "utils/arraylist.h"

typedef enum {
	UNKNOWN,
	IDENTIFIER,
	STRING_LIT,
	NUMBER_LIT,
	INTEGER, 
       	SEMICOLON,
	EQUALS,
	RETURN,
} Token_type;

typedef struct {
	Token_type type;
	const char* lexeme;
	size_t length;
	unsigned int col, row;

} Token; 

// creates a token based upon the given params
// @param type - the type of the token
// @param lexeme - the associated string
// @param length - the length of the string
// @param col - the column the token is in
// @param row - the row the token is in
Token* createToken(Token_type type, const char* lexeme, size_t length, unsigned int col, unsigned int row);
#endif 

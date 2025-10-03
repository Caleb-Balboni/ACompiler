#ifndef TOKENS
#define TOKENS

#include <stdio.h>
#include "utils/arraylist.h"

typedef enum {
	UNKNOWN,
  // LONGER TYPE LEXMES
  IDENTIFIER,
	STRING_LIT,
	NUMBER_LIT,
	INTEGER, 
  RETURN,
  // SINGULAR LEXMES
  SEMICOLON,
	EQUAL,
	LEFT_PAREN,
	RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  STAR,
  DIVIDE,
  GREATER,
  LESS,
  NOT,
  // DOUBLE CHAR LEXMES
  EQUAL_EQUAL,
  NOT_EQUAL,
  LESS_EQUAL,
  GREATER_EQUAL,
} Token_type;

typedef struct {
	Token_type type;
	char* lexeme;
	size_t length;
	unsigned int line, col;

} Token; 

#endif 

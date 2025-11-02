#ifndef TOKENS
#define TOKENS

#define MAX_LEXEME 64
#include <stdio.h>
#include "utils/arraylist.h"

typedef enum {
	T_UNKNOWN,
  T_EOF,
  // LONGER TYPE LEXMES
  T_IDENTIFIER,
  T_IF,
  T_ELSE,
  T_LET,
  T_CALL,
  T_FUNC,
  T_BYTE,
  T_WORD,
  T_DWORD,
  T_QWORD,
  T_STRING_LIT,
	T_NUMBER_LIT,
  T_RETURN,
  // SINGULAR LEXMES
  T_SEMICOLON,
  T_COLON,
	T_EQUAL,
	T_LEFT_PAREN,
	T_RIGHT_PAREN,
  T_LEFT_BRACE,
  T_RIGHT_BRACE,
  T_COMMA,
  T_AND,
  T_DOT,
  T_MINUS,
  T_PLUS,
  T_STAR,
  T_DIVIDE,
  T_DIVDIV,
  T_GREATER,
  T_LESS,
  T_NOT,
  // DOUBLE CHAR LEXMES
  T_EQUAL_EQUAL,
  T_NOT_EQUAL,
  T_LESS_EQUAL,
  T_GREATER_EQUAL,
} Token_type;

typedef struct {
	Token_type type;
	char lexeme[MAX_LEXEME];
	size_t length;
	unsigned int line, col;

} Token; 

#endif 

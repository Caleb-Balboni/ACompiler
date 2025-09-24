#ifndef TOKENS
#define TOKENS

#include <stdio.h>

typedef enum {
	SEMICOLON,
	RETURN,
} Token;

typedef struct {
	Token* tokens;
	size_t len;
} TokenList;


#endif 

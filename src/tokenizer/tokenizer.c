#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/detector.h"

Token* createToken(Token_type type, const char* lexeme, size_t length, unsigned int col, unsigned int row) {
	Token* temp = malloc(sizeof(Token) * 1);
	temp->type = type;
	temp->lexeme = lexeme;
	temp->length = length;
	temp->col = col;
	temp->row = row;
	return temp;
}

TokenList* tokenize(FILE* sourcefile) {
	return NULL;
}

Token* parseString(char* input, unsigned int column, unsigned int row) {
	Token* temp = malloc(sizeof(Token) * 1);
	temp->lexeme = input;	

	// first test if input is identifier
	if (isIdentifier(input)) {
		return createToken(IDENTIFIER, input, strlen(input), column, row);
	}	
	return NULL;
}	

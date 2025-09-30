#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tokenizer/tokenizer.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"
#include "utils/detector.h"

static hashtable_t* token_hash = NULL;

void initTokenMap(void) {
	
	token_hash = createHashTable(100);
	addHashTable(token_hash, "int", INTEGER); 
	addHashTable(token_hash, "return", RETURN);
}

Token* createToken(Token_type type, const char* lexeme, Tokenizer* tokenizer) {
	Token* temp = malloc(sizeof(Token) * 1);
	assert(temp);
	temp->type = type;
	temp->lexeme = strndup(lexeme, length);
	temp->length = length;
	temp->col = tokenizer->current;
	temp->row = tokenizer->line;
	return temp;
}

bool match(Tokenizer* tokenizer, char c) {
	if (tokenizer->cur == EOF) {
		return false;
	}
	
	if (tokenizer->cur != c) {
		retunr false;	
	}
	
	tokenizer->current += 1;
	return true;	
}

char advance(Tokenizer* tokenizer) {
	char temp = tokenizer->cur;
	tokenizer->cur = fgetsc(tokenizer->sourcefile);
	tokenizer->current += 1;
	return temp;
}

Token* scanToken(Tokenizer* tokenizer) {
	char c = advance(tokenizer);
	switch(c) {
		case '(':
			return createToken(LEFT_PARAN, c, tokenizer);
			break;
		case ')':
			return createToken(RIGHT_PARAN, c, tokenizer);
			break;
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			tokenizer->line += 1;
			break;
			
	}	

	// TODO
}

ArrayList* tokenize(FILE* sourcefile) {

	assert(sourcefile != NULL && "no inputted source file");
	initTokenMap();
	
	Tokenizer* tokenizer;
	tokenizer->sourcefile = sourcefile;
	tokenizer->start = 0;
	tokenizer->current = 0;
	tokenizer->line = 0;	
	ArrayList* tokens = initArrayList(100, sizeof(Token*));
	assert(tokens != NULL && "Token list was null");

	while((tokenizer->cur != EOF)) {
		tokenizer->start = tokenizer->current;
		addArrayList(tokens, scanToken(tokenizer);
	}
	
	free(token_hash);
	
	return array;
}


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
	addHashTable(token_hash, ";", SEMICOLON);
	addHashTable(token_hash, "=", EQUALS);
	addHashTable(token_hash, "int", INTEGER); 
	addHashTable(token_hash, "return", RETURN);
}

Token* createToken(Token_type type, const char* lexeme, size_t length, unsigned int col, unsigned int row) {
	Token* temp = malloc(sizeof(Token) * 1);
	assert(temp);
	temp->type = type;
	temp->lexeme = strndup(lexeme, length);
	temp->length = length;
	temp->col = col;
	temp->row = row;
	return temp;
}

bool isDelimiter(char input, char* delimiters) {

	for (int i = 0; i < strlen(delimiters); i++) {
		if (input == delimiters[i]) {
			return true;
		}
	}

	return false;
}

void tokenizeLine(const char* s, char* delims, ArrayList* out) {
    size_t i = 0, n = strlen(s);

    while (i < n) {
        // skip whitespace
        i += strspn(s + i, " \t\r\n");
        if (i >= n) break;

        // delimiter? -> 1-char token
        if (strchr(delims, (unsigned char)s[i])) {
            char d[2] = { s[i], '\0' };
            Token *t = parseString(d, 1, 1);
            appendArrayList(out, t);
	    printf("CREATED: type=%d lexeme='%s'\n", t->type, t->lexeme);
            i++;
            continue;
        }

        // word up to next delim or whitespace
        size_t len_delim  = strcspn(s + i, delims);
        size_t len_space  = strcspn(s + i, " \t\r\n");
        size_t len = len_delim < len_space ? len_delim : len_space;

        char buf[256];
        if (len >= sizeof buf) len = sizeof buf - 1; // clamp
        memcpy(buf, s + i, len);
        buf[len] = '\0';

        Token *t = parseString(buf, 1, 1);
        appendArrayList(out, t);
	printf("CREATED: type=%d lexeme='%s'\n", t->type, t->lexeme);
        i += len;
    }
}

ArrayList* tokenize(FILE* sourcefile) {

	assert(sourcefile != NULL && "no inputted source file");
	initTokenMap();
	
	ArrayList* array = initArrayList(100, sizeof(Token*));
	if (array == NULL) {
		printf("error allocating");
		return NULL;
	}
	char line[4096];
	char* delimiters = ";,(){}[]+-*/%<>!&|=.";
	while (fgets(line, sizeof line, sourcefile)) {
		tokenizeLine(line, delimiters, array);
	}

	free(token_hash);
	
	return array;
}

Token* parseString(char* input, unsigned int column, unsigned int row) {
	
	Token_type token = getHashTable(token_hash, input);

	if (isIdentifier(input) && token == UNKNOWN) {
		return createToken(IDENTIFIER, input, strlen(input), column, row);
	}

	if (isStringLit(input) && token == UNKNOWN) {
		return createToken(STRING_LIT, input, strlen(input), column, row);
	}

	if (isNumberLit(input) && token == UNKNOWN) {
		return createToken(NUMBER_LIT, input, strlen(input), column, row);
	}

	assert(token != UNKNOWN);
	return createToken(token, input, strlen(input), column, row);
}	

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tokenizer/tokenizer.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

static hashtable_t* token_hash = NULL;

void initTokenMap(void) {
	
	token_hash = createHashTable(100);
	addHashTable(token_hash, "int", INTEGER); 
	addHashTable(token_hash, "return", RETURN);
}

char* getCurWord(Tokenizer* tokenizer) {
  unsigned int length = tokenizer->cur_idx - tokenizer->start_idx;
  char* word = malloc(length + 1);
  unsigned long saved_idx = ftell(tokenizer->sourcefile);
  fseek(tokenizer->sourcefile, tokenizer->start_idx, SEEK_SET);
  fread(word, 1, length, tokenizer->sourcefile);
  word[length] = '\0';
  fseek(tokenizer->sourcefile, saved_idx, SEEK_SET);
  return word;
}

Token* createToken(Token_type type, Tokenizer* tokenizer) {
	Token* temp = malloc(sizeof(Token) * 1);
	assert(temp);
	temp->type = type;
  temp->length = tokenizer->cur_idx - tokenizer->start_idx;
  temp->lexeme = malloc(temp->length + 1);

  strcpy(temp->lexeme, getCurWord(tokenizer));

	temp->col = tokenizer->cur_idx;
	temp->line = tokenizer->cur_line;
	return temp;
}

Token* createIdentifer(Tokenizer* tokenizer) {
  char p = peek(tokenizer);
  while (isalpha(p) || isdigit(p)) {
    advance(tokenizer);
    p = peek(tokenizer);
  }

  Token_type type = getHashTable(token_hash, getCurWord(tokenizer));
  if (type != UNKNOWN) {
    return createToken(type, tokenizer);
  }
  return createToken(IDENTIFIER, tokenizer);
}

Token* createString(Tokenizer* tokenizer) {
  char p = peek(tokenizer);
  while(p != '\0' && p != '"') {
    advance(tokenizer);
    p = peek(tokenizer);
  }

  if (p == '\0') {
    return createToken(UNKNOWN, tokenizer);
  }

  tokenizer->start_idx += 1;
  Token* temp = createToken(STRING_LIT, tokenizer);
  advance(tokenizer);
  advance(tokenizer);
  return temp;
}

Token* createNumber(Tokenizer* tokenizer) {
  char p = peek(tokenizer);
  while (isdigit(p)) {
    advance(tokenizer);
    p = peek(tokenizer);
  }

  if (p == '.' && isdigit(peekNext(tokenizer))) {
    advance(tokenizer);
    p = peek(tokenizer);
    while (isdigit(p)) {
      advance(tokenizer);
    }
  }

  return createToken(NUMBER_LIT, tokenizer);
}

bool match(Tokenizer* tokenizer, char c) {
	if (tokenizer->cur_idx >= tokenizer->filesize) {
		return false;
	}
	
	if (tokenizer->cur != c) {
    return false;
	}
	
	tokenizer->cur_idx += 1;
	return true;	
}

char peek(Tokenizer* tokenizer) {
  if (tokenizer->cur_idx >= tokenizer->filesize) {
    return '\0';
  }

  return tokenizer->cur;
}

char peekNext(Tokenizer* tokenizer) {
  if (tokenizer->cur_idx + 1 >= tokenizer->filesize) {
    return '\0';
  }
  char c = fgetc(tokenizer->sourcefile);
  ungetc(c, tokenizer->sourcefile);
  return c;
}

char advance(Tokenizer* tokenizer) {
	char temp = tokenizer->cur;
	tokenizer->cur = fgetc(tokenizer->sourcefile);
	tokenizer->cur_idx += 1;
	return temp;
}

Token* scanToken(Tokenizer* tokenizer) {
	char c = advance(tokenizer);
	switch(c) {
		case '(':
			return createToken(LEFT_PAREN, tokenizer);
			break;
		case ')':
			return createToken(RIGHT_PAREN, tokenizer);
			break;
    case '{':
      return createToken(LEFT_BRACE, tokenizer);
      break;
    case '}':
      return createToken(RIGHT_BRACE, tokenizer);
      break;
    case ',':
      return createToken(COMMA, tokenizer);
      break;
    case '.':
      return createToken(DOT, tokenizer);
      break;
    case '-':
      return createToken(MINUS, tokenizer);
      break;
    case '+':
      return createToken(PLUS, tokenizer);
      break;
    case '*':
      return createToken(STAR, tokenizer);
      break;
    case '/':
      return createToken(DIVIDE, tokenizer);
      break;
    case '!':
      return createToken(match(tokenizer, '=') ? NOT_EQUAL : NOT, tokenizer);
      break;
    case '>':
      return createToken(match(tokenizer, '=') ? GREATER_EQUAL : GREATER, tokenizer);
      break;
    case '<':
      return createToken(match(tokenizer, '=') ? LESS_EQUAL : LESS, tokenizer);
      break;
    case '=':
      return createToken(match(tokenizer, '=') ? EQUAL_EQUAL : EQUAL, tokenizer);
      break;
    case ';':
      return createToken(SEMICOLON, tokenizer);
    case '"':
      return createString(tokenizer);
      break;
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			tokenizer->cur_line+= 1;
			break;
    default:
      if (isalpha(c)) {
        return createIdentifer(tokenizer);
      }
      
      if (isdigit(c)) {
        return createNumber(tokenizer);
      }
      printf("Unidentifiable type %s\n", getCurWord(tokenizer));
      exit(EXIT_FAILURE);
      break;
  }
  
  return NULL;
}

ArrayList* tokenize(FILE* sourcefile, unsigned long char_count) {

	assert(sourcefile != NULL && "no inputted source file");
	initTokenMap();
	
	Tokenizer* tokenizer;
	tokenizer->sourcefile = sourcefile;
  tokenizer->filesize = char_count;
  tokenizer->cur = fgetc(sourcefile);
	tokenizer->start_idx = 0;
	tokenizer->cur_idx = 0;
	tokenizer->cur_line = 0;	
	ArrayList* tokens = initArrayList(100);
	assert(tokens != NULL && "Token list was null");

	while(tokenizer->cur_idx < tokenizer->filesize) {
		tokenizer->start_idx = tokenizer->cur_idx;
    Token* temp = scanToken(tokenizer);
    if (temp != NULL) {
      appendArrayList(tokens, temp);
    }
	}
	
	free(token_hash);
	
	return tokens;
}


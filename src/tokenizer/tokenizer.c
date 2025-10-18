#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tokenizer/tokenizer.h"
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

static hashtable_t* token_hash = NULL;

Token_type* createTokenType(Token_type type) {
  Token_type* t = malloc(sizeof(Token_type));
  return t;
}

void initTokenMap(void) {
	
	token_hash = create_ht(100);
	add_ht(token_hash, "let", createTokenType(T_LET)); 
  add_ht(token_hash, "VOID", createTokenType(T_VOID));
  add_ht(token_hash, "BYTE", createTokenType(T_BYTE));
  add_ht(token_hash, "WORD", createTokenType(T_WORD));
  add_ht(token_hash, "DWORD", createTokenType(T_DWORD));
  add_ht(token_hash, "QWORD", createTokenType(T_QWORD));
  add_ht(token_hash, "return", createTokenType(T_RETURN));
  add_ht(token_hash, "if", createTokenType(T_IF));
  add_ht(token_hash, "else", createTokenType(T_ELSE));
  add_ht(token_hash, "func", createTokenType(T_FUNC));
}

void getCurWord(Tokenizer* tokenizer, char* buf) {
  unsigned int length = tokenizer->cur_idx - tokenizer->start_idx;
  char word[MAX_LEXEME];
  unsigned long saved_idx = ftell(tokenizer->sourcefile);
  fseek(tokenizer->sourcefile, tokenizer->start_idx, SEEK_SET);
  fread(word, 1, length, tokenizer->sourcefile);
  word[length] = '\0';
  strncpy(buf, word, MAX_LEXEME - 1);
  buf[MAX_LEXEME - 1] = '\0';
  fseek(tokenizer->sourcefile, saved_idx, SEEK_SET);
}

Token* createToken(Token_type type, Tokenizer* tokenizer) {
	Token* temp = malloc(sizeof(Token) * 1);
	assert(temp);
	temp->type = type;
  temp->length = tokenizer->cur_idx - tokenizer->start_idx;
  getCurWord(tokenizer, temp->lexeme);
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
  char buf[MAX_LEXEME];
  getCurWord(tokenizer, buf);
  Token_type* type = (Token_type*)get_ht(token_hash, buf);
  if (type != NULL) {
    return createToken(*type, tokenizer);
  }
  return createToken(T_IDENTIFIER, tokenizer);
}

Token* createString(Tokenizer* tokenizer) {
  char p = peek(tokenizer);
  while(p != '\0' && p != '"') {
    advance(tokenizer);
    p = peek(tokenizer);
  }

  if (p == '\0') {
    return createToken(T_UNKNOWN, tokenizer);
  }

  tokenizer->start_idx += 1;
  Token* temp = createToken(T_STRING_LIT, tokenizer);
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

  return createToken(T_NUMBER_LIT, tokenizer);
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
			return createToken(T_LEFT_PAREN, tokenizer);
			break;
		case ')':
			return createToken(T_RIGHT_PAREN, tokenizer);
			break;
    case '{':
      return createToken(T_LEFT_BRACE, tokenizer);
      break;
    case '}':
      return createToken(T_RIGHT_BRACE, tokenizer);
      break;
    case ',':
      return createToken(T_COMMA, tokenizer);
      break;
    case '.':
      return createToken(T_DOT, tokenizer);
      break;
    case '-':
      return createToken(T_MINUS, tokenizer);
      break;
    case '+':
      return createToken(T_PLUS, tokenizer);
      break;
    case '*':
      return createToken(T_STAR, tokenizer);
      break;
    case '/':
      return createToken(T_DIVIDE, tokenizer);
      break;
    case ':':
      return createToken(T_COLON, tokenizer);
      break;
    case '!':
      return createToken(match(tokenizer, '=') ? T_NOT_EQUAL : T_NOT, tokenizer);
      break;
    case '>':
      return createToken(match(tokenizer, '=') ? T_GREATER_EQUAL : T_GREATER, tokenizer);
      break;
    case '<':
      return createToken(match(tokenizer, '=') ? T_LESS_EQUAL : T_LESS, tokenizer);
      break;
    case '=':
      return createToken(match(tokenizer, '=') ? T_EQUAL_EQUAL : T_EQUAL, tokenizer);
      break;
    case ';':
      return createToken(T_SEMICOLON, tokenizer);
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
      char buf[MAX_LEXEME];
      getCurWord(tokenizer, buf);
      printf("Unidentifiable type %s\n", buf);
      exit(EXIT_FAILURE);
      break;
  }
  
  return NULL;
}

ArrayList* tokenize(FILE* sourcefile, unsigned long char_count) {

	assert(sourcefile != NULL && "no inputted source file");
	initTokenMap();
	
	Tokenizer* tokenizer = malloc(sizeof(Tokenizer));
	tokenizer->sourcefile = sourcefile;
  tokenizer->filesize = char_count;
  tokenizer->cur = fgetc(sourcefile);
	tokenizer->start_idx = 0;
	tokenizer->cur_idx = 0;
	tokenizer->cur_line = 0;	
	ArrayList* tokens = init_list(100);
	assert(tokens != NULL && "Token list was null");

	while(tokenizer->cur_idx < tokenizer->filesize) {
		tokenizer->start_idx = tokenizer->cur_idx;
    Token* temp = scanToken(tokenizer);
    if (temp != NULL) {
      add_list(tokens, temp);
    }
	}
  Token* eof = malloc(sizeof(Token));
  eof->type = T_EOF;
  strncpy(eof->lexeme, "eof", MAX_LEXEME - 1);
  eof->lexeme[MAX_LEXEME] = '\0';
  eof->length = 3;
  eof->line = -1;
  eof->col = -1;
  add_list(tokens, eof);	
  destroy_ht(token_hash);
  token_hash = NULL;
  free(tokenizer);
	return tokens;
}


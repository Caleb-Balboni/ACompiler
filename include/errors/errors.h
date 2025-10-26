#include <stdlib.h>
#include <stdio.h>
#include "parser/parser.h"
#include "tokenizer/tokens.h"

void compile_error(Token* t, const char* message);

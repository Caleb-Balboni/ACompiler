#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"
#include "tokens.h"

int main(int argc, char *argv[]) {

	FILE* source_file;
	source_file = fopen(argv[1], "r");
	tokenize(source_file);
	return 0;
}

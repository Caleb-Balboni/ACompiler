#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/hashtable.h"

int main(int argc, char *argv[]) {

	FILE* source_file;
	source_file = fopen(argv[1], "r");
	tokenize(source_file);

	// testing functions of hash table will move to a seperate test file
	hashtable_t* table = createHashTable(100);
	
	addHashTable(table, "test", RETURN);

	Token test = getHashTable(table, "test");
		
	printf("%d\n", test);

	bool removed = removeHashTable(table, "test");

	
	printf("%d", removed);
	return 0;
}

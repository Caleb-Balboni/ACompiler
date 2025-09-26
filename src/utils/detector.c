#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "utils/detector.h"

bool isNumberLit(char* input) {

	for (int i = 0; i < strlen(input); i++) {
		if (!isdigit(input[i])) {
			return false;
		}
	}

	return true;
}

bool isStringLit(char* input) {
	if (input[0] != '"' || input[strlen(input) - 1] != '"') {
		return false;
	}
	
	for (int i = 1; i < strlen(input) - 1; i++) {
		if (!isalpha(input[i])) {
			return false;
		}
	}

	return true;
}

bool isIdentifier(char* input) {

	for (int i = 0; i < strlen(input); i++) {
		if (!isalpha(input[i])) {
			return false;
		}
	}

	return true;

}

#ifndef DETECTOR
#define DETECTOR
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// checks if a given string is a number literal (can be decimal or int)
// @param input - the inputted string to check
// @return - true if it is an number, false otherwise 
bool isNumberLit(char* input);

// check if a given string is a literal string
// @param input - the inputted string to check
// @return - true if it is a literal string, false otherwise
bool isStringLit(char* input);

// checks if a given string is an indetifier
// @param input - the inputted string to check
// @return - true if it is an identifer, false otherwise
bool isIdentifier(char* input);

#endif

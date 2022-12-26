#include <stdbool.h>
#include <stdbool.h>
#include "API/CyclicLists.h"

#pragma once

/*
 * Returns a pointer to a new string allocated on the heap.
 */
char* StringAlloc(char*);

/*
 * Python-like string slice function
 */
int PythonicStringSlice(char*, int, int, char*);

/*
 * Python like string split.
 */
int PythonicStringSplit(char*, char*, t_CyclicList**, int);

/*
 * Split a string when the saparators are whitespaces such as: ' ', '\t' etc...
 */
int SplitStringWhitespaceSeparators(char*, t_CyclicList**, int);

/*
 *  Split a string when the separators are commas
 */
int StringSplitCommasSeparators(char*, t_CyclicList**);

/*
 * Whether string starts with a specific value
 */
bool Startswith(char*, char*);

/*
 * Whether string1 == string2
 */
bool StringsEqual(char*, char*);

/*
 * Remove the whitespaces from the beginning and the end of a string.
 */
int TrimString(char*, char*);

/*
 *  Get string line by a given index.
 */
int GetStringLine(char*, int, char*);

/*
 *  Replace whitespaces with spaces.
 */
int ReplaceWhitespacesWithSpaces(char*);

/*
 * Reberse a string, for example: ReverseString("abc") -> "cba"
 */
int ReverseString(char*, char*);


/*
 * Allocate a number on the heap.
 */
int* IntAlloc(int);


/*
* Match a regex pattern with a textual value.
 */
bool RegEqual(char* pattern, char* text);
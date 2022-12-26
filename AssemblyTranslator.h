#include "API/CyclicLists.h"

#pragma once


/*
 * Translate a list of raw strings (assembly code) to list of numbers (assembled code).
 */
int BinaryTranslate(t_CyclicList*, t_CyclicList*, t_CyclicList**, t_CyclicList**);

/*
 * Build the list of symbols for the given assembly source code.
 */
int BuildSymbolsList(t_CyclicList*, t_CyclicList**);
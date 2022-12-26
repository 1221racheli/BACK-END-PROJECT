#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Helpers.h"
#include <regex.h>


char* StringAlloc(char* string) {
    int string_length = strlen(string);
    char* result = malloc(string_length + 1);
    return strcpy(result, string);
}


char* get_first_non_separator_character(char* string, char* separator) {
    int separator_length = strlen(separator);
    while (string == strstr(string, separator)) {
        string += separator_length;
    }

    return string;
}


char* get_last_separator(char* string, char* separator) {
    int separator_length = strlen(separator);
    char* position = string + strlen(string) - 1;

    while (strstr(position - separator_length, separator) == position && position - separator_length >= position) {
        position -= separator_length;
    }

    return position + 1;
}

int PythonicStringSlice(char* string, int start, int end, char* result) {
    int string_length = strlen(string);

    if (end == -1) {
        end = string_length;
    }

    if (string_length != 0 && (start < 0 || start >= string_length || end < start || end > string_length + 1)) {
        return 1;
    }

    strncpy(result, string + start, end - start);
    result[end - start] = 0;
    return 0;
}

int SplitStringWhitespaceSeparators(char* string, t_CyclicList** result, int max_splits) {
    t_CyclicList* split_result;
    int return_code;

    ReplaceWhitespacesWithSpaces(string);
    return_code = PythonicStringSplit(string, " ", &split_result, max_splits);

    *result = split_result;
    return return_code;
}

int PythonicStringSplit(char* string, char* separator, t_CyclicList** result, int max_splits) {
    char split_item[4096];
    int splits_count = 0;
    char* position;

    t_CyclicList* split_items = CyclicListAlloc();

    do {
        splits_count++;
        string = get_first_non_separator_character(string, separator);

        position = strstr(string, separator);
        if (position == NULL) {
            strcpy(split_item, string);
        } else {
            PythonicStringSlice(string, 0, position - string, split_item);
        }

        AppendListWithNewElement(split_items, StringAlloc(split_item));
        string = position;
    } while ((max_splits == -1 || splits_count < max_splits) && position != NULL && strlen(string) > 0);

    if (splits_count == max_splits && string != NULL) {
        string = get_first_non_separator_character(string, separator);
        position = get_last_separator(string, separator);

        if (string != position) {
            PythonicStringSlice(string, 0, position - string, split_item);
            AppendListWithNewElement(split_items, StringAlloc(split_item));
        }
    }

    *result = split_items;
    return 0;
}

int StringSplitCommasSeparators(char* string, t_CyclicList** result) {
    t_CyclicList* split_result;
    t_CyclicList* stripped_result = CyclicListAlloc();
    t_CyclicList* cyclic_list;

    PythonicStringSplit(string, ",", &split_result, -1);

    cyclic_list = split_result;
    while (cyclic_list != NULL && DoesListContainElements(split_result)) {
        char* data = cyclic_list->data;
        TrimString(data, data);
        AppendListWithNewElement(stripped_result, StringAlloc(data));
        cyclic_list = cyclic_list->next_list;
    }

    *result = stripped_result;
    ListDealloc(split_result, free);
    return 0;
}

bool Startswith(char* string, char* substring) {
    return strstr(string, substring) == string;
}

bool StringsEqual(char* string1, char* string2) {
    return strcmp(string1, string2) == 0;
}

int ReverseString(char* string, char* result) {
    int i;
    int string_length = strlen(string);
    char* temp_string = malloc(string_length + 1);
    for (i = 0; i < strlen(string); i++) {
        temp_string[i] = string[string_length - i - 1];
    }
    temp_string[string_length] = 0;
    strcpy(result, temp_string);
    free(temp_string);
    return 0;
}

int GetStringLine(char* string, int idx, char* result) {
    bool is_last_line;
    int string_length = strlen(string);
    char* next_line = strstr(string, "\n");
    char* original_string = string;
    int i;

    for (i = 0; i < idx && next_line != NULL; i++) {
        string = next_line + 1;
        next_line = strstr(string, "\n");
    }
    if (i != idx) {
        return 1;
    }
    is_last_line = next_line == NULL && (string - original_string) <= string_length;
    if (is_last_line) {
        strcpy(result, string);
    } else {
        PythonicStringSlice(string, 0, next_line - string, result);
    }
    return 0;
}

int TrimString(char* string, char* result) {
    int end_idx;
    int start_idx;
    for (start_idx = 0; isspace(string[start_idx]); start_idx++) {}
    for (end_idx = strlen(string) - 1; isspace(string[end_idx]); end_idx--) {}
    return PythonicStringSlice(string, start_idx, end_idx + 1, result);
}


int ReplaceWhitespacesWithSpaces(char* string) {
    int string_len = strlen(string);
    int i;
    for (i = 0; i < string_len; i++) {
        if (isspace(string[i])) {
            string[i] = ' ';
        }
    }
    return 0;
}


int* IntAlloc(int number) {
    int* pointer = malloc(sizeof(int));
    *pointer = number;
    return pointer;
}


bool RegEqual(char* pattern, char* text) {
    int match_result;
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED);
    match_result = regexec(&regex, text, 0, NULL, 0);
    return match_result != REG_NOMATCH;
}


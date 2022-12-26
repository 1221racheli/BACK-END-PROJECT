#include <string.h>
#include <stdio.h>
#include "API/CyclicLists.h"
#include "API/LinesMetadataConsumer.h"
#include "Helpers.h"
#include "API/Core/Types.h"

#include "AssemblyTranslator.h"
#include "API/Core/Functions.h"

int CalculateStringInstructionSize(char* ins) {
    char* str_value;
    int res;
    t_CyclicList* split_result;

    ReplaceWhitespacesWithSpaces(ins);
    PythonicStringSplit(ins, " ", &split_result, 1);

    if (ListCountElements(split_result) != 2 || !RegEqual("^\\s*\".*\"\\s*$", split_result->next_list->data)) {
        printf("E: error using .string\n");
        exit(1);
    }

    str_value = split_result->next_list->data;
    TrimString(str_value, str_value);
    res = strlen(str_value) - 1;

    ListDealloc(split_result, free);
    return res;
}

int CalculateStructInstructionSize(char* ins) {
    t_CyclicList* split_result;
    t_CyclicList* commas_split_result;

    char* value_str;
    int res;

    SplitStringWhitespaceSeparators(ins, &split_result, 1);
    StringSplitCommasSeparators(split_result->next_list->data, &commas_split_result);

    if (ListCountElements(split_result) != 2 ||
        !RegEqual("^\\s*[\\+\\-]*[0123456789]*\\s*,\\s*\".*\"\\s*$", split_result->next_list->data)) {
        printf("E: error using '.struct'\n");
        exit(1);
    }

    value_str = commas_split_result->next_list->data;
    TrimString(value_str, value_str);
    res = strlen(value_str);

    ListDealloc(split_result, free);
    ListDealloc(commas_split_result, free);

    return res;
}

int BuildLabelGeneric(t_CyclicList* lines, char* prefix, t_CyclicList** res_label_list) {
    t_CyclicList* entry_label_list = CyclicListAlloc();
    char* code;
    char* label;

    t_CyclicList* cyclic_list = lines;
    while (cyclic_list != NULL && DoesListContainElements(lines)) {
        t_LineMetadata* data = cyclic_list->data;
        code = data->code;
        if (Startswith(code, prefix)) {
            ReplaceWhitespacesWithSpaces(code);
            code = strstr(code, " ");

            label = StringAlloc(code);
            TrimString(code, label);
            AppendListWithNewElement(entry_label_list, label);
        }
        cyclic_list = cyclic_list->next_list;
    }

    *res_label_list = entry_label_list;
    return 0;
}

int CalculateDataInstructionSize(char* ins) {
    int amount_of_blobs;

    t_CyclicList* split;
    t_CyclicList* parameterssplit;

    ReplaceWhitespacesWithSpaces(ins);
    PythonicStringSplit(ins, " ", &split, 1);

    if (ListCountElements(split) != 2) {
        printf("E: error using .data\n");
        exit(1);
    }

    StringSplitCommasSeparators(split->next_list->data, &parameterssplit);

    amount_of_blobs = ListCountElements(parameterssplit);
    ListDealloc(split, free);
    ListDealloc(parameterssplit, free);

    return amount_of_blobs;
}

int BuildExternalLabels(t_CyclicList* lines, t_CyclicList** results) {
    return BuildLabelGeneric(lines, ".extern", results);
}

int BuildEntryLabels(t_CyclicList* lines, t_CyclicList** result_list) {
    return BuildLabelGeneric(lines, ".entry", result_list);
}

t_Symbol* SymbolAllocFromEntries(char* label, int address, int attribute, t_CyclicList* entries) {
    t_CyclicList* attributes = CyclicListAlloc();
    if (CheckIfListOfStringsContainsString(entries, label)) {
        AppendListWithNewElement(attributes, IntAlloc(2));
    }

    AppendListWithNewElement(attributes, IntAlloc(attribute));
    return SymbolAlloc(
            label,
            address,
            attributes
    );
}

int BuildExternals(t_CyclicList* lines, t_CyclicList* symbol_table) {
    t_CyclicList* externals;
    t_CyclicList* cyclic_list;
    BuildExternalLabels(lines, &externals);

    cyclic_list = externals;
    while (cyclic_list != NULL && DoesListContainElements(externals)) {
        char* data = cyclic_list->data;
        t_CyclicList* attributes = CyclicListAlloc();
        AppendListWithNewElement(attributes, IntAlloc(3));
        AppendListWithNewElement(symbol_table, SymbolAlloc(StringAlloc(data), 0, attributes));
        cyclic_list = cyclic_list->next_list;
    }

    ListDealloc(externals, free);
    return 0;
}

int BuildSymbolsList(t_CyclicList* lines, t_CyclicList** result_symbols_table) {
    char* code;
    char* label;

    int data_counter = 0;
    int ins_counter = 100;

    t_Symbol* symbol;
    t_CyclicList* entries;
    t_CyclicList* symbol_table = CyclicListAlloc();
    t_CyclicList* cyclic_list;

    BuildExternals(lines, symbol_table);
    BuildEntryLabels(lines, &entries);

    cyclic_list = lines;
    while (cyclic_list != NULL && DoesListContainElements(lines)) {
        t_LineMetadata* data = cyclic_list->data;
        if (!Startswith(data->code, ".")) {
            if (data->label != NULL) {
                symbol = SymbolAllocFromEntries(data->label, ins_counter, 1, entries);
                AppendListWithNewElement(symbol_table, symbol);
            }
            ins_counter += CalculateInstructionWords(data->code);
        }
        cyclic_list = cyclic_list->next_list;
    }

    cyclic_list = lines;
    while (cyclic_list != NULL && DoesListContainElements(lines)) {
        t_LineMetadata* data = cyclic_list->data;
        code = data->code;
        label = data->label;

        if (Startswith(code, ".data")) {
            if (label != NULL) {
                symbol = SymbolAllocFromEntries(label, ins_counter + data_counter, 4, entries);
                AppendListWithNewElement(symbol_table, symbol);
            }
            data_counter += CalculateDataInstructionSize(code);

        } else if (Startswith(code, ".string")) {
            if (label != NULL) {
                symbol = SymbolAllocFromEntries(label, ins_counter + data_counter, 4, entries);
                AppendListWithNewElement(symbol_table, symbol);
            }
            data_counter += CalculateStringInstructionSize(code);
        } else if (Startswith(code, ".struct")) {
            if (label != NULL) {
                symbol = SymbolAllocFromEntries(label, ins_counter + data_counter, 4, entries);
                AppendListWithNewElement(symbol_table, symbol);
            }
            data_counter += CalculateStructInstructionSize(code);
        }
        cyclic_list = cyclic_list->next_list;
    }

    *result_symbols_table = symbol_table;
    return 0;
}


int BinaryTranslateStringWithQuotes(char* str, t_CyclicList* binary_translated_blobs) {
    int i;
    int binary_translated_blob;

    if (!RegEqual("^\\s*\"\\s*.*\"\\s*$", str)) {
        printf("E: error string has been provided\n");
        exit(1);
    }

    PythonicStringSlice(str, 1, strlen(str) - 1, str);

    for (i = 0; i < strlen(str); i++) {
        binary_translated_blob = (int) str[i];
        AppendListWithNewElement(binary_translated_blobs, IntAlloc(binary_translated_blob));
    }

    AppendListWithNewElement(binary_translated_blobs, IntAlloc(0));
    return 0;
}

int BinaryTranslateData(char* ins, t_CyclicList* binary_translated_blob_list) {
    int binary_translated_blob;

    t_CyclicList* splitted_res;
    t_CyclicList* parameterssplitted_res;
    t_CyclicList* cyclic_list;
    ReplaceWhitespacesWithSpaces(ins);
    PythonicStringSplit(ins, " ", &splitted_res, 1);

    if (ListCountElements(splitted_res) != 2) {
        printf("E: error using .data\n");
        exit(1);
    }

    StringSplitCommasSeparators(splitted_res->next_list->data, &parameterssplitted_res);

    cyclic_list = parameterssplitted_res;
    while (cyclic_list != NULL && DoesListContainElements(parameterssplitted_res)) {
        char* data = cyclic_list->data;
        binary_translated_blob = atoi(data);
        AppendListWithNewElement(binary_translated_blob_list, IntAlloc(binary_translated_blob));
        cyclic_list = cyclic_list->next_list;
    }

    ListDealloc(splitted_res, free);
    ListDealloc(parameterssplitted_res, free);

    return 0;
}

int BinaryTranslateString(char* ins, t_CyclicList* binary_translated_blob_list) {
    t_CyclicList* splitted_res;

    ReplaceWhitespacesWithSpaces(ins);
    PythonicStringSplit(ins, " ", &splitted_res, 1);

    if (ListCountElements(splitted_res) != 2) {
        printf("E: error using .string\n");
        exit(1);
    }

    BinaryTranslateStringWithQuotes(splitted_res->next_list->data, binary_translated_blob_list);
    ListDealloc(splitted_res, free);
    return 0;
}

int BinaryTranslateStruct(char* ins, t_CyclicList* binary_translated_blobs) {
    char* num;
    char* str;

    t_CyclicList* splitted_res;
    t_CyclicList* parameterssplitted_res;

    SplitStringWhitespaceSeparators(ins, &splitted_res, 1);

    if (ListCountElements(splitted_res) != 2) {
        printf("E: error using .struct\n");
        exit(1);
    }

    StringSplitCommasSeparators(splitted_res->next_list->data, &parameterssplitted_res);
    if (ListCountElements(parameterssplitted_res) != 2) {
        printf("E: error using .struct\n");
        exit(1);
    }

    num = parameterssplitted_res->data;
    str = parameterssplitted_res->next_list->data;

    TrimString(num, num);
    TrimString(str, str);

    AppendListWithNewElement(binary_translated_blobs, IntAlloc(atoi(num)));
    BinaryTranslateStringWithQuotes(str, binary_translated_blobs);

    ListDealloc(splitted_res, free);
    ListDealloc(parameterssplitted_res, free);

    return 0;
}

int SaveExternalSymbolUsage(t_CyclicList* parameterlist, int ins_counter) {
    int usage_address = ins_counter + 1;

    t_CyclicList* cyclic_list = parameterlist;
    while (cyclic_list != NULL && DoesListContainElements(parameterlist)) {
        t_Parameter* data = cyclic_list->data;
        if (data->referenced_sym != NULL && SymbolExternal(*data->referenced_sym)) {
            AppendListWithNewElement(data->referenced_sym->usage_addresses, IntAlloc(usage_address));
        }
        usage_address += ParameterResultSizeByMode(data->mode);
        cyclic_list = cyclic_list->next_list;
    }

    return 0;
}

int BInaryTranslateNoramlInstruction(t_LineMetadata* line, int ins_counter, t_CyclicList* symbols_table,
                                     t_CyclicList* binary_translated_inss, char* error_log) {
    t_CyclicList* cyclic_list;
    t_Instruction ins;
    t_CyclicList* binary_translated_res;

    if (InstructionStringToStruct(
            line->code,
            symbols_table,
            &ins,
            error_log) != 0) {
        return 1;
    }

    SaveExternalSymbolUsage(ins.parameters, ins_counter);
    TranslateInstructionToAssembly(ins, &binary_translated_res);

    cyclic_list = binary_translated_res;
    while (cyclic_list != NULL && DoesListContainElements(binary_translated_res)) {
        int* data = cyclic_list->data;
        AppendListWithNewElement(binary_translated_inss, IntAlloc(*data));
        cyclic_list = cyclic_list->next_list;
    }

    ListDealloc(binary_translated_res, free);
    return 0;
}

int BinaryTranslate(
        t_CyclicList* lines,
        t_CyclicList* symbols_table,
        t_CyclicList** result_blob_list,
        t_CyclicList** result_data_blob_list
) {
    t_LineMetadata* line;
    int ins_counter = 100;
    int return_code = 0;
    char error_log[1024];

    t_CyclicList* binary_translated_blobs = CyclicListAlloc();
    t_CyclicList* binary_translated_data_blobs = CyclicListAlloc();


    t_CyclicList* cyclic_list = lines;
    while (cyclic_list != NULL && DoesListContainElements(lines)) {
        t_LineMetadata* data = cyclic_list->data;
        line = data;

        if (Startswith(line->code, ".string")) {
            BinaryTranslateString(line->code, binary_translated_data_blobs);
        } else if (Startswith(line->code, ".data")) {
            BinaryTranslateData(line->code, binary_translated_data_blobs);
        } else if (Startswith(line->code, ".struct")) {
            BinaryTranslateStruct(line->code, binary_translated_data_blobs);
        } else if (!Startswith(line->code, ".")) {
            if (BInaryTranslateNoramlInstruction(line, ins_counter, symbols_table, binary_translated_blobs,
                                                 error_log) != 0) {
                return_code = 1;
                printf("E: Line idx %d: %s\n", line->idx + 1, error_log);
            } else {
                ins_counter += CalculateInstructionWords(line->code);
            }
        }
        cyclic_list = cyclic_list->next_list;
    }

    *result_blob_list = binary_translated_blobs;
    *result_data_blob_list = binary_translated_data_blobs;

    return return_code;
}

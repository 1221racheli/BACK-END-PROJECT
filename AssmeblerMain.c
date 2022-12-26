#include <string.h>
#include <stdio.h>
#include "API/CyclicLists.h"
#include "API/Core/Functions.h"
#include "API/LinesMetadataConsumer.h"
#include "AssemblyTranslator.h"
#include "API/OutputProducers.h"

/*
 * Produce the output files
 */
void ProduceFiles(const char* file_name, t_CyclicList* symbol_table, char* filepath);

/*
 * Validate that arguments are passed correctly to the program.
 */
void CheckArguments(int argc, char* const* argv);

/*
 * Add ".obj" extension to the given file path.
 */
void AddObjectExtension(char* filepath) { strncat(filepath, ".obj", 4096 - 1); }

/*
 * Add ".am" extension to the given file path.
 */
void AddAMExtension(char* filepath) { strncat(filepath, ".am", 4096 - 1); }

/*
 * Add ".as" extension to the given file path.
 */
void AddAsExtension(char* filepath) { strncat(filepath, ".as", 4096 - 1); }

/*
 * Add ".ent" extension to the given file path.
 */
void AddEntExtension(char* filepath) { strncat(filepath, ".ent", 4096 - 1); }

/*
 * Add ".ext" extension to the given file path.
 */
void AddExtExtension(char* filepath) { strncat(filepath, ".ext", 4096 - 1); }

int StartBinaryTranslator(char* file_name) {
    t_CyclicList* lines;
    t_CyclicList* symbol_table;
    t_CyclicList* binary_translated_inss;
    t_CyclicList* binary_translated_data;
    char filepath[4096];

    InstructionMetaListAlloc();

    strcpy(filepath, file_name);
    AddAsExtension(filepath);

    if (ConsumeLinesToCyclicList(filepath, &lines) != 0) {
        printf("E: Error reading source file (.as file)\n");
        return 1;
    }

    strcpy(filepath, file_name);
    AddAMExtension(filepath);

    if (ProcessConsumedSourceFile(lines, filepath) != 0) {
        printf("E: error in macro expension process.\n");
    }

    ListDealloc(lines, (void (*)(void*)) LineMetaDealloc);
    ConsumeLinesToCyclicList(filepath, &lines);
    BuildSymbolsList(lines, &symbol_table);

    if (BinaryTranslate(lines, symbol_table, &binary_translated_inss, &binary_translated_data) != 0) {
        printf("E: error in assemblying process.\n");
        return 1;
    }

    strcpy(filepath, file_name);
    AddObjectExtension(filepath);

    if (ProduceObjectFile(binary_translated_inss, binary_translated_data, filepath) != 0) {
        printf("E: Error while producing objects.\n");
        return 1;
    }

    ProduceFiles(file_name, symbol_table, filepath);

    return 0;
}

void ProduceFiles(const char* file_name, t_CyclicList* symbol_table, char* filepath) {
    strcpy(filepath, file_name);
    AddEntExtension(filepath);

    if (ProduceEntriesFile(symbol_table, filepath) != 0) {
        printf("E: Error producing entries\n");
    }

    strcpy(filepath, file_name);
    AddExtExtension(filepath);

    if (ProduceExternalFile(symbol_table, filepath) != 0) {
        printf("E: Error producing externals file\n");
    }
}

void CheckArguments(int argc, char* const* argv) {
    if (argc <= 1) {
        printf("Source files should be provided: %s file1 file2 ...\n", argv[0]);
        exit(1);
    }
}

int main(int argc, char** argv) {
    int i;
    CheckArguments(argc, argv);
    for (i = 1; i < argc; i++) {
        StartBinaryTranslator(argv[i]);
    }
    return 0;
}
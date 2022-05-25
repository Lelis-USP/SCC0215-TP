#pragma once

#include <stdio.h>
#include <stdint.h>

enum Command {
    PARSE_AND_SERIALIZE = 1,
    DESERIALIZE_AND_PRINT = 2,
    DESERIALIZE_FILTER_AND_PRINT = 3,
    DESERIALIZE_SEARCH_RRN_AND_PRINT = 4
};

enum FileType {
    TYPE1 = 1,
    TYPE2 = 2
};

typedef struct CommandArgs {
    enum Command command;
    enum FileType fileType;
    char* sourceFile;
    char* destFile;
    FILE* source;
    void* specificData;
} CommandArgs;

typedef struct SearchByRRNArgs {
   uint64_t rrn;
} SearchByRRNArgs;

typedef struct FilterArgs {
    char* key;
    char* value;
    struct FilterArgs* next;
} FilterArgs;

CommandArgs* new_command_args(enum Command command);
void destroy_command_args(CommandArgs* args);

void execute(FILE* data_in);
CommandArgs* read_command(FILE* source);

void c_parse_and_serialize(CommandArgs* args);
void c_deserialize_and_print(CommandArgs* args);
void c_deserialize_filter_and_print(CommandArgs* args);
void c_deserialize_search_rrn_and_print(CommandArgs* args);



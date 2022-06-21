/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>
#include <stdio.h>
#include "../struct/registry.h"

#define MIN_COMMAND 1
#define MAX_COMMAND 4

enum Command {
    PARSE_AND_SERIALIZE = 1,
    DESERIALIZE_AND_PRINT = 2,
    DESERIALIZE_FILTER_AND_PRINT = 3,
    DESERIALIZE_SEARCH_RRN_AND_PRINT = 4
};

typedef struct CommandArgs {
    enum Command command;
    RegistryType registry_type;
    char* source_file;
    char* dest_file;
    FILE* source;
    void* specific_data;
} CommandArgs;

typedef struct SearchByRRNArgs {
    uint64_t rrn;
} SearchByRRNArgs;

typedef struct FilterArgs {
    char* key;
    char* value;
    void* parsed_value;
    struct FilterArgs* next;
} FilterArgs;

static const char ID_FIELD_NAME[] = "id";
static const char ANO_FIELD_NAME[] = "ano";
static const char QTT_FIELD_NAME[] = "qtt";
static const char SIGLA_FIELD_NAME[] = "sigla";
static const char CIDADE_FIELD_NAME[] = "cidade";
static const char MARCA_FIELD_NAME[] = "marca";
static const char MODELO_FIELD_NAME[] = "modelo";


CommandArgs* new_command_args(enum Command command);
void destroy_command_args(CommandArgs* args);

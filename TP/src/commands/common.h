/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "../index/index.h"
#include "../struct/registry.h"

// Consts //
#define MIN_COMMAND 1
#define MAX_COMMAND 10

enum Command {
    PARSE_AND_SERIALIZE = 1,
    DESERIALIZE_AND_PRINT = 2,
    DESERIALIZE_FILTER_AND_PRINT = 3,
    DESERIALIZE_SEARCH_RRN_AND_PRINT = 4,
    BUILD_LINEAR_INDEX_FROM_REGISTRY = 5,
    REMOVE_REGISTRY_WITH_LINEAR_INDEX = 6,
    INSERT_REGISTRY_WITH_LINEAR_INDEX = 7,
    UPDATE_REGISTRY_WITH_LINEAR_INDEX = 8,
    BUILD_BTREE_INDEX_FROM_REGISTRY = 9,
    QUERY_REGISTRY_WITH_BTREE_INDEX = 10,

    // Implementation not requested
    INSERT_REGISTRY_WITH_BTREE_INDEX = 11,
    REMOVE_REGISTRY_WITH_BTREE_INDEX = 12,
    UPDATE_REGISTRY_WITH_BTREE_INDEX = 13
};

// Field names for input parsing
static const char ID_FIELD_NAME[] = "id";
static const char ANO_FIELD_NAME[] = "ano";
static const char QTT_FIELD_NAME[] = "qtt";
static const char SIGLA_FIELD_NAME[] = "sigla";
static const char CIDADE_FIELD_NAME[] = "cidade";
static const char MARCA_FIELD_NAME[] = "marca";
static const char MODELO_FIELD_NAME[] = "modelo";

// Structs //

typedef struct CommandArgs {
    enum Command command;
    RegistryType registry_type;
    IndexType index_type;
    char* primary_file;
    char* secondary_file;
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

typedef struct RemovalTarget {
    FilterArgs* indexed_filter_args;
    FilterArgs* unindexed_filter_args;
} RemovalTarget;

typedef struct RemovalArgs {
    uint32_t n_removals;
    RemovalTarget* removal_targets;
} RemovalArgs;

typedef struct InsertionTarget {
    int32_t id;
    int32_t ano;
    int32_t qtt;
    char sigla[REGISTRY_SIGLA_SIZE];
    char* cidade;
    char* marca;
    char* modelo;
} InsertionTarget;

typedef struct InsertionArgs {
    uint32_t n_insertions;
    InsertionTarget* insertion_targets;
} InsertionArgs;

typedef struct UpdateTarget {
    FilterArgs* indexed_filter_args;
    FilterArgs* unindexed_filter_args;

    bool update_id;
    int32_t id;
    bool update_ano;
    int32_t ano;
    bool update_qtt;
    int32_t qtt;
    bool update_sigla;
    char sigla[REGISTRY_SIGLA_SIZE];
    bool update_cidade;
    char* cidade;
    bool update_marca;
    char* marca;
    bool update_modelo;
    char* modelo;
} UpdateTarget;

typedef struct UpdateArgs {
    uint32_t n_updates;
    UpdateTarget* update_targets;
} UpdateArgs;

typedef struct SearchByIDArgs {
    int32_t id;
} SearchByIDArgs;

/**
 * Create command args struct
 * @param command target command
 * @return the struct ptr
 */
CommandArgs* new_command_args(enum Command command);

/**
 * Destroy command args struct
 * @param args target struct
 */
void destroy_command_args(CommandArgs* args);

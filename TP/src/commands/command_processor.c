/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "command_processor.h"

#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "common.h"
#include "../const/const.h"
#include "../utils/provided_functions.h"
#include "../utils/utils.h"
#include "../exception/exception.h"

//#define COMMANDS_BUFFER_SIZE 512

/**
 * Read and execute the command in the given file
 * @param data_in data input
 */
void execute(FILE* data_in) {
    CommandArgs* args = read_command(data_in);

    if (args == NULL) {
        return;
    }

    switch (args->command) {
        case PARSE_AND_SERIALIZE:
            c_parse_and_serialize(args);
            break;
        case DESERIALIZE_AND_PRINT:
            c_deserialize_and_print(args);
            break;
        case DESERIALIZE_FILTER_AND_PRINT:
            c_deserialize_filter_and_print(args);
            break;
        case DESERIALIZE_SEARCH_RRN_AND_PRINT:
            c_deserialize_direct_access_rrn_and_print(args);
            break;
        case BUILD_INDEX_FROM_REGISTRY:
            c_build_index_from_registry(args);
            break;
        case REMOVE_REGISTRY:
            c_remove_registry(args);
            break;
        case INSERT_REGISTRY:
            c_insert_registry(args);
            break;
        case UPDATE_REGISTRY:
            c_update_registry(args);
            break;
    }

    destroy_command_args(args);
}

// Reading Utilities //

/**
 * Read a 32-bit integer
 * @param source source file
 * @return the read integer
 */
int32_t read_integer_field(FILE* source) {
    char buffer[COMMANDS_BUFFER_SIZE];
    scan_quote_string(buffer);
    size_t value_len = strnlen(buffer, COMMANDS_BUFFER_SIZE);

    if (value_len == 0) {
        return -1;
    }

    return (int32_t) strtol(buffer, NULL, 10);
}

/**
 * Read a string field (possibly quoted, or NULL)
 * @param source source file
 * @return read string
 */
char* read_string_field(FILE* source) {
    char buffer[COMMANDS_BUFFER_SIZE];
    scan_quote_string(buffer);
    size_t value_len = strnlen(buffer, COMMANDS_BUFFER_SIZE);
    if (value_len == 0) {
        return NULL;
    }

    char* str = malloc((value_len + 1) * sizeof (char));
    memcpy(str, buffer, value_len * sizeof (char));
    str[value_len] = '\0';
    return str;
}

/**
 * Read a raw string (unquoted)
 * @param source source file
 * @return read string
 */
char* read_string_raw(FILE* source) {
    char buffer[COMMANDS_BUFFER_SIZE];
    read_buffer_string(source, buffer);
    size_t field_len = strnlen(buffer, COMMANDS_BUFFER_SIZE);
    char* str = malloc(field_len + 1 * sizeof(char));
    memcpy(str, buffer, field_len);
    str[field_len] = '\0';
    return str;
}

/**
 * Read a sigla field
 * @param source source file
 * @param sigla destination ptr
 */
void read_sigla_field(FILE* source, char* sigla) {
    char buffer[COMMANDS_BUFFER_SIZE];
    scan_quote_string(buffer);
    size_t value_len = strnlen(buffer, COMMANDS_BUFFER_SIZE);
    if (value_len == 0) {
        // Load NULL sigla
        for (uint32_t i = 0; i < REGISTRY_SIGLA_SIZE; i++) {
            sigla[i] = FILLER_BYTE[0];
        }
        return;
    }

    // Load sigla
    memcpy(sigla, buffer, min(REGISTRY_SIGLA_SIZE, value_len));

    // Fill remainings with NULLs
    for (size_t j = value_len; j < REGISTRY_SIGLA_SIZE; j++) {
        sigla[j] = FILLER_BYTE[0];
    }
}

/**
 * Read the path for the secondary file
 * @param source source file
 * @param args command args ptr
 */
void read_secondary_file_path(FILE* source, CommandArgs* args) {
    args->secondary_file = read_string_raw(source) ;
}

// Read and parse commands //
/**
 * Read command information from the given file
 * @param source command source
 * @return command information
 */
CommandArgs* read_command(FILE* source) {
    // Reading buffer //
    char buffer[COMMANDS_BUFFER_SIZE];

    // Retrieve command id //
    uint32_t command;
    fscanf(source, "%u", &command);

    // Validate command //
    ex_assert(command >= MIN_COMMAND && command <= MAX_COMMAND, EX_COMMAND_PARSE_ERROR);

    // Create base args //
    CommandArgs* args = new_command_args(command);

    // Read file type //
    read_buffer_string(source, buffer);

    // Check if a valid file type was inserted
    args->registry_type = UNKNOWN;
    if (strncasecmp("tipo1", buffer, 6) == 0) {
        args->registry_type = FIX_LEN;
    } else if (strncasecmp("tipo2", buffer, 6) == 0) {
        args->registry_type = VAR_LEN;
    }

    // Invalid registry type
    if (args->registry_type == UNKNOWN) {
        puts(EX_COMMAND_PARSE_ERROR);
        destroy_command_args(args);
        return NULL;
    }

    // Read primary file path //
    read_buffer_string(source, buffer);
    size_t path_len = strnlen(buffer, COMMANDS_BUFFER_SIZE);
    args->primary_file = calloc(path_len + 1, sizeof(char));
    memcpy(args->primary_file, buffer, path_len);

    // Handle command-specific params //

    switch (args->command) {
        case BUILD_INDEX_FROM_REGISTRY:
        case PARSE_AND_SERIALIZE:
            read_secondary_file_path(source, args);
            break;

        case DESERIALIZE_AND_PRINT:
            break;

        case DESERIALIZE_FILTER_AND_PRINT:
            ; // This is not a typo
            // Read number of filters to read
            uint32_t n_filters;
            fscanf(source, "%u", &n_filters);

            // Build filter list
            FilterArgs* filter_tail = NULL;

            for (uint32_t i = 0; i < n_filters; i++) {
                FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                new_filter->next = NULL;
                new_filter->parsed_value = NULL;

                // Field name
                new_filter->key = read_string_raw(source);
                // Field value
                new_filter->value = read_string_field(source);

                // Update list filter_tail ref
                if (filter_tail == NULL) {
                    args->specific_data = new_filter;
                } else {
                    filter_tail->next = new_filter;
                }

                // Update tail ref
                filter_tail = new_filter;
            }
            break;

        case DESERIALIZE_SEARCH_RRN_AND_PRINT:
            ; // This is not a typo
            SearchByRRNArgs* rrn_args;
            // Read RRN value
            rrn_args = malloc(sizeof(struct SearchByRRNArgs));
            rrn_args->rrn = 0;
            fscanf(source, "%lu", &rrn_args->rrn);
            args->specific_data = rrn_args;
            break;

        case REMOVE_REGISTRY:
            // Load secondary file path
            read_secondary_file_path(source, args);
            RemovalArgs* removal_args = malloc(sizeof (struct RemovalArgs));
            args->specific_data = removal_args;

            // Load n removals
            fscanf(source, "%u", &removal_args->n_removals);

            // Allocate removal targets
            removal_args->removal_targets = calloc(removal_args->n_removals, sizeof (struct RemovalTarget));

            // Read every removal filter
            for (uint32_t i = 0; i < removal_args->n_removals; i++) {
                // Number of filtered fields
                uint32_t n_fields;
                fscanf(source, "%u", &n_fields);

                // Tail refs
                FilterArgs* indexed_tail = NULL;
                FilterArgs* unindexed_tail = NULL;

                // Read each field
                for (uint32_t j = 0; j < n_fields; j++) {
                    FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                    new_filter->next = NULL;
                    new_filter->parsed_value = NULL;

                    // Read field name
                    new_filter->key = read_string_raw(source);
                    // Read field value
                    new_filter->value = read_string_field(source);

                    // Check if field is indexed
                    if (strcmp(ID_FIELD_NAME, new_filter->key) == 0) {
                        // Update list filter_tail ref
                        if (indexed_tail == NULL) {
                            removal_args->removal_targets[i].indexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }
                        // Update tail ref
                        indexed_tail = new_filter;
                    } else {
                        // Update list filter_tail ref
                        if (unindexed_tail == NULL) {
                            removal_args->removal_targets[i].unindexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }
                        // Update tail ref
                        unindexed_tail = new_filter;
                    }
                }
            }
            break;

        case INSERT_REGISTRY:
            // Load secondary file path
            read_secondary_file_path(source, args);

            // Allocate insertion args
            InsertionArgs* insertion_args = malloc(sizeof (struct InsertionArgs));
            args->specific_data = insertion_args;

            // Read amount of insertions
            fscanf(source, "%u", &insertion_args->n_insertions);

            // Allocate insertion target's array
            insertion_args->insertion_targets = calloc(insertion_args->n_insertions, sizeof (struct InsertionTarget));

            for (uint32_t i = 0; i < insertion_args->n_insertions; i++) {
                // ID
                insertion_args->insertion_targets[i].id = read_integer_field(source);
                // Ano
                insertion_args->insertion_targets[i].ano = read_integer_field(source);
                // Qtt
                insertion_args->insertion_targets[i].qtt = read_integer_field(source);
                // Sigla
                read_sigla_field(source, insertion_args->insertion_targets[i].sigla);
                // Cidade
                insertion_args->insertion_targets[i].cidade = read_string_field(source);
                // Marca
                insertion_args->insertion_targets[i].marca= read_string_field(source);
                // Modelo
                insertion_args->insertion_targets[i].modelo= read_string_field(source);
            }
            break;

        case UPDATE_REGISTRY:
            // Load secondary file path
            read_secondary_file_path(source, args);

            // Allocate insertion args
            UpdateArgs* update_args = malloc(sizeof (struct UpdateArgs));
            args->specific_data = update_args;

            // Read amount of insertions
            fscanf(source, "%u", &update_args->n_updates);

            // Allocate insertion target's array
            update_args->update_targets= calloc(update_args->n_updates, sizeof (struct UpdateTarget));

            // Load each update
            for (uint32_t i = 0; i < update_args->n_updates; i++) {
                // Load update filters //
                uint32_t n_fields;
                fscanf(source, "%u", &n_fields);

                FilterArgs* indexed_tail = NULL;
                FilterArgs* unindexed_tail = NULL;

                for (uint32_t j = 0; j < n_fields; j++) {
                    FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                    new_filter->next = NULL;
                    new_filter->parsed_value = NULL;

                    // Read field name
                    new_filter->key = read_string_raw(source);
                    // Read field value
                    new_filter->value = read_string_field(source);

                    // Check if field is indexed
                    if (strcmp(ID_FIELD_NAME, new_filter->key) == 0) {
                        // Update list filter_tail ref
                        if (indexed_tail == NULL) {
                            update_args->update_targets[i].indexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }
                        // Update tail ref
                        indexed_tail = new_filter;
                    } else {
                        // Update list filter_tail ref
                        if (unindexed_tail == NULL) {
                            update_args->update_targets[i].unindexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }
                        // Update tail ref
                        unindexed_tail = new_filter;
                    }
                }

                // Load fields //
                fscanf(source, "%u", &n_fields);

                for (uint32_t j = 0; j < n_fields; j++) {
                    // Read field name
                    read_buffer_string(source, buffer);

                    if (strcmp(buffer, ID_FIELD_NAME) == 0) {
                        update_args->update_targets[i].id = read_integer_field(source);
                        update_args->update_targets[i].update_id = true;
                    } else if (strcmp(buffer, ANO_FIELD_NAME) == 0) {
                        update_args->update_targets[i].ano = read_integer_field(source);
                        update_args->update_targets[i].update_ano = true;
                    } else if (strcmp(buffer, QTT_FIELD_NAME) == 0) {
                        update_args->update_targets[i].qtt = read_integer_field(source);
                        update_args->update_targets[i].update_qtt = true;
                    } else if (strcmp(buffer, SIGLA_FIELD_NAME) == 0) {
                        read_sigla_field(source, update_args->update_targets[i].sigla);
                        update_args->update_targets[i].update_sigla = true;
                    } else if (strcmp(buffer, CIDADE_FIELD_NAME) == 0) {
                        update_args->update_targets[i].cidade = read_string_field(source);
                        update_args->update_targets[i].update_cidade = true;
                    } else if (strcmp(buffer, MARCA_FIELD_NAME) == 0) {
                        update_args->update_targets[i].marca = read_string_field(source);
                        update_args->update_targets[i].update_marca = true;
                    } else if (strcmp(buffer, MODELO_FIELD_NAME) == 0) {
                        update_args->update_targets[i].modelo= read_string_field(source);
                        update_args->update_targets[i].update_modelo= true;
                    }
                }
            }
            break;
    }

    return args;
}

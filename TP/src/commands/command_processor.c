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

void read_output_file_path(FILE* source, CommandArgs* args) {
    char buffer[512];
    fscanf(source, "%511s", buffer);// Read up to buffer size or separator
    size_t dest_path_len = strnlen(buffer, 512);
    args->dest_file = calloc(dest_path_len + 1, sizeof(char));
    memcpy(args->dest_file, buffer, dest_path_len);
}

/**
 * Read command information from the given file
 * @param source command source
 * @return command information
 */
CommandArgs* read_command(FILE* source) {
    // Retrieve command
    uint32_t command;
    fscanf(source, "%u", &command);

    // Validate command
    ex_assert(command >= MIN_COMMAND && command <= MAX_COMMAND, EX_COMMAND_PARSE_ERROR);

    // Create base args
    CommandArgs* args = new_command_args(command);

    // Read file type
    char buffer[512];               // Static buffer, should be enough for most cases
    fscanf(source, "%511s", buffer);// Read up to buffer size or separator

    // Check if a valid file type was inserted
    args->registry_type = UNKNOWN;
    if (strncasecmp("tipo1", buffer, 6) == 0) {
        args->registry_type = FIX_LEN;
    } else if (strncasecmp("tipo2", buffer, 6) == 0) {
        args->registry_type = VAR_LEN;
    }

    if (args->registry_type == UNKNOWN) {
        puts(EX_COMMAND_PARSE_ERROR);
        destroy_command_args(args);
        return NULL;
    }

    // Read input file path
    fscanf(source, "%511s", buffer);// Read up to buffer size or separator
    size_t path_len = strnlen(buffer, 512);
    args->source_file = calloc(path_len + 1, sizeof(char));
    memcpy(args->source_file, buffer, path_len);

    // Handle command-specific params
    uint32_t n_filters;
    SearchByRRNArgs* rrn_args;

    switch (args->command) {
        case BUILD_INDEX_FROM_REGISTRY:
        case PARSE_AND_SERIALIZE:
            read_output_file_path(source, args);
            break;

        case DESERIALIZE_AND_PRINT:
            break;

        case DESERIALIZE_FILTER_AND_PRINT:
            fscanf(source, "%u", &n_filters);

            // Build filter list
            FilterArgs* tail = NULL;
            for (uint32_t i = 0; i < n_filters; i++) {
                FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                new_filter->next = NULL;
                new_filter->parsed_value = NULL;

                // Read field name
                fscanf(source, "%511s", buffer);
                size_t field_len = strnlen(buffer, 512);
                new_filter->key = calloc(field_len + 1, sizeof(char));
                memcpy(new_filter->key, buffer, field_len);

                // Read value
                scan_quote_string(buffer);
                size_t value_len = strnlen(buffer, 512);
                new_filter->value = calloc(value_len + 1, sizeof(char));
                memcpy(new_filter->value, buffer, value_len);

                // Update list tail ref
                if (tail == NULL) {
                    args->specific_data = new_filter;
                } else {
                    tail->next = new_filter;
                }

                tail = new_filter;
            }
            break;

        case DESERIALIZE_SEARCH_RRN_AND_PRINT:
            // Read RRN value
            rrn_args = malloc(sizeof(struct SearchByRRNArgs));
            rrn_args->rrn = 0;
            fscanf(source, "%lu", &rrn_args->rrn);
            args->specific_data = rrn_args;
            break;

        case REMOVE_REGISTRY:
            // Load "output file" path
            read_output_file_path(source, args);
            RemovalArgs* removal_args = malloc(sizeof (struct RemovalArgs));
            args->specific_data = removal_args;

            // Load n removals
            fscanf(source, "%u", &removal_args->n_removals);

            // Allocate removal targets
            removal_args->removal_targets = calloc(removal_args->n_removals, sizeof (struct RemovalTarget));

            for (uint32_t i = 0; i < removal_args->n_removals; i++) {
                uint32_t n_fields;
                fscanf(source, "%u", &n_fields);

                FilterArgs* indexed_tail = NULL;
                FilterArgs* unindexed_tail = NULL;

                for (uint32_t j = 0; j < n_fields; j++) {
                    FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                    new_filter->next = NULL;
                    new_filter->parsed_value = NULL;

                    // Read field name
                    fscanf(source, "%511s", buffer);
                    size_t field_len = strnlen(buffer, 512);
                    new_filter->key = calloc(field_len + 1, sizeof(char));
                    memcpy(new_filter->key, buffer, field_len);

                    // Read value
                    scan_quote_string(buffer);
                    size_t value_len = strnlen(buffer, 512);
                    new_filter->value = calloc(value_len + 1, sizeof(char));
                    memcpy(new_filter->value, buffer, value_len);

                    // Check if field is indexed
                    if (strncmp(ID_FIELD_NAME, new_filter->key, 512) == 0) {
                        // Update list tail ref
                        if (indexed_tail == NULL) {
                            removal_args->removal_targets[i].indexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }

                        indexed_tail = new_filter;
                    } else {
                        // Update list tail ref
                        if (unindexed_tail == NULL) {
                            removal_args->removal_targets[i].unindexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }

                        unindexed_tail = new_filter;
                    }
                }
            }
            break;
        case INSERT_REGISTRY:
            // Load "output file" path
            read_output_file_path(source, args);

            // Allocate insertion args
            InsertionArgs* insertion_args = malloc(sizeof (struct InsertionArgs));
            args->specific_data = insertion_args;

            // Read amount of insertions
            fscanf(source, "%u", &insertion_args->n_insertions);

            // Allocate insertion target's array
            insertion_args->insertion_targets = calloc(insertion_args->n_insertions, sizeof (struct InsertionTarget));

            for (uint32_t i = 0; i < insertion_args->n_insertions; i++) {
                // ID
                scan_quote_string(buffer);
                size_t value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].id = -1;
                } else {
                    insertion_args->insertion_targets[i].id = (int32_t) strtol(buffer, NULL, 10);
                }


                // Ano
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].ano = -1;
                } else {
                    insertion_args->insertion_targets[i].ano = (int32_t) strtol(buffer, NULL, 10);
                }

                // Qtt
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].qtt = -1;
                } else {
                    insertion_args->insertion_targets[i].qtt = (int32_t) strtol(buffer, NULL, 10);
                }


                // Sigla
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    // Load NULL sigla
                    for (uint32_t j = 0; j < REGISTRY_SIGLA_SIZE; j++) {
                        insertion_args->insertion_targets[i].sigla[j] = FILLER_BYTE[0];
                    }
                } else {
                    // Load sigla
                    memcpy(insertion_args->insertion_targets[i].sigla, buffer, min(REGISTRY_SIGLA_SIZE, value_len));

                    // Fill remaining with NULLs
                    for (size_t j = value_len; j < REGISTRY_SIGLA_SIZE; j++) {
                        insertion_args->insertion_targets[i].sigla[j] = FILLER_BYTE[0];
                    }
                }

                // Cidade
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].cidade = NULL;
                } else {
                    insertion_args->insertion_targets[i].cidade = malloc((value_len + 1) * sizeof (char));
                    memcpy(insertion_args->insertion_targets[i].cidade, buffer, value_len * sizeof (char));
                    insertion_args->insertion_targets[i].cidade[value_len] = '\0';
                }

                // Marca
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].marca = NULL;
                } else {
                    insertion_args->insertion_targets[i].marca = malloc((value_len + 1) * sizeof (char));
                    memcpy(insertion_args->insertion_targets[i].marca, buffer, value_len * sizeof (char));
                    insertion_args->insertion_targets[i].marca[value_len] = '\0';
                }

                // Modelo
                scan_quote_string(buffer);
                value_len = strnlen(buffer, 512);
                if (value_len == 0) {
                    insertion_args->insertion_targets[i].modelo = NULL;
                } else {
                    insertion_args->insertion_targets[i].modelo = malloc((value_len + 1) * sizeof (char));
                    memcpy(insertion_args->insertion_targets[i].modelo, buffer, value_len * sizeof (char));
                    insertion_args->insertion_targets[i].modelo[value_len] = '\0';
                }
            }
            break;
        case UPDATE_REGISTRY:
            // Load "output file" path
            read_output_file_path(source, args);

            // Allocate insertion args
            UpdateArgs* update_args = malloc(sizeof (struct UpdateArgs));
            args->specific_data = update_args;

            // Read amount of insertions
            fscanf(source, "%u", &update_args->n_updates);

            // Allocate insertion target's array
            update_args->update_targets= calloc(update_args->n_updates, sizeof (struct UpdateTarget));

            for (uint32_t i = 0; i < update_args->n_updates; i++) {
                // Load filters
                uint32_t n_fields;
                fscanf(source, "%u", &n_fields);

                FilterArgs* indexed_tail = NULL;
                FilterArgs* unindexed_tail = NULL;

                for (uint32_t j = 0; j < n_fields; j++) {
                    FilterArgs* new_filter = malloc(sizeof(struct FilterArgs));
                    new_filter->next = NULL;
                    new_filter->parsed_value = NULL;

                    // Read field name
                    fscanf(source, "%511s", buffer);
                    size_t field_len = strnlen(buffer, 512);
                    new_filter->key = calloc(field_len + 1, sizeof(char));
                    memcpy(new_filter->key, buffer, field_len);

                    // Read value
                    scan_quote_string(buffer);
                    size_t value_len = strnlen(buffer, 512);
                    new_filter->value = calloc(value_len + 1, sizeof(char));
                    memcpy(new_filter->value, buffer, value_len);

                    // Check if field is indexed
                    if (strncmp(ID_FIELD_NAME, new_filter->key, 512) == 0) {
                        // Update list tail ref
                        if (indexed_tail == NULL) {
                            update_args->update_targets[i].indexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }

                        indexed_tail = new_filter;
                    } else {
                        // Update list tail ref
                        if (unindexed_tail == NULL) {
                            update_args->update_targets[i].unindexed_filter_args = new_filter;
                        } else {
                            unindexed_tail->next = new_filter;
                        }

                        unindexed_tail = new_filter;
                    }
                }

                // Load fields
                update_args->update_targets[i].update_id = false;
                update_args->update_targets[i].update_ano = false;
                update_args->update_targets[i].update_qtt = false;
                update_args->update_targets[i].update_sigla = false;
                update_args->update_targets[i].update_cidade = false;
                update_args->update_targets[i].update_marca = false;
                update_args->update_targets[i].update_modelo = false;

                fscanf(source, "%u", &n_fields);

                for (uint32_t j = 0; j < n_fields; j++) {
                    // Read field name
                    fscanf(source, "%511s", buffer);

                    if (strcmp(buffer, ID_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_id = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].id = -1;
                        } else {
                            update_args->update_targets[i].id = (int32_t) strtol(buffer, NULL, 10);
                        }
                    } else if (strcmp(buffer, ANO_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_ano = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].ano = -1;
                        } else {
                            update_args->update_targets[i].ano = (int32_t) strtol(buffer, NULL, 10);
                        }
                    } else if (strcmp(buffer, QTT_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_qtt = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].qtt = -1;
                        } else {
                            update_args->update_targets[i].qtt = (int32_t) strtol(buffer, NULL, 10);
                        }
                    } else if (strcmp(buffer, SIGLA_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_sigla = true;
                        if (value_len == 0) {
                            // Load NULL sigla
                            for (uint32_t k = 0; k < REGISTRY_SIGLA_SIZE; k++) {
                                update_args->update_targets[i].sigla[k] = FILLER_BYTE[0];
                            }
                        } else {
                            // Load sigla
                            memcpy(update_args->update_targets[i].sigla, buffer, min(REGISTRY_SIGLA_SIZE, value_len));

                            // Fill remaining with NULLs
                            for (size_t k = value_len; k < REGISTRY_SIGLA_SIZE; k++) {
                                update_args->update_targets[i].sigla[k] = FILLER_BYTE[0];
                            }
                        }
                    } else if (strcmp(buffer, CIDADE_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_cidade = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].cidade = NULL;
                        } else {
                            update_args->update_targets[i].cidade = malloc((value_len + 1) * sizeof (char));
                            memcpy(update_args->update_targets[i].cidade, buffer, value_len * sizeof (char));
                            update_args->update_targets[i].cidade[value_len] = '\0';
                        }
                    } else if (strcmp(buffer, MARCA_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_marca = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].marca = NULL;
                        } else {
                            update_args->update_targets[i].marca = malloc((value_len + 1) * sizeof (char));
                            memcpy(update_args->update_targets[i].marca, buffer, value_len * sizeof (char));
                            update_args->update_targets[i].marca[value_len] = '\0';
                        }
                    } else if (strcmp(buffer, MODELO_FIELD_NAME) == 0) {
                        scan_quote_string(buffer);
                        size_t value_len = strnlen(buffer, 512);
                        update_args->update_targets[i].update_modelo = true;
                        if (value_len == 0) {
                            update_args->update_targets[i].modelo = NULL;
                        } else {
                            update_args->update_targets[i].modelo = malloc((value_len + 1) * sizeof (char));
                            memcpy(update_args->update_targets[i].modelo, buffer, value_len * sizeof (char));
                            update_args->update_targets[i].modelo[value_len] = '\0';
                        }
                    }
                }
            }
            break;
    }

    return args;
}

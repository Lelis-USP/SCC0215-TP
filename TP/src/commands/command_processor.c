/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "command_processor.h"

#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "common.h"
#include "../utils/provided_functions.h"
#include "../exception/exception.h"


/**
 * Read and execute the command in the given file
 * @param data_in data input
 */
void execute(FILE* data_in) {
    CommandArgs* args = read_command(data_in);

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
    }

    destroy_command_args(args);
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
    ex_assert(command >= MIN_COMMAND, EX_COMMAND_PARSE_ERROR);
    ex_assert(command <= MAX_COMMAND, EX_COMMAND_PARSE_ERROR);

    // Create base args
    CommandArgs* args = new_command_args(command);

    // Read file type
    char buffer[512];               // Static buffer, should be enough for most cases
    fscanf(source, "%511s", buffer);// Read up to buffer size or separator

    // Check if a valid file type was inserted
    args->registry_type = FIX_LEN;
    if (strncasecmp("tipo2", buffer, 6) == 0) {
        args->registry_type = VAR_LEN;
    } else {
        ex_assert(strncasecmp("tipo1", buffer, 6) == 0, EX_COMMAND_PARSE_ERROR);// If not "tipo2", assert that the input was "tipo1"
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
        case PARSE_AND_SERIALIZE:
            // Read output file
            fscanf(source, "%511s", buffer);// Read up to buffer size or separator
            size_t dest_path_len = strnlen(buffer, 512);
            args->dest_file = calloc(dest_path_len + 1, sizeof(char));
            memcpy(args->dest_file, buffer, dest_path_len);
            break;

        case DESERIALIZE_AND_PRINT:
            break;

        case DESERIALIZE_FILTER_AND_PRINT:
            scanf("%u", &n_filters);

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
    }

    return args;
}

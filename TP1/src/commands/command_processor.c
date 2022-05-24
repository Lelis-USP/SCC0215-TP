#include "command_processor.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "const/const.h"
#include "struct/t1_struct.h"
#include "struct/t2_struct.h"
#include "utils/provided_functions.h"
#include "utils/csv_parser.h"

CommandArgs* new_command_args(enum Command command) {
    CommandArgs* args = malloc(sizeof(struct CommandArgs));

    args->sourceFile = NULL;
    args->destFile = NULL;
    args->source = NULL;
    args->specificData = NULL;

    if (command == DESERIALIZE_SEARCH_RRN_AND_PRINT) {
        SearchByRRNArgs* rrnArgs = malloc(sizeof (struct SearchByRRNArgs));
        rrnArgs->rrn = -1;
        args->specificData = rrnArgs;
    }

    return args;
}

void destroy_command_args(CommandArgs* args) {
    if (args == NULL) return;

    free(args->sourceFile);
    free(args->destFile);

    if (args->specificData != NULL) {
        // RRN cleanup
        if (args->command == DESERIALIZE_SEARCH_RRN_AND_PRINT) {
            free(args->specificData);
        }

        // Filter cleanup
        if (args->command == DESERIALIZE_FILTER_AND_PRINT) {
            FilterArgs* cur = (FilterArgs*) args->specificData;
            while (cur != NULL) {
                FilterArgs* next = cur->next;
                free(cur->key);
                free(cur->value);
                free(cur);
                cur = next;
            }
        }
    }

    free(args);
}

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
            c_deserialize_search_rrn_and_print(args);
            break;
    }

    destroy_command_args(args);
}

CommandArgs* read_command(FILE* source) {
    // Retrieve command
    uint32_t command;
    fscanf(source, "%u", &command);

    // Validate command
    assert(command >= 1);
    assert(command <= 4);

    // Create base args
    CommandArgs* args = new_command_args(command);

    // Read file type
    char buffer[512]; // Static buffer, should be enough for most cases
    fscanf(source, "%511s", buffer); // Read up to buffer size or separator

    // Check if a valid file type was inserted
    args->fileType = TYPE2;
    if (strncasecmp("tipo1", buffer, 6) == 0) {
        args->fileType = TYPE1;
    }
    assert(strncasecmp("tipo2", buffer, 6) == 0); // If not "tipo1", assert that the input was "tipo2"

    // Read input file path
    fscanf(source, "%511s", buffer); // Read up to buffer size or separator
    size_t path_len = strnlen(buffer, 512);
    args->sourceFile = calloc(path_len + 1, sizeof(char));
    memcpy(args->sourceFile, buffer, path_len);


    // Handle command-specific params
    switch (args->command) {
        case PARSE_AND_SERIALIZE:
            // Read output file
            fscanf(source, "%511s", buffer); // Read up to buffer size or separator
            size_t dest_path_len = strnlen(buffer, 512);
            args->destFile = calloc(dest_path_len + 1, sizeof(char));
            memcpy(args->destFile, buffer, dest_path_len);
            break;
        case DESERIALIZE_AND_PRINT:
            break;
        case DESERIALIZE_FILTER_AND_PRINT:
            uint32_t n_filters;
            scanf("%u", &n_filters);

            FilterArgs* tail = NULL;
            for (uint32_t i = 0; i  < n_filters; i++) {
                FilterArgs* new_filter = malloc(sizeof (struct FilterArgs));
                new_filter->next = NULL;

                // Read field name
                fscanf(source, "%511s", buffer);
                size_t field_len = strnlen(buffer, 512);
                new_filter->key = calloc(field_len + 1, sizeof (char));
                memcpy(new_filter->key, buffer, field_len);

                // Read value
                scan_quote_string(buffer);
                size_t value_len = strnlen(buffer, 512);
                new_filter->value = calloc(value_len + 1, sizeof (char));
                memcpy(new_filter->value, buffer, value_len);

                // Update list tail ref
                if (tail == NULL) {
                    args->specificData = new_filter;
                } else {
                    tail->next = new_filter;
                }

                tail = new_filter;
            }
            break;
        case DESERIALIZE_SEARCH_RRN_AND_PRINT:
            SearchByRRNArgs* rrn_args = malloc(sizeof(struct SearchByRRNArgs));
            fscanf(source, "%llu", &rrn_args->rrn);
            args->specificData = rrn_args;
            break;
    }

    return args;
}

void c_parse_and_serialize(CommandArgs* args) {
    assert(args->sourceFile != NULL);
    assert(args->destFile != NULL);

    // Open CSV file
    FILE* csv_file = fopen(args->sourceFile, "r");
    assert(csv_file != NULL);

    // Load CSV Content
    CSVContent* csv_content = read_csv(csv_file, true);
    fclose(csv_file);

    // Write contents into the file
    FILE* dest_file = fopen(args->destFile, "wb");
    if (args->fileType == TYPE1) {
        // Write default header with a bad status
        T1Header header = DEFAULT_T1_HEADER; // Copy default T1Header
        header.status = STATUS_BAD;
        t1_write_header(&header, dest_file);

        // STOPPED HERE

        // Update status at beginning
        header.status = STATUS_GOOD;
        fseek(dest_file, 0, SEEK_SET);
        t1_write_header(&header, dest_file);
    } else {

    }
    fclose(dest_file);


    destroy_csvcontent(csv_content); // Free CSVContent's memory
}

void c_deserialize_and_print(CommandArgs* args);
void c_deserialize_filter_and_print(CommandArgs* args);
void c_deserialize_search_rrn_and_print(CommandArgs* args);

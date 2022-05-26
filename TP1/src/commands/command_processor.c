#include "command_processor.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "const/const.h"
#include "struct/t1_struct.h"
#include "struct/t2_struct.h"
#include "utils/provided_functions.h"
#include "utils/csv_parser.h"
#include "utils/registry_builder.h"

CommandArgs* new_command_args(enum Command command) {
    CommandArgs* args = malloc(sizeof(struct CommandArgs));

    args->command = command;
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
    } else {
        assert(strncasecmp("tipo2", buffer, 6) == 0); // If not "tipo1", assert that the input was "tipo2"
    }

    // Read input file path
    fscanf(source, "%511s", buffer); // Read up to buffer size or separator
    size_t path_len = strnlen(buffer, 512);
    args->sourceFile = calloc(path_len + 1, sizeof(char));
    memcpy(args->sourceFile, buffer, path_len);

    // Handle command-specific params
    uint32_t n_filters;
    SearchByRRNArgs* rrn_args;
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
            rrn_args = malloc(sizeof(struct SearchByRRNArgs));
            fscanf(source, "%lu", &rrn_args->rrn);
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

        // Write registries
        CSVLine* current_line = csv_content->head_line;
        while (current_line != NULL) {
            T1Registry* registry = t1_build_from_csv_line(csv_content, current_line);
            t1_write_registry(registry, dest_file);
            current_line = current_line->next;
            header.proxRRN++;
            t1_destroy_registry(registry);
        }

        // Update status at beginning
        header.status = STATUS_GOOD;
        fseek(dest_file, 0, SEEK_SET);
        t1_write_header(&header, dest_file);
    } else {
        // Write default header with a bad status
        T2Header header = DEFAULT_T2_HEADER; // Copy default T1Header
        header.status = STATUS_BAD;
        t2_write_header(&header, dest_file);

        // Write registries
        CSVLine* current_line = csv_content->head_line;
        while (current_line != NULL) {
            T2Registry* registry = t2_build_from_csv_line(csv_content, current_line);
            size_t written_bytes = t2_write_registry(registry, dest_file);
            current_line = current_line->next;
            header.proxByteOffset += (int64_t) written_bytes;
            t2_destroy_registry(registry);
        }

        // Update status at beginning
        header.status = STATUS_GOOD;
        fseek(dest_file, 0, SEEK_SET);
        t2_write_header(&header, dest_file);
    }
    fclose(dest_file);

    binarioNaTela(args->destFile);

    destroy_csvcontent(csv_content); // Free CSVContent's memory
}

/**
 * Print a fixed length string
 * @param desc target string
 * @param n string length
 */
void print_fixed_len_str(char* desc, size_t n) {
    for (size_t i = 0; i < n; i++) {
        putchar(desc[i]);
    }
}

// Macro for printing a column description
#define print_column_description(desc) print_fixed_len_str(desc, sizeof(desc)/sizeof(char))

/**
 * Print a given type 1 registry into stdout
 *
 * @param header current file header
 * @param registry current registry
 */
void t1_print_registry(T1Header* header, T1Registry* registry) {
    // Marca
    print_column_description(header->desC6);
    if (registry->marca == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->marca);
    }

    // Modelo
    print_column_description(header->desC7);
    if (registry->modelo == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->modelo);
    }

    // Ano Fabricacao
    print_column_description(header->desC2);
    if (registry->ano == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry->ano);
    }

    // Cidade
    print_column_description(header->desC5);
    if (registry->cidade == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->cidade);
    }

    // Qtt
    print_column_description(header->desC3);
    if (registry->qtt == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry->qtt);
    }

    putchar('\n');
}


/**
 * Print a given type 2 registry into stdout
 *
 * @param header current file header
 * @param registry current registry
 */
void t2_print_registry(T2Header* header, T2Registry* registry) {
    // Marca
    print_column_description(header->desC6);
    if (registry->marca == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->marca);
    }

    // Modelo
    print_column_description(header->desC7);
    if (registry->modelo == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->modelo);
    }

    // Ano Fabricacao
    print_column_description(header->desC2);
    if (registry->ano == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry->ano);
    }

    // Cidade
    print_column_description(header->desC5);
    if (registry->cidade == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry->cidade);
    }

    // Qtt
    print_column_description(header->desC3);
    if (registry->qtt == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry->qtt);
    }

    putchar('\n');
}

void c_deserialize_and_print(CommandArgs* args) {
    assert(args->sourceFile != NULL);

    FILE* file = fopen(args->sourceFile, "rb");
    if (args->fileType == TYPE1) {
        T1Header* header = t1_new_header();
        size_t read_bytes = t1_read_header(header, file);

        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            puts(FILE_ERROR_MSG);
        } else {
            T1Registry* registry = t1_new_registry();
            for (uint32_t i = 0; i < header->proxRRN; i++) {
                read_bytes += t1_read_registry(registry, file);

                if (registry == NULL || registry->removido == REMOVED) {
                    continue;
                }

                t1_print_registry(header, registry);
                printed = true;
            }
            t1_destroy_registry(registry);

            if (!printed) {
                puts(EMPTY_REGISTRY_MSG);
            }
        }

        t1_destroy_header(header);
    } else {
        T2Header* header = t2_new_header();
        size_t read_bytes = t2_read_header(header, file);

        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            puts(FILE_ERROR_MSG);
        } else {

            T2Registry* registry = t2_new_registry();
            while (read_bytes < header->proxByteOffset) {
                read_bytes += t2_read_registry(registry, file);

                if (registry == NULL || registry->removido == REMOVED) {
                    continue;
                }

                t2_print_registry(header, registry);
                printed = true;
            }
            t2_destroy_registry(registry);

            if (!printed) {
                puts(EMPTY_REGISTRY_MSG);
            }
        }

        t2_destroy_header(header);
    }
    fclose(file);
}

void c_deserialize_filter_and_print(CommandArgs* args) {}
void c_deserialize_search_rrn_and_print(CommandArgs* args) {}

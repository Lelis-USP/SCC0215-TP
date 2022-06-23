/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "commands.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../const/const.h"
#include "../exception/exception.h"
#include "../utils/csv_parser.h"
#include "../utils/provided_functions.h"
#include "../utils/registry_loader.h"
#include "common.h"


// Commands //

typedef struct CSVParseArgs {
    Header* header;
    Registry* registry;
    FILE* dest_file;
} CSVParseArgs;

void parse_csv_line(int idx, CSVLine* line, void* passthrough) {
    CSVParseArgs* args = passthrough;

    // Ignore header
    if (idx == 0) {
        return;
    }

    setup_registry(args->registry);
    load_registry_from_csv_line(args->registry, line);
    size_t appended_bytes = write_registry(args->registry, args->dest_file);
    header_increment_next(args->header, appended_bytes);
}

/**
 * Parse csv file and build registry
 * @param args command args
 */
void c_parse_and_serialize(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->dest_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Open CSV file
    FILE* csv_file = fopen(args->source_file, "r");

    if (csv_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Write contents into the file
    FILE* dest_file = fopen(args->dest_file, "wb");
    if (dest_file == NULL) {
        puts(EX_FILE_ERROR);
        fclose(csv_file);
        return;
    }

    // Write default header with a bad status
    Header* header = build_default_header(args->registry_type);
    set_header_status(header, STATUS_BAD);
    write_header(header, dest_file);

    // Write registries
    Registry* registry = new_registry();
    registry->registry_type = args->registry_type;

    CSVParseArgs csv_parse_args = {
        header,
        registry,
        dest_file
    };

    stream_csv(csv_file, parse_csv_line, &csv_parse_args);

    destroy_registry(registry);

    fclose(csv_file);

    // Update status at beginning
    set_header_status(header, STATUS_GOOD);
    fseek(dest_file, 0, SEEK_SET);
    write_header(header, dest_file);
    destroy_header(header);

    fclose(dest_file);
    print_autocorrection_checksum(args->dest_file);
}

/**
 * Deserialize a registry and prints its contents
 * @param args command args
 */
void c_deserialize_and_print(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);

    FILE* file = fopen(args->source_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);
    bool printed = false;

    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
    } else {
        Registry* registry = build_registry(header);
        size_t max_offset = get_max_offset(header);
        while (read_bytes < max_offset) {
            read_bytes += read_registry(registry, file);

            if (registry == NULL || is_registry_removed(registry)) {
                continue;
            }

            print_registry(header, registry);
            printed = true;
        }
        destroy_registry(registry);

        if (!printed) {
            puts(EX_REGISTRY_NOT_FOUND);
        }
    }

    destroy_header(header);
    fclose(file);
}

/**
 * Deserialize a registry and print everyone matching the given filter
 * @param args command args
 */
void c_deserialize_filter_and_print(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);
    FilterArgs* filters = args->specific_data;

    FILE* file = fopen(args->source_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);
    bool printed = false;

    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
    } else {
        Registry* registry = build_registry(header);
        size_t max_offset = get_max_offset(header);
        while (read_bytes < max_offset) {
            read_bytes += read_registry(registry, file);

            if (registry == NULL || is_registry_removed(registry) || !registry_filter_match(registry, filters)) {
                continue;
            }

            print_registry(header, registry);
            printed = true;
        }
        destroy_registry(registry);

        if (!printed) {
            puts(EX_REGISTRY_NOT_FOUND);
        }
    }

    destroy_header(header);
    fclose(file);
}

/**
 * Try access a registry by its RRN and print it
 * @param args command args
 */
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->registry_type == FIX_LEN, EX_FILE_ERROR);

    SearchByRRNArgs* rrn_args = args->specific_data;

    FILE* file = fopen(args->source_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);

    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(file);
        destroy_header(header);
        return;
    }

    bool found_registry = seek_registry(header, file, rrn_args->rrn);
    if (!found_registry) {
        puts(EX_REGISTRY_NOT_FOUND);
        fclose(file);
        destroy_header(header);
        return;
    }

    Registry* registry = build_registry(header);
    read_registry(registry, file);

    if (!is_registry_removed(registry)) {
        print_registry(header, registry);
    } else {
        puts(EX_REGISTRY_NOT_FOUND);
    }

    destroy_registry(registry);
    destroy_header(header);

    fclose(file);
}


// Utils //

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

/**
 * Print a given type 1 registry into stdout
 *
 * @param header current file header
 * @param registry current registry
 */
void print_registry(Header* header, Registry* registry) {
    HeaderContent* header_content = header->header_content;
    RegistryContent* registry_content = registry->registry_content;
    // Marca
    print_column_description(header_content->desC6);
    if (registry_content->marca == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->marca);
    }

    // Modelo
    print_column_description(header_content->desC7);
    if (registry_content->modelo == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->modelo);
    }

    // Ano Fabricacao
    print_column_description(header_content->desC2);
    if (registry_content->ano == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry_content->ano);
    }

    // Cidade
    print_column_description(header_content->desC5);
    if (registry_content->cidade == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->cidade);
    }

    // Qtt
    print_column_description(header_content->desC3);
    if (registry_content->qtt == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry_content->qtt);
    }

    putchar('\n');
}

/**
 * Parse filter to int32
 * @param filter target filter
 * @return corresponding int32 value
 */
int32_t parse_int32_filter(FilterArgs* filter) {
    int32_t value = -1;

    // Filter parsing
    if (filter->parsed_value != NULL) {
        value = *((int32_t*) filter->parsed_value);
    } else {
        if (filter->value != NULL && filter->value[0] != '\0') {
            value = (int32_t) strtol(filter->value, NULL, 10);
        }
        filter->parsed_value = malloc(sizeof(value));
        *((int32_t*) filter->parsed_value) = value;
    }

    return value;
}

/**
 * Checks if a registry matches the given filter list
 * @param registry target registry
 * @param filters target filters
 * @return if registry matches the filters or not
 */
bool registry_filter_match(Registry* registry, FilterArgs* filters) {
    if (is_registry_removed(registry)) {
        return false;
    }

    if (filters == NULL) {
        return true;
    }

    RegistryContent* registry_content = registry->registry_content;
    FilterArgs* cur_filter = filters;

    while (cur_filter != NULL) {
        bool is_null = cur_filter->value == NULL || cur_filter->value[0] == '\0';
        if (strcmp(ID_FIELD_NAME, cur_filter->key) == 0) {// id
            int32_t id_filter = parse_int32_filter(cur_filter);
            if (id_filter != registry_content->id) {
                return false;
            }
        } else if (strcmp(ANO_FIELD_NAME, cur_filter->key) == 0) {// ano
            int32_t ano_filter = parse_int32_filter(cur_filter);
            if (ano_filter != registry_content->ano) {
                return false;
            }
        } else if (strcmp(QTT_FIELD_NAME, cur_filter->key) == 0) {// qtt
            int32_t qtt_filter = parse_int32_filter(cur_filter);
            if (qtt_filter != registry_content->qtt) {
                return false;
            }
        } else if (strcmp(SIGLA_FIELD_NAME, cur_filter->key) == 0) {// sigla
            // In case filter is NULL, check sigla for null value
            if (cur_filter->value == NULL) {
                if (registry_content->sigla[0] != '$') {
                    return false;
                }
            } else {
                // check if filter is smaller than or equal to the sigla
                size_t len = strlen(cur_filter->value);
                if (len > REGISTRY_SIGLA_SIZE) {
                    return false;
                }

                // compare char-by-char sigla and filter (if filter is smaller, ensure remaining bytes are NULL)
                for (size_t i = 0; i < REGISTRY_SIGLA_SIZE; i++) {
                    if (i >= len && registry_content->sigla[i] != FILLER_BYTE[0]) {
                        return false;
                    } else if (cur_filter->value[i] != registry_content->sigla[i]) {
                        return false;
                    }
                }
            }
        } else if (strcmp(CIDADE_FIELD_NAME, cur_filter->key) == 0) {// cidade
            // Check for null fields
            if (is_null || registry_content->cidade == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->cidade != NULL) || (registry_content->cidade == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->cidade) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MARCA_FIELD_NAME, cur_filter->key) == 0) {// marca
            // Check for null fields
            if (is_null || registry_content->marca == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->marca != NULL) || (registry_content->marca == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->marca) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MODELO_FIELD_NAME, cur_filter->key) == 0) {// modelo
            // Check for null fields
            if (is_null || registry_content->modelo == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->modelo != NULL) || (registry_content->modelo == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->modelo) != 0) {// Compare non-null values directly
                return false;
            }
        }

        cur_filter = cur_filter->next;
    }

    return true;
}
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

/**
 * Parse csv file and build registry
 * @param args command args
 */
void c_parse_and_serialize(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->dest_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Open CSV file
    FILE* csv_file = fopen(args->source_file, "r");
    ex_assert(csv_file != NULL, EX_FILE_ERROR);

    // Load CSV Content
    CSVContent* csv_content = read_csv(csv_file, true);
    fclose(csv_file);

    // Write contents into the file
    FILE* dest_file = fopen(args->dest_file, "wb");
    ex_assert(dest_file != NULL, EX_FILE_ERROR);

    // Write default header with a bad status
    Header* header = build_default_header(args->registry_type);
    set_header_status(header, STATUS_BAD);
    write_header(header, dest_file);

    // Write registries
    CSVLine* current_line = csv_content->head_line;
    Registry* registry = build_registry(header);
    while (current_line != NULL) {
        load_registry_from_csv_line(registry, csv_content, current_line);
        size_t appended_bytes = write_registry(registry, dest_file);
        current_line = current_line->next;
        header_increment_next(header, appended_bytes);
    }
    destroy_registry(registry);

    // Update status at beginning
    set_header_status(header, STATUS_GOOD);
    fseek(dest_file, 0, SEEK_SET);
    write_header(header, dest_file);
    destroy_header(header);

    fclose(dest_file);
    print_autocorrection_checksum(args->dest_file);
    destroy_csvcontent(csv_content);// Free CSVContent's memory
}

/**
 * Deserialize a registry and prints its contents
 * @param args command args
 */
void c_deserialize_and_print(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);

    FILE* file = fopen(args->source_file, "rb");
    ex_assert(file != NULL, EX_FILE_ERROR);

    if (args->registry_type == TYPE1) {
        T1Header* header = t1_new_header();
        size_t read_bytes = t1_read_header(header, file);
        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            fclose(file);
            ex_raise(EX_FILE_ERROR);
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
                fclose(file);
                ex_raise(EX_REGISTRY_NOT_FOUND);
            }
        }

        t1_destroy_header(header);
    } else {
        T2Header* header = t2_new_header();
        size_t read_bytes = t2_read_header(header, file);
        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            fclose(file);
            ex_raise(EX_FILE_ERROR);
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
                fclose(file);
                ex_raise(EX_REGISTRY_NOT_FOUND);
            }
        }

        t2_destroy_header(header);
    }
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
    ex_assert(file != NULL, EX_FILE_ERROR);

    if (args->registry_type == TYPE1) {
        T1Header* header = t1_new_header();
        size_t read_bytes = t1_read_header(header, file);
        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            fclose(file);
            ex_raise(EX_FILE_ERROR);
        } else {
            T1Registry* registry = t1_new_registry();
            for (uint32_t i = 0; i < header->proxRRN; i++) {
                read_bytes += t1_read_registry(registry, file);

                if (registry == NULL || !t1_registry_filter_match(registry, filters)) {
                    continue;
                }

                t1_print_registry(header, registry);
                printed = true;
            }
            t1_destroy_registry(registry);

            if (!printed) {
                fclose(file);
                ex_raise(EX_REGISTRY_NOT_FOUND);
            }
        }

        t1_destroy_header(header);
    } else {
        T2Header* header = t2_new_header();
        size_t read_bytes = t2_read_header(header, file);
        bool printed = false;

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            fclose(file);
            ex_raise(EX_FILE_ERROR);
        } else {

            T2Registry* registry = t2_new_registry();
            while (read_bytes < header->proxByteOffset) {
                read_bytes += t2_read_registry(registry, file);

                if (registry == NULL || !t2_registry_filter_match(registry, filters)) {
                    continue;
                }

                t2_print_registry(header, registry);
                printed = true;
            }
            t2_destroy_registry(registry);

            if (!printed) {
                fclose(file);
                ex_raise(EX_REGISTRY_NOT_FOUND);
            }
        }

        t2_destroy_header(header);
    }
    fclose(file);
}

/**
 * Try access a registry by its RRN and print it
 * @param args command args
 */
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args) {
    ex_assert(args->source_file != NULL, EX_COMMAND_PARSE_ERROR);
    SearchByRRNArgs* rrn_args = args->specific_data;

    FILE* file = fopen(args->source_file, "rb");
    ex_assert(file != NULL, EX_FILE_ERROR);

    if (args->registry_type == TYPE1) {
        T1Header* header = t1_new_header();
        size_t read_bytes = t1_read_header(header, file);

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            fclose(file);
            ex_raise(EX_FILE_ERROR);
        } else if (rrn_args->rrn >= header->proxRRN) {
            fclose(file);
            ex_raise(EX_REGISTRY_NOT_FOUND);
        } else {
            int64_t jump_size = T1_TOTAL_REGISTRY_SIZE * rrn_args->rrn;
            fseek(file, jump_size, SEEK_CUR);

            T1Registry* registry = t1_new_registry();
            t1_read_registry(registry, file);

            if (registry != NULL && registry->removido == NOT_REMOVED) {
                t1_print_registry(header, registry);
            } else {
                fclose(file);
                ex_raise(EX_REGISTRY_NOT_FOUND);
            }

            t1_destroy_registry(registry);
        }

        t1_destroy_header(header);
    } else {
        fclose(file);
        ex_raise(EX_FILE_ERROR);
    }
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
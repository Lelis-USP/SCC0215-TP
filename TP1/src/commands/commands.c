/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "commands.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "const/const.h"
#include "struct/common.h"
#include "struct/t1_struct.h"
#include "struct/t2_struct.h"
#include "utils/csv_parser.h"
#include "utils/provided_functions.h"
#include "utils/registry_builder.h"


// Commands //

/**
 * Parse csv file and build registry
 * @param args command args
 */
void c_parse_and_serialize(CommandArgs* args) {
    assert(args->sourceFile != NULL);
    assert(args->destFile != NULL);

    // Open CSV file
    FILE* csv_file = fopen(args->sourceFile, "r");
    if (csv_file == NULL) {
        puts(FILE_ERROR_MSG);
        return;
    }

    // Load CSV Content
    CSVContent* csv_content = read_csv(csv_file, true);
    fclose(csv_file);

    // Write contents into the file
    FILE* dest_file = fopen(args->destFile, "wb");
    if (dest_file == NULL) {
        puts(FILE_ERROR_MSG);
        return;
    }

    if (args->fileType == TYPE1) {
        // Write default header with a bad status
        T1Header header = DEFAULT_T1_HEADER;// Copy default T1Header
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
        T2Header header = DEFAULT_T2_HEADER;// Copy default T1Header
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

    print_autocorrection_checksum(args->destFile);

    destroy_csvcontent(csv_content);// Free CSVContent's memory
}

/**
 * Deserialize a registry and prints its contents
 * @param args command args
 */
void c_deserialize_and_print(CommandArgs* args) {
    assert(args->sourceFile != NULL);

    FILE* file = fopen(args->sourceFile, "rb");

    if (file == NULL) {
        puts(FILE_ERROR_MSG);
        return;
    }

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

/**
 * Deserialize a registry and print everyone matching the given filter
 * @param args command args
 */
void c_deserialize_filter_and_print(CommandArgs* args) {
    assert(args->sourceFile != NULL);
    FilterArgs* filters = args->specificData;

    FILE* file = fopen(args->sourceFile, "rb");
    if (file == NULL) {
        puts(FILE_ERROR_MSG);
        return;
    }

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

                if (registry == NULL || !t1_registry_filter_match(registry, filters)) {
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

                if (registry == NULL || !t2_registry_filter_match(registry, filters)) {
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

/**
 * Try access a registry by its RRN and print it
 * @param args command args
 */
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args) {
    assert(args->sourceFile != NULL);
    SearchByRRNArgs* rrn_args = args->specificData;

    FILE* file = fopen(args->sourceFile, "rb");

    if (file == NULL) {
        puts(FILE_ERROR_MSG);
        return;
    }

    if (args->fileType == TYPE1) {
        T1Header* header = t1_new_header();
        size_t read_bytes = t1_read_header(header, file);

        if (read_bytes == 0 || header->status == STATUS_BAD) {
            puts(FILE_ERROR_MSG);
        } else if (rrn_args->rrn >= header->proxRRN) {
            puts(EMPTY_REGISTRY_MSG);
        } else {
            int64_t jump_size = T1_TOTAL_REGISTRY_SIZE * rrn_args->rrn;
            fseek(file, jump_size, SEEK_CUR);

            T1Registry* registry = t1_new_registry();
            t1_read_registry(registry, file);

            if (registry != NULL && registry->removido == NOT_REMOVED) {
                t1_print_registry(header, registry);
            } else {
                puts(EMPTY_REGISTRY_MSG);
            }

            t1_destroy_registry(registry);
        }

        t1_destroy_header(header);
    } else {
        puts(FILE_ERROR_MSG);
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
bool t1_registry_filter_match(T1Registry* registry, FilterArgs* filters) {
    if (registry->removido == REMOVED) {
        return false;
    }

    if (filters == NULL) {
        return true;
    }

    FilterArgs* cur_filter = filters;

    while (cur_filter != NULL) {
        bool is_null = cur_filter->value == NULL || cur_filter->value[0] == '\0';
        if (strcmp(ID_FIELD_NAME, cur_filter->key) == 0) {// id
            int32_t id_filter = parse_int32_filter(cur_filter);
            if (id_filter != registry->id) {
                return false;
            }
        } else if (strcmp(ANO_FIELD_NAME, cur_filter->key) == 0) {// ano
            int32_t ano_filter = parse_int32_filter(cur_filter);
            if (ano_filter != registry->ano) {
                return false;
            }
        } else if (strcmp(QTT_FIELD_NAME, cur_filter->key) == 0) {// qtt
            int32_t qtt_filter = parse_int32_filter(cur_filter);
            if (qtt_filter != registry->qtt) {
                return false;
            }
        } else if (strcmp(SIGLA_FIELD_NAME, cur_filter->key) == 0) {// sigla
            // In case filter is NULL, check sigla for null value
            if (cur_filter->value == NULL) {
                if (registry->sigla[0] != '$') {
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
                    if (i >= len && registry->sigla[i] != FILLER_BYTE[0]) {
                        return false;
                    } else if (cur_filter->value[i] != registry->sigla[i]) {
                        return false;
                    }
                }
            }
        } else if (strcmp(CIDADE_FIELD_NAME, cur_filter->key) == 0) {// cidade
            // Check for null fields
            if (is_null || registry->cidade == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->cidade != NULL) || (registry->cidade == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->cidade) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MARCA_FIELD_NAME, cur_filter->key) == 0) {// marca
            // Check for null fields
            if (is_null || registry->marca == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->marca != NULL) || (registry->marca == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->marca) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MODELO_FIELD_NAME, cur_filter->key) == 0) {// modelo
            // Check for null fields
            if (is_null || registry->modelo == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->modelo != NULL) || (registry->modelo == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->modelo) != 0) {// Compare non-null values directly
                return false;
            }
        }

        cur_filter = cur_filter->next;
    }

    return true;
}


/**
 * Checks if a registry matches the given filter list
 * @param registry target registry
 * @param filters target filters
 * @return if registry matches the filters or not
 */
bool t2_registry_filter_match(T2Registry* registry, FilterArgs* filters) {
    if (registry->removido == REMOVED) {
        return false;
    }

    if (filters == NULL) {
        return true;
    }

    FilterArgs* cur_filter = filters;

    while (cur_filter != NULL) {
        bool is_null = cur_filter->value == NULL || cur_filter->value[0] == '\0';
        if (strcmp(ID_FIELD_NAME, cur_filter->key) == 0) {// id
            int32_t id_filter = parse_int32_filter(cur_filter);
            if (id_filter != registry->id) {
                return false;
            }
        } else if (strcmp(ANO_FIELD_NAME, cur_filter->key) == 0) {// ano
            int32_t ano_filter = parse_int32_filter(cur_filter);
            if (ano_filter != registry->ano) {
                return false;
            }
        } else if (strcmp(QTT_FIELD_NAME, cur_filter->key) == 0) {// qtt
            int32_t qtt_filter = parse_int32_filter(cur_filter);
            if (qtt_filter != registry->qtt) {
                return false;
            }
        } else if (strcmp(SIGLA_FIELD_NAME, cur_filter->key) == 0) {// sigla
            // In case filter is NULL, check sigla for null value
            if (cur_filter->value == NULL) {
                if (registry->sigla[0] != '$') {
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
                    if (i >= len && registry->sigla[i] != FILLER_BYTE[0]) {
                        return false;
                    } else if (cur_filter->value[i] != registry->sigla[i]) {
                        return false;
                    }
                }
            }
        } else if (strcmp(CIDADE_FIELD_NAME, cur_filter->key) == 0) {// cidade
            // Check for null fields
            if (is_null || registry->cidade == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->cidade != NULL) || (registry->cidade == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->cidade) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MARCA_FIELD_NAME, cur_filter->key) == 0) {// marca
            // Check for null fields
            if (is_null || registry->marca == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->marca != NULL) || (registry->marca == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->marca) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MODELO_FIELD_NAME, cur_filter->key) == 0) {// modelo
            // Check for null fields
            if (is_null || registry->modelo == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry->modelo != NULL) || (registry->modelo == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry->modelo) != 0) {// Compare non-null values directly
                return false;
            }
        }

        cur_filter = cur_filter->next;
    }

    return true;
}

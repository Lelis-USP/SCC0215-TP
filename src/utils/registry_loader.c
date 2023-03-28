/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "registry_loader.h"

#include <stdlib.h>
#include <string.h>

#include "../const/const.h"
#include "../utils/utils.h"

/**
 * Load a variable length string from a csv field
 *
 * @param field target CSV field
 * @param str_field_ptr target string field ptr
 * @param size_field_ptr target size field ptr
 * @param code_field target code field
 * @param base_code target column code
 */
void registry_load_csv_var_len_str(CSVField* field, char** str_field_ptr, strlen_t* size_field_ptr, char* code_field, const char* base_code) {
    if (field->content == NULL || field->content_len == 0) {
        (*str_field_ptr) = NULL;
        (*size_field_ptr) = 0;
    } else {
        (*size_field_ptr) = field->content_len;
        (*str_field_ptr) = calloc(field->content_len + 1, sizeof(char));
        memcpy((*str_field_ptr), field->content, field->content_len);
        memcpy(code_field, base_code, CODE_FIELD_LEN);
    }
}

/**
 * Load a int32_t from a csv field
 * @param field target CSV field
 * @param int_field_ptr target field ptr
 */
void registry_load_csv_int32(CSVField* field, int32_t* int_field_ptr) {
    if (field->content_len == 0) {
        (*int_field_ptr) = -1;
    } else {
        (*int_field_ptr) = (int32_t) strtol(field->content, NULL, 10);
    }
}

/**
 * Load the sigla from a csv field
 * @param field target CSV field
 * @param sigla sigla field
 */
void registry_load_csv_sigla(CSVField* field, char* sigla) {
    // Copy sigla
    if (field->content != NULL) {
        memcpy(sigla, field->content, max(REGISTRY_SIGLA_SIZE, field->content_len));
    }
    // Fill remaining bytes
    for (size_t j = field->content_len; j < REGISTRY_SIGLA_SIZE; j++) {
        sigla[j] = FILLER_BYTE[0];
    }
}

/**
 * Load a registry from a CSV line
 * @param csv_content target csv file
 * @param csv_line current csv line
 */
void load_registry_from_csv_line(Registry* registry, CSVLine* csv_line) {
    RegistryContent* registry_content = registry->registry_content;

    CSVField* current_field = csv_line->head_field;
    for (int i = 0; current_field != NULL; i++, current_field = current_field->next) {
        switch (i) {
            case 0:// id
                registry_load_csv_int32(current_field, &registry_content->id);
                break;
            case 1:// anoFabricacao
                registry_load_csv_int32(current_field, &registry_content->ano);
                break;
            case 2:// cidade
                registry_load_csv_var_len_str(
                        current_field,
                        &registry_content->cidade,
                        &registry_content->tamCidade,
                        registry_content->codC5,
                        DEFAULT_HEADER_CONTENT.codC5);
                break;
            case 3:// quantidade
                registry_load_csv_int32(current_field, &registry_content->qtt);

                break;
            case 4:// siglaEstado
                registry_load_csv_sigla(current_field, registry_content->sigla);
                break;
            case 5:// marca
                registry_load_csv_var_len_str(
                        current_field,
                        &registry_content->marca,
                        &registry_content->tamMarca,
                        registry_content->codC6,
                        DEFAULT_HEADER_CONTENT.codC6);
                break;
            case 6:// modelo
                registry_load_csv_var_len_str(
                        current_field,
                        &registry_content->modelo,
                        &registry_content->tamModelo,
                        registry_content->codC7,
                        DEFAULT_HEADER_CONTENT.codC7);
                break;
            default:
                break;
        }
    }
}
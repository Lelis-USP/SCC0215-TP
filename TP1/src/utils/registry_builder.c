/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "registry_builder.h"

#include <stdlib.h>
#include <string.h>

#include "const/const.h"

/**
 * Maximum of two elements macro
 * @param a first element
 * @param b second element
 * @return the biggest of a and b
 */
#define max(a, b) ((a) > (b) ? (a) : (b))

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
 * Build a T1 registry from a CSV line
 * @param csv_content target csv file
 * @param csv_line current csv line
 * @return the built registry
 */
T1Registry* t1_build_from_csv_line(CSVContent* csv_content, CSVLine* csv_line) {
    T1Registry* registry = t1_new_registry();

    CSVField* current_field = csv_line->head_field;
    for (int i = 0; current_field != NULL; i++, current_field = current_field->next) {
        switch (i) {
            case 0:// id
                registry_load_csv_int32(current_field, &registry->id);
                break;
            case 1:// anoFabricacao
                registry_load_csv_int32(current_field, &registry->ano);
                break;
            case 2:// cidade
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->cidade,
                        &registry->tamCidade,
                        registry->codC5,
                        DEFAULT_T1_HEADER.codC5);
                break;
            case 3:// quantidade
                registry_load_csv_int32(current_field, &registry->qtt);

                break;
            case 4:// siglaEstado
                registry_load_csv_sigla(current_field, registry->sigla);
                break;
            case 5:// marca
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->marca,
                        &registry->tamMarca,
                        registry->codC6,
                        DEFAULT_T1_HEADER.codC6);
                break;
            case 6:// modelo
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->modelo,
                        &registry->tamModelo,
                        registry->codC7,
                        DEFAULT_T1_HEADER.codC7);
                break;
            default:
                break;
        }
    }

    return registry;
}

/**
 * Build a T2 registry from a CSV line
 * @param csv_content target csv file
 * @param csv_line current csv line
 * @return the built registry
 */
T2Registry* t2_build_from_csv_line(CSVContent* csv_content, CSVLine* csv_line) {
    T2Registry* registry = t2_new_registry();

    CSVField* current_field = csv_line->head_field;
    for (int i = 0; current_field != NULL; i++, current_field = current_field->next) {
        switch (i) {
            case 0:// id
                registry_load_csv_int32(current_field, &registry->id);
                break;
            case 1:// anoFabricacao
                registry_load_csv_int32(current_field, &registry->ano);
                break;
            case 2:// cidade
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->cidade,
                        &registry->tamCidade,
                        registry->codC5,
                        DEFAULT_T1_HEADER.codC5);
                break;
            case 3:// quantidade
                registry_load_csv_int32(current_field, &registry->qtt);

                break;
            case 4:// siglaEstado
                registry_load_csv_sigla(current_field, registry->sigla);
                break;
            case 5:// marca
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->marca,
                        &registry->tamMarca,
                        registry->codC6,
                        DEFAULT_T1_HEADER.codC6);
                break;
            case 6:// modelo
                registry_load_csv_var_len_str(
                        current_field,
                        &registry->modelo,
                        &registry->tamModelo,
                        registry->codC7,
                        DEFAULT_T1_HEADER.codC7);
                break;
            default:
                break;
        }
    }

    return registry;
}
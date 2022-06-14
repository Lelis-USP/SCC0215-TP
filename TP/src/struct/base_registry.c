#include "base_registry.h"

#include <stdlib.h>
#include <string.h>

#include "../exception/exception.h"

size_t write_header_content(BaseHeaderContent* header_content, FILE* dest) {
    ex_assert(header_content != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    written_bytes += fwrite_member_field(header_content, descricao, dest);
    written_bytes += fwrite_member_field(header_content, desC1, dest);
    written_bytes += fwrite_member_field(header_content, desC2, dest);
    written_bytes += fwrite_member_field(header_content, desC3, dest);
    written_bytes += fwrite_member_field(header_content, desC4, dest);
    written_bytes += fwrite_member_field(header_content, codC5, dest);
    written_bytes += fwrite_member_field(header_content, desC5, dest);
    written_bytes += fwrite_member_field(header_content, codC6, dest);
    written_bytes += fwrite_member_field(header_content, desC6, dest);
    written_bytes += fwrite_member_field(header_content, codC7, dest);
    written_bytes += fwrite_member_field(header_content, desC7, dest);

    return written_bytes;
}

size_t read_header_content(BaseHeaderContent* header_content, FILE* src) {
    ex_assert(header_content != NULL, EX_GENERIC_ERROR);
    setup_header_content(header_content);

    size_t read_bytes = 0;

    read_bytes += fread_member_field(header_content, descricao, src);
    read_bytes += fread_member_field(header_content, desC1, src);
    read_bytes += fread_member_field(header_content, desC2, src);
    read_bytes += fread_member_field(header_content, desC3, src);
    read_bytes += fread_member_field(header_content, desC4, src);
    read_bytes += fread_member_field(header_content, codC5, src);
    read_bytes += fread_member_field(header_content, desC5, src);
    read_bytes += fread_member_field(header_content, codC6, src);
    read_bytes += fread_member_field(header_content, desC6, src);
    read_bytes += fread_member_field(header_content, codC7, src);
    read_bytes += fread_member_field(header_content, desC7, src);

    return read_bytes;
}

size_t write_registry_content(BaseRegistryContent* registry_content, FILE* dest) {
    ex_assert(registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    written_bytes += fwrite_member_field(registry_content, id, dest);
    written_bytes += fwrite_member_field(registry_content, ano, dest);
    written_bytes += fwrite_member_field(registry_content, qtt, dest);
    written_bytes += fwrite_member_field(registry_content, sigla, dest);

    // Write variable length fields to file
    written_bytes += fwrite_member_var_len_str(registry_content, cidade, tamCidade, codC5, dest);
    written_bytes += fwrite_member_var_len_str(registry_content, marca, tamMarca, codC6, dest);
    written_bytes += fwrite_member_var_len_str(registry_content, modelo, tamModelo, codC7, dest);

    return written_bytes;
}

size_t read_registry_content(BaseRegistryContent* registry_content, FILE* src) {
    ex_assert(registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    free(registry_content->cidade);
    free(registry_content->marca);
    free(registry_content->modelo);
    setup_registry_content(registry_content);

    size_t read_bytes = 0;

    read_bytes += fread_member_field(registry_content, id, src);
    read_bytes += fread_member_field(registry_content, ano, src);
    read_bytes += fread_member_field(registry_content, qtt, src);
    read_bytes += fread_member_field(registry_content, sigla, src);

    // Read variable length fields
    for (uint8_t i = 0; i < 3; i++) {
        VarLenStrField var_len_field = fread_var_len_str(src);
        read_bytes += var_len_field.read_bytes;

        // Null field, no more fields remaining, break
        if (var_len_field.data == NULL) {
            break;
        }

        // Fill appropriate column based on the column code
        if (strncmp(var_len_field.code, "0", CODE_FIELD_LEN) == 0) {
            registry_content->tamCidade = var_len_field.size;
            memcpy(registry_content->codC5, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry_content->cidade = var_len_field.data;
        } else if (strncmp(var_len_field.code, "1", CODE_FIELD_LEN) == 0) {
            registry_content->tamMarca = var_len_field.size;
            memcpy(registry_content->codC6, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry_content->marca = var_len_field.data;
        } else if (strncmp(var_len_field.code, "2", CODE_FIELD_LEN) == 0) {
            registry_content->tamModelo = var_len_field.size;
            memcpy(registry_content->codC7, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry_content->modelo = var_len_field.data;
        } else {
            ex_raise(EX_FILE_ERROR);
        }
    }

    return read_bytes;
}

void setup_header_content(BaseHeaderContent* header_content) {
    // Nothing to set up
}

void setup_registry_content(BaseRegistryContent* registry_content) {
    for (uint8_t i = 0; i < REGISTRY_SIGLA_SIZE; i++) {
        registry_content->sigla[i] = FILLER_BYTE[0];
    }
    registry_content->tamCidade = 0;
    registry_content->cidade = NULL;
    registry_content->tamMarca = 0;
    registry_content->marca = NULL;
    registry_content->tamModelo = 0;
    registry_content->modelo = NULL;
}

BaseHeaderContent* new_header_content() {
    BaseHeaderContent* header_content = malloc(sizeof(struct BaseHeaderContent));
    ex_assert(header_content != NULL, EX_MEMORY_ERROR);
    setup_header_content(header_content);
    return header_content;
}

BaseRegistryContent* new_registry_content() {
    BaseRegistryContent* registry_content = malloc(sizeof(struct BaseRegistryContent));
    ex_assert(registry_content != NULL, EX_MEMORY_ERROR);
    setup_registry_content(registry_content);
    return registry_content;
}

void destroy_header_content(BaseHeaderContent* header_content) {
    if (header_content == NULL) {
        return;
    }

    free(header_content);
}
void destroy_registry_content(BaseRegistryContent* registry_content) {
    if (registry_content == NULL) {
        return;
    }

    free(registry_content->cidade);
    free(registry_content->marca);
    free(registry_content->modelo);
    free(registry_content);
}

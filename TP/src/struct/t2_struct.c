/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "t2_struct.h"

#include <stdlib.h>
#include <string.h>

#include "../exception/exception.h"

/**
 * Write a header struct to a file (at the current position, no seeking)
 *
 * @param header header struct to be written
 * @param dest destination file
 * @return amount of bytes written
 */
size_t t2_write_header(T2Header* header, FILE* dest) {
    /**
     * Checklist:
     * - Shall I use a local memory buffer to write everything in a single call? (I guess there wouldn't be benefits, if anything memory management problems)
     * - Shall I check for write failures here? (Kinda addressed it by returning total written bytes)
     */

    // Basic validation
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    // Writing
    size_t written_bytes = 0;

    // Tidy af (https://bit.ly/3MRaPk3)
    // Write struct's fields in order (to see how the macro works, take a look at common.h)
    written_bytes += fwrite_member_field(header, status, dest);
    written_bytes += fwrite_member_field(header, topo, dest);
    written_bytes += fwrite_member_field(header, descricao, dest);
    written_bytes += fwrite_member_field(header, desC1, dest);
    written_bytes += fwrite_member_field(header, desC2, dest);
    written_bytes += fwrite_member_field(header, desC3, dest);
    written_bytes += fwrite_member_field(header, desC4, dest);
    written_bytes += fwrite_member_field(header, codC5, dest);
    written_bytes += fwrite_member_field(header, desC5, dest);
    written_bytes += fwrite_member_field(header, codC6, dest);
    written_bytes += fwrite_member_field(header, desC6, dest);
    written_bytes += fwrite_member_field(header, codC7, dest);
    written_bytes += fwrite_member_field(header, desC7, dest);
    written_bytes += fwrite_member_field(header, proxByteOffset, dest);
    written_bytes += fwrite_member_field(header, nroRegRem, dest);

    return written_bytes;
}

/**
 * Read a header struct from a file (at the current position, no seeking)
 *
 * @param header the header to write the data into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t2_read_header(T2Header* header, FILE* src) {
    /**
     * Checklist:
     * Shall I check for read failures for every field (to catch an early EOF for example)
     */
    if (header == NULL) {
        return 0;
    }

    t2_setup_header(header);// Place default data into header

    // Read struct's fields in order

    // Measure read bytes for the first field, if 0 there was a failure, stop from here. if 1, assume that everything is fine
    size_t read_bytes = 0;
    read_bytes += fread_member_field(header, status, src);

    if (read_bytes == 0) {
        return read_bytes;
    }

    read_bytes += fread_member_field(header, topo, src);
    read_bytes += fread_member_field(header, descricao, src);
    read_bytes += fread_member_field(header, desC1, src);
    read_bytes += fread_member_field(header, desC2, src);
    read_bytes += fread_member_field(header, desC3, src);
    read_bytes += fread_member_field(header, desC4, src);
    read_bytes += fread_member_field(header, codC5, src);
    read_bytes += fread_member_field(header, desC5, src);
    read_bytes += fread_member_field(header, codC6, src);
    read_bytes += fread_member_field(header, desC6, src);
    read_bytes += fread_member_field(header, codC7, src);
    read_bytes += fread_member_field(header, desC7, src);
    read_bytes += fread_member_field(header, proxByteOffset, src);
    read_bytes += fread_member_field(header, nroRegRem, src);

    return read_bytes;
}

/**
 * Computes the size the registry will fill when serialized
 * @param registry registry to be measured
 * @return the registry serialized size (without considering removed flag and the size field)
 */
size_t t2_registry_size(T2Registry* registry) {
    size_t size = 0;

    // Static fields (without removed and registry size)
    size += sizeof(registry->prox);
    size += sizeof(registry->id);
    size += sizeof(registry->ano);
    size += sizeof(registry->qtt);
    size += sizeof(registry->sigla);

    // Var len str fields
    if (registry->tamCidade != 0 && registry->cidade != NULL) {
        size += sizeof(registry->tamCidade);
        size += sizeof(registry->codC5);
        size += sizeof(char) * registry->tamCidade;
    }

    if (registry->tamMarca != 0 && registry->marca != NULL) {
        size += sizeof(registry->tamMarca);
        size += sizeof(registry->codC6);
        size += sizeof(char) * registry->tamMarca;
    }

    if (registry->tamModelo != 0 && registry->modelo != NULL) {
        size += sizeof(registry->tamModelo);
        size += sizeof(registry->codC7);
        size += sizeof(char) * registry->tamModelo;
    }

    return size;
}

/**
 * Writes a registry into a given file
 *
 * @param registry the registry to be written into the file
 * @param dest destination file
 * @return number of bytes written
 */
size_t t2_write_registry(T2Registry* registry, FILE* dest) {
    // Basic validation
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    // Total fields written
    size_t written_bytes = 0;

    // Update the registry size
    registry->tamanhoRegistro = t2_registry_size(registry);

    // Write fixed length fields to file
    written_bytes += fwrite_member_field(registry, removido, dest);
    written_bytes += fwrite_member_field(registry, tamanhoRegistro, dest);
    written_bytes += fwrite_member_field(registry, prox, dest);
    written_bytes += fwrite_member_field(registry, id, dest);
    written_bytes += fwrite_member_field(registry, ano, dest);
    written_bytes += fwrite_member_field(registry, qtt, dest);
    written_bytes += fwrite_member_field(registry, sigla, dest);

    // Write variable length fields to file
    written_bytes += fwrite_member_var_len_str(registry, cidade, tamCidade, codC5, dest);
    written_bytes += fwrite_member_var_len_str(registry, marca, tamMarca, codC6, dest);
    written_bytes += fwrite_member_var_len_str(registry, modelo, tamModelo, codC7, dest);

    // Sanity check that the amount of bytes written is the expected size
    ex_assert(written_bytes == registry->tamanhoRegistro + t2_ignored_size, EX_GENERIC_ERROR);

    return written_bytes;
}

/**
 * Read a registry from the current file position
 *
 * @param registry the regisrty to write the data into
 * @param src source file
 * @return the amount of bytes read (include skipped bytes)
 */
size_t t2_read_registry(T2Registry* registry, FILE* src) {
    if (registry == NULL) {
        return 0;
    }

    free(registry->cidade);
    free(registry->marca);
    free(registry->modelo);
    t2_setup_registry(registry);// Place default data into registry

    // Read struct's fields in order

    // Measure read bytes for the first field, if 0 there was a failure, stop from here. if 1, assume that everything is fine
    size_t read_bytes = fread_member_field(registry, removido, src);
    if (read_bytes == 0) {
        return 0;
    }

    // Read fixed length fields
    read_bytes += fread_member_field(registry, tamanhoRegistro, src);
    read_bytes += fread_member_field(registry, prox, src);
    read_bytes += fread_member_field(registry, id, src);
    read_bytes += fread_member_field(registry, ano, src);
    read_bytes += fread_member_field(registry, qtt, src);
    read_bytes += fread_member_field(registry, sigla, src);

    // Read variable length fields
    while (read_bytes < registry->tamanhoRegistro + t2_ignored_size) {
        VarLenStrField var_len_field = fread_var_len_str(src);
        read_bytes += var_len_field.read_bytes;

        // Null field, no more fields remaining, break
        if (var_len_field.data == NULL) {
            break;
        }

        // Fill appropriate column based on the column code
        if (strncmp(var_len_field.code, "0", CODE_FIELD_LEN) == 0) {
            registry->tamCidade = var_len_field.size;
            memcpy(registry->codC5, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry->cidade = var_len_field.data;
        } else if (strncmp(var_len_field.code, "1", CODE_FIELD_LEN) == 0) {
            registry->tamMarca = var_len_field.size;
            memcpy(registry->codC6, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry->marca = var_len_field.data;
        } else if (strncmp(var_len_field.code, "2", CODE_FIELD_LEN) == 0) {
            registry->tamModelo = var_len_field.size;
            memcpy(registry->codC7, var_len_field.code, CODE_FIELD_LEN * sizeof(char));
            registry->modelo = var_len_field.data;
        } else {
            ex_raise(EX_GENERIC_ERROR);
        }
    }

    // Ensure the amount of bytes read follows the expected size
    ex_assert(read_bytes == registry->tamanhoRegistro + t2_ignored_size, EX_GENERIC_ERROR);

    return read_bytes;
}


// Constructors & Destructors //
/**
 * Setup header with NULL/default data
 *
 * @param registry target header
 */
void t2_setup_header(T2Header* header) {
    header->status = STATUS_BAD;
    header->topo = -1;
    header->proxByteOffset = 0;
    header->nroRegRem = 0;
}

/**
 * Setup registry with NULL/default data
 *
 * @param registry target registry
 */
void t2_setup_registry(T2Registry* registry) {
    registry->removido = NOT_REMOVED;
    for (uint8_t i = 0; i < REGISTRY_SIGLA_SIZE; i++) {
        registry->sigla[i] = FILLER_BYTE[0];
    }
    registry->prox = -1;
    registry->tamCidade = 0;
    registry->cidade = NULL;
    registry->tamMarca = 0;
    registry->marca = NULL;
    registry->tamModelo = 0;
    registry->modelo = NULL;
    registry->tamanhoRegistro = 0;
}

/**
 * Allocate a new header and setup some of its fields to ensure consistent behaviour
 * @return the newly allocated header
 */
T2Header* t2_new_header() {
    T2Header* header = malloc(sizeof(struct T2Header));
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    t2_setup_header(header);
    return header;
}

/**
 * Allocate a new registry and setup some of its fields to ensure consistent behaviour
 * @return the newly allocated registry
 */
T2Registry* t2_new_registry() {
    T2Registry* registry = malloc(sizeof(struct T2Registry));
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    t2_setup_registry(registry);
    return registry;
}

/**
 * Destroy a header struct and its heap-allocated fields (if it had any)
 * @param header the header ptr to be freed
 */
void t2_destroy_header(T2Header* header) {
    if (header == NULL) {
        return;
    }

    free(header);
}

/**
 * Destroy a registry struct and its heap-allocated fields
 * @param registry the registry ptr to be freed
 */
void t2_destroy_registry(T2Registry* registry) {
    if (registry == NULL) {
        return;
    }

    free(registry->cidade);
    free(registry->marca);
    free(registry->modelo);
    free(registry);
}

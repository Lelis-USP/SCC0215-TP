
#include "t1_struct.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// TO-DO: should I do custom exception handling instead of asserts? Yes you dumbass (from me to myself)

/**
 * Write a header struct to a file (at the current position, no seeking)
 *
 * @param header header struct to be written
 * @param dest destination file
 * @return amount of bytes written
 */
size_t t1_write_header(T1Header* header, FILE* dest) {
    /**
     * Checklist:
     * - Shall I use a local memory buffer to write everything in a single call? (I guess there wouldn't be benefits, if anything memory management problems)
     * - Shall I check for write failures here? (Kinda addressed it by returning total written bytes)
     */

    // Basic validation
    assert(header != NULL);
    assert(dest != NULL);

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
    written_bytes += fwrite_member_field(header, proxRRN, dest);
    written_bytes += fwrite_member_field(header, nroRegRem, dest);

    return written_bytes;
}

/**
 * Read a header struct from a file (at the current position, no seeking)
 *
 * @param src source file
 * @return the read header (NULL if failed)
 */
T1Header* t1_read_header(FILE* src) {
    /**
     * Checklist:
     * Shall I check for read failures for every field (to catch an early EOF for example)
     */
    T1Header* header = t1_new_header();
    if (header == NULL) {
        return header;
    }

    // Read struct's fields in order

    // Measure read bytes for the first field, if 0 there was a failure, stop from here. if 1, assume that everything is fine
    size_t read_bytes = 0;
    read_bytes += fread_member_field(header, status, src);

    if (read_bytes == 0) {
        t1_destroy_header(header);
        return NULL;
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
    read_bytes += fread_member_field(header, proxRRN, src);
    read_bytes += fread_member_field(header, nroRegRem, src);

    return header;
}


/**
 * Writes a registry into a given file
 *
 * @param registry the registry to be written into the file
 * @param dest destination file
 * @return number of bytes written
 */
size_t t1_write_registry(T1Registry* registry, FILE* dest) {
    // Basic validation
    assert(registry != NULL);
    assert(dest != NULL);

    // Total fields written
    size_t written_bytes = 0;

    // Write fixed length fields to file
    written_bytes += fwrite_member_field(registry, removido, dest);
    written_bytes += fwrite_member_field(registry, prox, dest);
    written_bytes += fwrite_member_field(registry, id, dest);
    written_bytes += fwrite_member_field(registry, ano, dest);
    written_bytes += fwrite_member_field(registry, qtt, dest);
    written_bytes += fwrite_member_field(registry, sigla, dest);

    // Write variable length fields to file
    written_bytes += fwrite_member_var_len_str(registry, cidade, tamCidade, codC5, dest);
    written_bytes += fwrite_member_var_len_str(registry, marca, tamMarca, codC6, dest);
    written_bytes += fwrite_member_var_len_str(registry, modelo, tamModelo, codC7, dest);

    // Fill remaining bytes
    size_t missing_bytes = T1_TOTAL_REGISTRY_SIZE - written_bytes;
    fill_bytes(missing_bytes, dest);

    return written_bytes;
}

/**
 * Read a registry from the current file position
 *
 * @param src source file
 * @return the read registry
 */
T1Registry* t1_read_registry(FILE* src) {
    T1Registry* registry = t1_new_registry();
    if (registry == NULL) {
        return registry;
    }

    // Read struct's fields in order

    // Measure read bytes for the first field, if 0 there was a failure, stop from here. if 1, assume that everything is fine
    size_t read_bytes = fread_member_field(registry, removido, src);
    if (read_bytes == 0) {
        t1_destroy_registry(registry);
        return NULL;
    }

    // Read fixed length fields
    read_bytes += fread_member_field(registry, prox, src);
    read_bytes += fread_member_field(registry, id, src);
    read_bytes += fread_member_field(registry, ano, src);
    read_bytes += fread_member_field(registry, qtt, src);
    read_bytes += fread_member_field(registry, sigla, src);

    // Read variable length fields
    for (uint8_t i = 0; i < 3; i++) {
        VarLenStrField var_len_field = fread_var_len_str(src);
        read_bytes += var_len_field.read_bytes;

        // Null field, no more fields remaining, break
        if (var_len_field.data == NULL) {
            break;
        }

        /**
         * Was it overkill to declare the code as an array of chars and handle it like that instead of a normal single char?
         * perhaps, but I ain't going back now
         */

        // Fill appropriate column based on the column code
        if (strncmp(var_len_field.code, "0", CODE_FIELD_LEN) == 0) {
            registry->tamCidade = var_len_field.size;
            memcpy(registry->codC5, var_len_field.code, CODE_FIELD_LEN * sizeof (char));
            registry->cidade = var_len_field.data;
        } else if (strncmp(var_len_field.code, "1", CODE_FIELD_LEN) == 0) {
            registry->tamMarca = var_len_field.size;
            memcpy(registry->codC6, var_len_field.code, CODE_FIELD_LEN * sizeof (char));
            registry->marca = var_len_field.data;
        } else if (strncmp(var_len_field.code, "2", CODE_FIELD_LEN) == 0) {
            registry->tamModelo = var_len_field.size;
            memcpy(registry->codC7, var_len_field.code, CODE_FIELD_LEN * sizeof (char));
            registry->modelo = var_len_field.data;
        } else {
            assert(0 && "Invalid column code");
        }
    }

    // Skip remaining bytes to be ready at the next RRN
    size_t remaining_bytes = T1_TOTAL_REGISTRY_SIZE - read_bytes;
    fseek(src, (long) remaining_bytes, SEEK_CUR);

    return registry;
}

// Constructors & Destructors //

/**
 * Allocate a new header and setup some of its fields to ensure consistent behaviour
 * @return the newly allocated header
 */
T1Header* t1_new_header() {
        T1Header* header = malloc(sizeof (struct T1Header));
        assert(header != NULL);

        header->status = STATUS_BAD;
        header->topo = -1;
        header->proxRRN = 0;
        header->nroRegRem = 0;
        return header;
}

/**
 * Allocate a new registry and setup some of its fields to ensure consistent behaviour
 * @return the newly allocated registry
 */
T1Registry* t1_new_registry() {
    T1Registry* registry = malloc(sizeof (struct T1Registry));
    assert(registry != NULL);

    registry->removido = NOT_REMOVED;
    registry->prox = -1;
    registry->tamCidade = 0;
    registry->cidade = NULL;
    registry->tamMarca = 0;
    registry->marca = NULL;
    registry->tamModelo = 0;
    registry->modelo = NULL;
    return registry;
}

/**
 * Destroy a header struct and its heap-allocated fields (if it had any)
 * @param header the header ptr to be freed
 */
void t1_destroy_header(T1Header* header) {
    if (header == NULL) {
        return;
    }

    free(header);
}

/**
 * Destroy a registry struct and its heap-allocated fields
 * @param registry the registry ptr to be freed
 */
void t1_destroy_registry(T1Registry* registry) {
    if (registry == NULL) {
        return;
    }

    free(registry->cidade);
    free(registry->marca);
    free(registry->modelo);
    free(registry);
}

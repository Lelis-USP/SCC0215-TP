/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "t1_registry.h"

#include <stdlib.h>

#include "../exception/exception.h"

/**
 * Writes the given header (of type RT_FIX_LEN) into the target file
 * @param header header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t1_write_header(Header* header, FILE* dest) {
    // Basic validation
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_content != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(header->registry_type == RT_FIX_LEN, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    T1HeaderMetadata* metadata = header->header_metadata;
    HeaderContent* header_content = header->header_content;

    // Amount of bytes written
    size_t written_bytes = 0;

    // Write metadata before content
    written_bytes += fwrite_member_field(metadata, status, dest);
    written_bytes += fwrite_member_field(metadata, topo, dest);

    // Write header content
    written_bytes += write_header_content(header_content, dest);

    // Write metadata after content
    written_bytes += fwrite_member_field(metadata, proxRRN, dest);
    written_bytes += fwrite_member_field(metadata, nroRegRem, dest);

    return written_bytes;
}

/**
 * Reads the given header (of type RT_FIX_LEN) from the target file
 * @param header header to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t1_read_header(Header* header, FILE* src) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_content != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(header->registry_type == RT_FIX_LEN, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    T1HeaderMetadata* metadata = header->header_metadata;
    HeaderContent* content = header->header_content;

    // Assumes that header is already set-up on parent call
    size_t read_bytes = 0;

    // Read metadata before content
    read_bytes += fread_member_field(metadata, status, src);
    read_bytes += fread_member_field(metadata, topo, src);

    read_bytes += read_header_content(content, src);

    // Read metadata after content
    read_bytes += fread_member_field(metadata, proxRRN, src);
    read_bytes += fread_member_field(metadata, nroRegRem, src);

    return read_bytes;
}

/**
 * Writes the given registry (of type RT_FIX_LEN) into the target file
 * @param registry registry to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t1_write_registry(Registry* registry, FILE* dest) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_type == RT_FIX_LEN, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    T1RegistryMetadata* registry_metadata = registry->registry_metadata;
    RegistryContent* registry_content = registry->registry_content;

    // Amount of bytes written
    size_t written_bytes = 0;

    // Write registry metadata
    written_bytes += fwrite_member_field(registry_metadata, removido, dest);
    written_bytes += fwrite_member_field(registry_metadata, prox, dest);

    // Don't update other data if registry is removed
    if (registry_metadata->removido == REMOVED) {
        return written_bytes;
    }

    // Write registry content
    written_bytes += write_registry_content(registry_content, dest);

    // Fill remaining bytes
    size_t missing_bytes = T1_REGISTRY_SIZE - written_bytes;
    written_bytes += fill_bytes(missing_bytes, dest);

    return written_bytes;
}

/**
 * Reads the given registry (of type RT_FIX_LEN) from the target file
 * @param registry registry to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t1_read_registry(Registry* registry, FILE* src) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_type == RT_FIX_LEN, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    T1RegistryMetadata* registry_metadata = registry->registry_metadata;
    RegistryContent* registry_content = registry->registry_content;

    // Amount of bytes read
    size_t read_bytes = 0;

    // Read registry metadata
    read_bytes += fread_member_field(registry_metadata, removido, src);
    read_bytes += fread_member_field(registry_metadata, prox, src);

    if (registry_metadata->removido == REMOVED) {
        fseek(src, (long) (T1_REGISTRY_SIZE - read_bytes), SEEK_CUR);
        return T1_REGISTRY_SIZE;
    }

    // Read registry content
    read_bytes += read_registry_content(registry_content, src, T1_REGISTRY_SIZE - read_bytes);

    // Skip remaining bytes
    size_t remaining_bytes = T1_REGISTRY_SIZE - read_bytes;
    fseek(src, (long) remaining_bytes, SEEK_CUR);

    return read_bytes + remaining_bytes;
}

/**
 * Setups the given RT_FIX_LEN header metadata
 * @param header_metadata target header metadata
 */
void t1_setup_header_metadata(T1HeaderMetadata* header_metadata) {
    header_metadata->status = STATUS_BAD;
    header_metadata->topo = -1;
    header_metadata->proxRRN = 0;
    header_metadata->nroRegRem = 0;
}

/**
 * Setups the given RT_FIX_LEN registry metadata
 * @param registry_metadata
 */
void t1_setup_registry_metadata(T1RegistryMetadata* registry_metadata) {
    registry_metadata->removido = NOT_REMOVED;
    registry_metadata->prox = -1;
}

/**
 * Allocates and setup a new RT_FIX_LEN header metadata
 * @return the allocated header metadata
 */
T1HeaderMetadata* t1_new_header_metadata() {
    T1HeaderMetadata* header_metadata = malloc(sizeof(struct T1HeaderMetadata));
    t1_setup_header_metadata(header_metadata);
    return header_metadata;
}

/**
 * Allocates and setup a new RT_FIX_LEN registry metadata
 * @return the allocated registry metadata
 */
T1RegistryMetadata* t1_new_registry_metadata() {
    T1RegistryMetadata* registry_metadata = malloc(sizeof(struct T1RegistryMetadata));
    t1_setup_registry_metadata(registry_metadata);
    return registry_metadata;
}

/**
 * Destroys (frees) the given header metadata and its contents
 * @param header_metadata the target header metadata
 */
void t1_destroy_header_metadata(T1HeaderMetadata* header_metadata) {
    if (header_metadata == NULL) {
        return;
    }

    free(header_metadata);
}

/**
 * Destroys (frees) the given registry metadata and its contents
 * @param registry_metadata the target registry metadata
 */
void t1_destroy_registry_metadata(T1RegistryMetadata* registry_metadata) {
    if (registry_metadata == NULL) {
        return;
    }

    free(registry_metadata);
}

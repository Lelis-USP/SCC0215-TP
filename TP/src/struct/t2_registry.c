/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "t2_registry.h"

#include <stdlib.h>

#include "../exception/exception.h"
#include "../utils/utils.h"

/**
 * Writes the given header (of type VAR_LEN) into the target file
 * @param header header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t2_write_header(Header* header, FILE* dest) {
    // Basic validation
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_content != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(header->registry_type == VAR_LEN, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    T2HeaderMetadata* metadata = header->header_metadata;
    HeaderContent* header_content = header->header_content;

    // Amount of bytes written
    size_t written_bytes = 0;

    // Write metadata before content
    written_bytes += fwrite_member_field(metadata, status, dest);
    written_bytes += fwrite_member_field(metadata, topo, dest);

    // Write header content
    written_bytes += write_header_content(header_content, dest);

    // Write metadata after content
    written_bytes += fwrite_member_field(metadata, proxByteOffset, dest);
    written_bytes += fwrite_member_field(metadata, nroRegRem, dest);

    return written_bytes;
}

/**
 * Reads the given header (of type VAR_LEN) from the target file
 * @param header header to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t2_read_header(Header* header, FILE* src) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_content != NULL, EX_GENERIC_ERROR);
    ex_assert(header->header_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(header->registry_type == VAR_LEN, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    T2HeaderMetadata *metadata = header->header_metadata;
    HeaderContent *content = header->header_content;

    // Assumes that header is already set-up on parent call
    size_t read_bytes = 0;

    // Read metadata before content
    read_bytes += fread_member_field(metadata, status, src);
    read_bytes += fread_member_field(metadata, topo, src);

    read_bytes += read_header_content(content, src);

    // Read metadata after content
    read_bytes += fread_member_field(metadata, proxByteOffset, src);
    read_bytes += fread_member_field(metadata, nroRegRem, src);

    return read_bytes;
}

/**
 * Computes the required registry size for a given registry
 * @param registry the target registry
 * @return the minimum required size for the registry (except for the ignored parts)
 */
size_t t2_registry_size(Registry* registry) {
    T2RegistryMetadata* registry_metadata = registry->registry_metadata;
    RegistryContent* registry_content = registry->registry_content;

    size_t size = 0;

    // Static fields (without removed and registry size)
    size += sizeof(registry_metadata->prox);

    size += sizeof(registry_content->id);
    size += sizeof(registry_content->ano);
    size += sizeof(registry_content->qtt);
    size += sizeof(registry_content->sigla);

    // Var len str fields
    if (registry_content->tamCidade != 0 && registry_content->cidade != NULL) {
        size += sizeof(registry_content->tamCidade);
        size += sizeof(registry_content->codC5);
        size += sizeof(char) * registry_content->tamCidade;
    }

    if (registry_content->tamMarca != 0 && registry_content->marca != NULL) {
        size += sizeof(registry_content->tamMarca);
        size += sizeof(registry_content->codC6);
        size += sizeof(char) * registry_content->tamMarca;
    }

    if (registry_content->tamModelo != 0 && registry_content->modelo != NULL) {
        size += sizeof(registry_content->tamModelo);
        size += sizeof(registry_content->codC7);
        size += sizeof(char) * registry_content->tamModelo;
    }

    return size;
}

/**
 * Writes the given registry (of type VAR_LEN) into the target file
 * @param registry registry to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t2_write_registry(Registry* registry, FILE* dest) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_type == VAR_LEN, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    T2RegistryMetadata* registry_metadata = registry->registry_metadata;
    RegistryContent* registry_content = registry->registry_content;

    // Update registry size
    registry_metadata->tamanhoRegistro = max(registry_metadata->tamanhoRegistro, t2_registry_size(registry));
    size_t expected_size = registry_metadata->tamanhoRegistro + T2_IGNORED_SIZE;

    // Amount of bytes written
    size_t written_bytes = 0;

    // Write registry metadata
    written_bytes += fwrite_member_field(registry_metadata, removido, dest);
    written_bytes += fwrite_member_field(registry_metadata, tamanhoRegistro, dest);
    written_bytes += fwrite_member_field(registry_metadata, prox, dest);

    // Write registry content
    written_bytes += write_registry_content(registry_content, dest);

    // Fill remaining bytes for reused registries
    if (written_bytes < expected_size) {
        written_bytes += fill_bytes(expected_size - written_bytes, dest);
    }

    return written_bytes;
}

/**
 * Reads the given registry (of type VAR_LEN) from the target file
 * @param registry registry to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t2_read_registry(Registry* registry, FILE* src) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_content != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_metadata != NULL, EX_GENERIC_ERROR);
    ex_assert(registry->registry_type == VAR_LEN, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    T2RegistryMetadata* registry_metadata = registry->registry_metadata;
    RegistryContent* registry_content = registry->registry_content;

    // Amount of bytes read
    size_t read_bytes = 0;

    // Read registry metadata
    read_bytes += fread_member_field(registry_metadata, removido, src);
    read_bytes += fread_member_field(registry_metadata, tamanhoRegistro, src);
    read_bytes += fread_member_field(registry_metadata, prox, src);

   size_t expected_size = registry_metadata->tamanhoRegistro + T2_IGNORED_SIZE;

   if (registry_metadata->removido == REMOVED) {
       fseek(src, (long) (expected_size-read_bytes), SEEK_CUR);
       return expected_size;
   }

    // Read registry content
    read_bytes += read_registry_content(registry_content, src, (registry_metadata->tamanhoRegistro + T2_IGNORED_SIZE) - read_bytes);

    // Skip remaining bytes
    if (read_bytes < expected_size) {
        fseek(src, (long) (expected_size-read_bytes), SEEK_CUR);
    }

    // Check for over-reads
    ex_assert(read_bytes <= expected_size, EX_CORRUPTED_REGISTRY);

    return expected_size;
}

/**
 * Setups the given VAR_LEN header metadata
 * @param header_metadata target header metadata
 */
void t2_setup_header_metadata(T2HeaderMetadata* header_metadata) {
    header_metadata->status = STATUS_BAD;
    header_metadata->topo = -1;
    header_metadata->proxByteOffset = 0;
    header_metadata->nroRegRem = 0;
}

/**
 * Setups the given VAR_LEN registry metadata
 * @param registry_metadata
 */
void t2_setup_registry_metadata(T2RegistryMetadata* registry_metadata) {
    registry_metadata->removido = NOT_REMOVED;
    registry_metadata->tamanhoRegistro = 0;
    registry_metadata->prox = -1;
}

/**
 * Allocates and setup a new VAR_LEN header metadata
 * @return the allocated header metadata
 */
T2HeaderMetadata* t2_new_header_metadata() {
    T2HeaderMetadata* header_metadata = malloc(sizeof (struct T2HeaderMetadata));
    t2_setup_header_metadata(header_metadata);
    return header_metadata;
}

/**
 * Allocates and setup a new VAR_LEN registry metadata
 * @return the allocated registry metadata
 */
T2RegistryMetadata* t2_new_registry_metadata() {
    T2RegistryMetadata* registry_metadata = malloc(sizeof (struct T2RegistryMetadata));
    t2_setup_registry_metadata(registry_metadata);
    return registry_metadata;
}

/**
 * Destroys (frees) the given header metadata and its contents
 * @param header_metadata the target header metadata
 */
void t2_destroy_header_metadata(T2HeaderMetadata* header_metadata) {
    if (header_metadata == NULL) {
        return;
    }

    free(header_metadata);
}

/**
 * Destroys (frees) the given registry metadata and its contents
 * @param registry_metadata the target registry metadata
 */
void t2_destroy_registry_metadata(T2RegistryMetadata* registry_metadata) {
    if (registry_metadata == NULL) {
        return;
    }

    free(registry_metadata);
}

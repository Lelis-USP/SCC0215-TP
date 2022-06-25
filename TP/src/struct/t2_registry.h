/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "common.h"
#include "registry_content.h"
#include "registry.h"

/**
 * VAR_LEN header metadata struct
 */
typedef struct T2HeaderMetadata {
    char status;
    int64_t topo;
    int64_t proxByteOffset;
    int32_t nroRegRem;
} T2HeaderMetadata;

static const size_t T2_HEADER_SIZE = 190;

/**
 * VAR_LEN registry metadata struct
 */
typedef struct T2RegistryMetadata {
    char removido;
    uint32_t tamanhoRegistro;
    int64_t prox;
} T2RegistryMetadata;

// Ignore T2 registry size on field tamanhoRegistro
static const size_t T2_IGNORED_SIZE = member_size(T2RegistryMetadata, removido) + member_size(T2RegistryMetadata, tamanhoRegistro);

// File I/O //

/**
 * Writes the given header (of type VAR_LEN) into the target file
 * @param header header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t2_write_header(Header* header, FILE* dest);

/**
 * Reads the given header (of type VAR_LEN) from the target file
 * @param header header to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t2_read_header(Header* header, FILE* src);

/**
 * Computes the required registry size for a given registry
 * @param registry the target registry
 * @return the minimum required size for the registry (except for the ignored parts)
 */
size_t t2_registry_size(Registry* registry);

/**
 * Writes the given registry (of type VAR_LEN) into the target file
 * @param registry registry to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t2_write_registry(Registry* registry, FILE* dest);

/**
 * Reads the given registry (of type VAR_LEN) from the target file
 * @param registry registry to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t2_read_registry(Registry* registry, FILE* src);

// Setup //

/**
 * Setups the given VAR_LEN header metadata
 * @param header_metadata target header metadata
 */
void t2_setup_header_metadata(T2HeaderMetadata* header_metadata);

/**
 * Setups the given VAR_LEN registry metadata
 * @param registry_metadata
 */
void t2_setup_registry_metadata(T2RegistryMetadata* registry_metadata);

// Allocators //

/**
 * Allocates and setup a new VAR_LEN header metadata
 * @return the allocated header metadata
 */
T2HeaderMetadata* t2_new_header_metadata();

/**
 * Allocates and setup a new VAR_LEN registry metadata
 * @return the allocated registry metadata
 */
T2RegistryMetadata* t2_new_registry_metadata();

// Destroy //

/**
 * Destroys (frees) the given header metadata and its contents
 * @param header_metadata the target header metadata
 */
void t2_destroy_header_metadata(T2HeaderMetadata* header_metadata);

/**
 * Destroys (frees) the given registry metadata and its contents
 * @param registry_metadata the target registry metadata
 */
void t2_destroy_registry_metadata(T2RegistryMetadata* registry_metadata);

/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "common.h"
#include "registry_content.h"
#include "registry.h"

#define T1_HEADER_SIZE 182
#define T1_REGISTRY_SIZE 97

/**
 * FIX_LEN header metadata struct
 */
typedef struct T1HeaderMetadata {
    char status;
    int32_t topo;
    int32_t proxRRN;
    int32_t nroRegRem;
} T1HeaderMetadata;

/**
 * FIX_LEN registry metadata struct
 */
typedef struct T1RegistryMetadata {
    char removido;
    int32_t prox;
} T1RegistryMetadata;

// File I/O //

/**
 * Writes the given header (of type FIX_LEN) into the target file
 * @param header header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t1_write_header(Header* header, FILE* dest);

/**
 * Reads the given header (of type FIX_LEN) from the target file
 * @param header header to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t1_read_header(Header* header, FILE* src);

/**
 * Writes the given registry (of type FIX_LEN) into the target file
 * @param registry registry to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t t1_write_registry(Registry* registry, FILE* dest);

/**
 * Reads the given registry (of type FIX_LEN) from the target file
 * @param registry registry to be read into
 * @param src source file
 * @return the amount of bytes read
 */
size_t t1_read_registry(Registry* registry, FILE* src);

// Setup //

/**
 * Setups the given FIX_LEN header metadata
 * @param header_metadata target header metadata
 */
void t1_setup_header_metadata(T1HeaderMetadata* header_metadata);

/**
 * Setups the given FIX_LEN registry metadata
 * @param registry_metadata
 */
void t1_setup_registry_metadata(T1RegistryMetadata* registry_metadata);

// Allocation //

/**
 * Allocates and setup a new FIX_LEN header metadata
 * @return the allocated header metadata
 */
T1HeaderMetadata* t1_new_header_metadata();

/**
 * Allocates and setup a new FIX_LEN registry metadata
 * @return the allocated registry metadata
 */
T1RegistryMetadata* t1_new_registry_metadata();

// Destroy //

/**
 * Destroys (frees) the given header metadata and its contents
 * @param header_metadata the target header metadata
 */
void t1_destroy_header_metadata(T1HeaderMetadata* header_metadata);

/**
 * Destroys (frees) the given registry metadata and its contents
 * @param registry_metadata the target registry metadata
 */
void t1_destroy_registry_metadata(T1RegistryMetadata* registry_metadata);

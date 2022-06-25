/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdbool.h>

#include "common.h"
#include "registry_content.h"

/**
 * Current availables registry types, in case of undefinied type, use UNKNOWN
 */
typedef enum RegistryType {
    UNKNOWN = 0,
    FIX_LEN = 1,
    VAR_LEN = 2
} RegistryType;

/**
 * Generic header struct
 */
typedef struct Header {
    void* header_metadata;
    HeaderContent* header_content;
    RegistryType registry_type;
} Header;

/**
 * Generic registry struct
 */
typedef struct Registry {
    void* registry_metadata;
    RegistryContent* registry_content;
    RegistryType registry_type;
} Registry;

// Memory management

/**
 * Setup an already allocated header with default NULL-equivalent data
 * @param header the target header pointer
 */
void setup_header(Header* header);

/**
 * Setup an already allocated registry with default NULL-equivalent data
 * @param registry the target registry pointer
 */
void setup_registry(Registry* registry);

/**
 * Allocates a new header with an unknown type
 * @return the allocated header
 */
Header* new_header();

/**
 * Allocates a new registry with an unkwnown type
 * @return the allocated registry
 */
Registry* new_registry();

/**
 * Deallocates an existing header and all of its components
 * @param header the target header
 */
void destroy_header(Header* header);

/**
 * Deallocates an existing registry and all of its components
 * @param registry the target registry
 */
void destroy_registry(Registry* registry);


/**
 * Allocates a header and setup it (allocates its components) for a target registry type
 * @param registry_type the header registry type
 * @return the allocated and set-up header
 */
Header* build_header(RegistryType registry_type);

/**
 * Allocates a header and setup it with the default data for a target registry type
 * @param registry_type the header registry type
 * @return the allocated and set-up header
 */
Header* build_default_header(RegistryType registry_type);

/**
 * Allocates a registry and setup it (allocates its components) for a target header spec (and registry type)
 * @param header the header of which the registry is associated
 * @return the allocated and set-up registry
 */
Registry* build_registry(Header* header);

/**
 * Allocates a registry and setup it (allocates its components) for a target registry type
 * @param registry_type the registry type
 * @return the allocated and set-up registry
 */
Registry* build_registry_from_type(RegistryType registry_type);

// File I/O

/**
 * Writes a generic header into the given file (must be already on the top position)
 * @param header the header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t write_header(Header* header, FILE* dest);

/**
 * Reads the header from a given file
 * @param header the header ptr on which the data will be written
 * @param src the source file
 * @return the amount of bytes read
 */
size_t read_header(Header* header, FILE* src);

/**
 * Writes a registry into the given file (at the current position)
 * @param registry the registry to be written
 * @param dest the destination file
 * @return the amount of bytes written
 */
size_t write_registry(Registry* registry, FILE* dest);

/**
 * Reads a registry from the given file
 * @param registry the registry ptr on which the data will be read into
 * @param src the source file
 * @return the amount of bytes read
 */
size_t read_registry(Registry* registry, FILE* src);


// Utils

/**
 * Utility to check if a registry was removed
 * @param registry target registry
 * @return if the registry is removed
 */
bool is_registry_removed(Registry* registry);

/**
 * Utility to update a header status
 * @param header the target header
 * @param status the new header status
 */
void set_header_status(Header* header, char status);

/**
 * Utility to retrieve a header status
 * @param header the target header
 * @return the current header status
 */
char get_header_status(Header* header);

/**
 * Increments the header's next open position reference (RRN or byte offset, depending on file type)
 * @param header the target header
 * @param appended_bytes the number of bytes appended to the end of the file on the last write
 */
void header_increment_next(Header* header, size_t appended_bytes);

/**
 * Computes the file's end reference based off the next open position reference
 * @param header the target header
 * @return the next offset after the end (aka, loop until less than, not equal)
 */
size_t get_max_offset(Header* header);

/**
 * Seek for a given registry based on its RRN or offset. If the seek fails, the file position goes to the end.
 * @param header the file's header
 * @param file the target file
 * @param target the target position
 * @return if the seek was successful
 */
bool seek_registry(Header* header, FILE* file, size_t target);

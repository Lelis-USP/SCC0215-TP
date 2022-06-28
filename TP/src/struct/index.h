/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>

#include "registry.h"

typedef struct IndexElement {
    int32_t id;
    uint64_t reference; // Might be an RRN (32bit) or a byte offset (64bit)
} IndexElement;

typedef struct IndexHeader {
    char status;
    bool sorted;
    RegistryType registry_type;
    IndexElement* index_pool;
    uint32_t pool_size;
    uint32_t pool_used;
} IndexHeader;

// Index pool initial size
#define INITIAL_POOL_SIZE 512
#define POOL_SCALING_FACTOR 2
#define MIN_ELEMENT_SIZE 8

// Memory management //

/**
 * Allocate and setup new index header
 * @return the allocated header
 */
IndexHeader* new_index_header();

/**
 * Destroys (frees) target index header
 * @param index_header target index header
 */
void destroy_index_header(IndexHeader* index_header);

/**
 * Allocates a new index for the given registry type
 * @param registry_type target registry type
 * @return the allocated index
 */
IndexHeader* new_index(RegistryType registry_type);

// Index opertaions //

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element if found (or else, NULL)
 */
IndexElement* index_query(IndexHeader* index_header, int32_t id);

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool index_remove(IndexHeader* index_header, int32_t id);

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool index_add(IndexHeader* index_header, int32_t id, uint64_t reference);

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool index_update(IndexHeader* index_header, int32_t id, uint64_t reference);

/**
 * Sorts the index for further searches
 * @param index_header target index header
 */
void index_sort(IndexHeader* index_header);

// File I/O //

/**
 * Write entire index into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_index(IndexHeader* index_header, FILE* dest);

/**
 * Read entire index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_index(IndexHeader* index_header, FILE* src);

/**
 * Write index header into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_index_header(IndexHeader* index_header, FILE* dest);

/**
 * Read index header from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_index_header(IndexHeader* index_header, FILE* src);

/**
 * Write index element into the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* dest);

/**
 * Read index element from the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param src source file
 * @return amount of bytes read
 */
size_t read_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* src);

// Index info //

/**
 * Update index status (memory only)
 * @param index_header target index header
 * @param status new status
 */
void set_index_status(IndexHeader* index_header, char status);

/**
 * Get index status (from memory)
 * @param index_header target index header
 * @return index status
 */
char get_index_status(IndexHeader* index_header);


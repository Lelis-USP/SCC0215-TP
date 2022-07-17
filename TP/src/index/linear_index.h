/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/registry.h"
#include "index.h"

/////////////
// Configs //
/////////////

// Pool starting size
#define INITIAL_POOL_SIZE 512

// Pool growth factor (the pool grows exponentially)
#define POOL_SCALING_FACTOR 2

/////////////////////////////
// Data structures & types //
/////////////////////////////

typedef struct LinearIndexHeader {
    // Actual data
    char status;
    // Metadata
    bool sorted;
    RegistryType registry_type;
    IndexElement* index_pool;
    uint32_t pool_size;
    uint32_t pool_used;
} LinearIndexHeader;

///////////////////////
// Memory management //
///////////////////////

/**
 * Allocate and setup new index header
 * @return the allocated header
 */
LinearIndexHeader* new_linear_index_header();

/**
 * Destroys (frees) target index header
 * @param index_header target index header
 */
void destroy_linear_index_header(LinearIndexHeader* index_header);

/**
 * Allocates a new index for the given registry type
 * @param registry_type target registry type
 * @return the allocated index
 */
LinearIndexHeader* new_linear_index(RegistryType registry_type);

/**
 * Reserve a new position in the index pool (increases the pool_used and, if needed, the pool size)
 * @param index_header target index header
 * @return the index of the new allocated position (pool_used - 1)
 */
uint32_t reserve_linear_pool_pos(LinearIndexHeader* index_header);

/////////////////////////////
// Public index operations //
/////////////////////////////

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element (id will be -1 if not found)
 */
IndexElement linear_index_query(LinearIndexHeader* index_header, int32_t id);

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool linear_index_add(LinearIndexHeader* index_header, int32_t id, int64_t reference);

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool linear_index_remove(LinearIndexHeader* index_header, int32_t id);

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool linear_index_update(LinearIndexHeader* index_header, int32_t id, int64_t reference);


//////////////
// File I/O //
//////////////

/**
 * Write entire index into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_linear_index(LinearIndexHeader* index_header, FILE* dest);

/**
 * Read entire index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_linear_index(LinearIndexHeader* index_header, FILE* src);

/**
 * Write index header into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_linear_index_header(LinearIndexHeader* index_header, FILE* dest);

/**
 * Read index header from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_linear_index_header(LinearIndexHeader* index_header, FILE* src);

/**
 * Write index element into the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_linear_index_element(LinearIndexHeader* index_header, IndexElement* index_element, FILE* dest);

/**
 * Read index element from the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param src source file
 * @return amount of bytes read
 */
size_t read_linear_index_element(LinearIndexHeader* index_header, IndexElement* index_element, FILE* src);

/////////////////
// Index utils //
/////////////////

/**
 * Update index status (memory only)
 * @param index_header target index header
 * @param status new status
 */
void set_linear_index_status(LinearIndexHeader* index_header, char status);

/**
 * Get index status (from memory)
 * @param index_header target index header
 * @return index status
 */
char get_linear_index_status(LinearIndexHeader* index_header);

void write_linear_index_status(LinearIndexHeader* index_header, FILE* file);


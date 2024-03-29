/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>

#include "../struct/registry.h"

/////////////
// Configs //
/////////////

// The minimum size of an index element
#define MIN_ELEMENT_SIZE 8

/////////////////////////////
// Data structures & types //
/////////////////////////////

// Index element (key-value pair)
typedef struct IndexElement {
    int32_t id;
    int64_t reference;// Might be an RRN (32bit) or a byte offset (64bit)
} IndexElement;

// Index types
typedef enum IndexType {
    IT_UNKNOWN,
    IT_LINEAR,
    IT_B_TREE
} IndexType;

// Shared index header structure
typedef struct IndexHeader {
    IndexType index_type;
    void* header;
    FILE* file;
} IndexHeader;


///////////////////////
// Memory management //
///////////////////////

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
IndexHeader* new_index(RegistryType registry_type, IndexType index_type);

///////////////////////
// Index operations //
//////////////////////

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element if found (or else, NULL)
 */
IndexElement index_query(IndexHeader* index_header, int32_t id);

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool index_add(IndexHeader* index_header, int32_t id, int64_t reference);

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool index_remove(IndexHeader* index_header, int32_t id);

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool index_update(IndexHeader* index_header, int32_t id, int64_t reference);

//////////////
// File I/O //
//////////////

/**
 * Write entire index into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_index(IndexHeader* index_header, FILE* dest);

/**
 * Read index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_index(IndexHeader* index_header, FILE* src);

/////////////////
// Index utils //
/////////////////

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

/**
 * Write index status (only the status) to the file
 * @param index_header target index header
 * @param file target file
 */
void write_index_status(IndexHeader* index_header, FILE* file);

/**
 * Update the file associated to the index (used for internal operations)
 * @param index_header target index header
 * @param file new index file (the previou won't be closed nor be touched in any way)
 */
void set_index_file(IndexHeader* index_header, FILE* file);

/**
 * Retrieve the current file associated to the index
 * @param index_header target index header
 * @return the file associated to the index (might be NULL)
 */
FILE* get_index_file(IndexHeader* index_header);

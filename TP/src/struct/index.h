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

#define INITIAL_POOL_SIZE 4096
#define POOL_SCALING_FACTOR 2
#define MIN_ELEMENT_SIZE 8

// Memory management //
IndexHeader* new_index_header();
void destroy_index_header(IndexHeader* index_header);
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

size_t write_index(IndexHeader* index_header, FILE* dest);
size_t read_index(IndexHeader* index_header, FILE* src);

size_t write_index_header(IndexHeader* index_header, FILE* dest);
size_t read_index_header(IndexHeader* index_header, FILE* src);

size_t write_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* dest);
size_t read_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* src);

// Index info //
void set_index_status(IndexHeader* index_header, char status);
char get_index_status(IndexHeader* index_header);


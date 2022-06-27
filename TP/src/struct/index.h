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
IndexElement* index_query(IndexHeader* index_header, int32_t id);
bool index_remove(IndexHeader* index_header, int32_t id);
bool index_add(IndexHeader* index_header, int32_t id, uint64_t reference);
bool index_update(IndexHeader* index_header, int32_t id, uint64_t reference);
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


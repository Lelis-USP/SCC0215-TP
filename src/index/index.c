/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "index.h"

#include <stdlib.h>

#include "../exception/exception.h"
#include "../struct/common.h"
#include "btree_index.h"
#include "linear_index.h"

///////////////////////
// Memory management //
///////////////////////

/**
 * Allocate and setup new index header
 * @return the allocated header
 */
IndexHeader* new_index_header() {
    IndexHeader* index_header = malloc(sizeof(struct IndexHeader));

    index_header->index_type = IT_UNKNOWN;
    index_header->header = NULL;
    index_header->file = NULL;

    return index_header;
}

/**
 * Destroys (frees) target index header
 * @param index_header target index header
 */
void destroy_index_header(IndexHeader* index_header) {
    if (index_header == NULL) {
        return;
    }

    switch (index_header->index_type) {
        case IT_LINEAR:
            destroy_linear_index_header(index_header->header);
            break;
        case IT_B_TREE:
            destroy_b_tree_index_header(index_header->header);
            break;
        default:
            free(index_header->header);
    }

    // Free the pool itself
    free(index_header);
}

/**
 * Allocates a new index for the given registry type
 * @param registry_type target registry type
 * @return the allocated index
 */
IndexHeader* new_index(RegistryType registry_type, IndexType index_type) {
    IndexHeader* index_header = new_index_header();

    index_header->index_type = index_type;
    switch (index_type) {
        case IT_LINEAR:
            index_header->header = new_linear_index(registry_type);
            break;
        case IT_B_TREE:
            index_header->header = new_b_tree_index(registry_type);
            break;
        default:
            index_header->index_type = IT_UNKNOWN;
    }

    return index_header;
}

///////////////////////
// Index operations //
//////////////////////

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element (id will be -1 if not found)
 */
IndexElement index_query(IndexHeader* index_header, int32_t id) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            return linear_index_query((LinearIndexHeader*) index_header->header, id);
        case IT_B_TREE:
            return b_tree_index_query((BTreeIndexHeader*) index_header->header, index_header->file, id);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return (IndexElement){-1, -1};
}

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool index_add(IndexHeader* index_header, int32_t id, int64_t reference) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            return linear_index_add((LinearIndexHeader*) index_header->header, id, reference);
        case IT_B_TREE:
            return b_tree_index_add((BTreeIndexHeader*) index_header->header, index_header->file, id, reference);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return false;
}

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool index_remove(IndexHeader* index_header, int32_t id) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            return linear_index_remove((LinearIndexHeader*) index_header->header, id);
        case IT_B_TREE:
            return b_tree_index_remove((BTreeIndexHeader*) index_header->header, index_header->file, id);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return false;
}

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool index_update(IndexHeader* index_header, int32_t id, int64_t reference) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            return linear_index_update((LinearIndexHeader*) index_header->header, id, reference);
        case IT_B_TREE:
            return b_tree_index_update((BTreeIndexHeader*) index_header->header, index_header->file, id, reference);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return false;
}

//////////////
// File I/O //
//////////////

/**
 * Write entire index into the target file (might change file ptr)
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_index(IndexHeader* index_header, FILE* dest) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);


    switch (index_header->index_type) {
        case IT_LINEAR:
            index_header->file = dest = freopen(NULL, "wb", dest);
            return write_linear_index((LinearIndexHeader*) index_header->header, dest);
        case IT_B_TREE:
            return write_b_tree_index((BTreeIndexHeader*) index_header->header, dest);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return 0;
}

/**
 * Read entire index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_index(IndexHeader* index_header, FILE* src) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    index_header->file = src;

    switch (index_header->index_type) {
        case IT_LINEAR:
            return read_linear_index((LinearIndexHeader*) index_header->header, src);
        case IT_B_TREE:
            return read_b_tree_index((BTreeIndexHeader*) index_header->header, src);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return 0;
}

/////////////////
// Index utils //
/////////////////

/**
 * Update index status (memory only)
 * @param index_header target index header
 * @param status new status
 */
void set_index_status(IndexHeader* index_header, char status) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            set_linear_index_status((LinearIndexHeader*) index_header->header, status);
            break;
        case IT_B_TREE:
            set_b_tree_index_status((BTreeIndexHeader*) index_header->header, status);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }
}

/**
 * Get index status (from memory)
 * @param index_header target index header
 * @return index status
 */
char get_index_status(IndexHeader* index_header) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            return get_linear_index_status((LinearIndexHeader*) index_header->header);
        case IT_B_TREE:
            return get_b_tree_index_status((BTreeIndexHeader*) index_header->header);
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return STATUS_BAD;
}

/**
 * Write index status (only the status) to the file
 * @param index_header target index header
 * @param file target file
 */
void write_index_status(IndexHeader* index_header, FILE* file) {
    switch (index_header->index_type) {
        case IT_LINEAR:
            write_linear_index_status((LinearIndexHeader*) index_header->header, file);
            break;
        case IT_B_TREE:
            write_b_tree_index_status((BTreeIndexHeader*) index_header->header, file);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }
}

/**
 * Update the file associated to the index (used for internal operations)
 * @param index_header target index header
 * @param file new index file (the previou won't be closed nor be touched in any way)
 */
void set_index_file(IndexHeader* index_header, FILE* file) {
    index_header->file = file;
}

/**
 * Retrieve the current file associated to the index
 * @param index_header target index header
 * @return the file associated to the index (might be NULL)
 */
FILE* get_index_file(IndexHeader* index_header) {
    return index_header->file;
}

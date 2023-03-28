/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/registry.h"
#include "index.h"

/////////////
// Configs //
/////////////

// B-Tree default degree (indicates the number of childs a node can have)
#define DEFAULT_BTREE_DEGREE 4

// B-Tree page size used for fixed length registries
#define DEFAULT_BTREE_PAGE_SIZE_FIX_LEN 45

// B-Tree page size used for variable length registries
#define DEFAULT_BTREE_PAGE_SIZE_VAR_LEN 57

// B-Tree header's actual data size
#define BTREE_HEADER_FIXED_SIZE 13

/////////////////////////////
// Data structures & types //
/////////////////////////////

// B-Tree node types
typedef enum BTreeNodeType {
    ROOT_NODE = '0',
    MIDDLE_NODE = '1',
    LEAF_NODE = '2'
} BTreeNodeType;

// BTreeNodeType actual data size
typedef char BTreeNodeType_t;

// B-Tree node
typedef struct BTreeIndexNode {
    // Actual data
    BTreeNodeType_t tipoNo;
    uint32_t nroChaves;
    IndexElement* elements;
    int32_t* edges;

    // Internal metadata
    struct BTreeIndexNode* parent_node;
    int32_t rrn;
} BTreeIndexNode;

// B-Tree header
typedef struct BTreeIndexHeader {
    // Actual data
    char status;
    int32_t no_raiz;
    int32_t proxRRN;
    uint32_t nroNos;

    // Internal metadata
    uint64_t page_size;
    uint32_t degree;
    RegistryType registry_type;
    BTreeIndexNode* root_node_ref;
    uint32_t minimum_leaf_occupation;
    uint32_t minimum_middle_occupation;
    uint32_t maximum_occupation;
} BTreeIndexHeader;

// B-Tree internal arguments/results

typedef struct BTreeNodeInsertRequest {
    IndexElement target;
    int32_t right_edge;
} BTreeNodeInsertRequest;

typedef struct BTreeNodeInsertResponse {
    bool conflict;
} BTreeNodeInsertResponse;

typedef struct BTreeNodeSplitResponse {
    BTreeIndexNode* right_node;
    IndexElement promoted_element;
} BTreeNodeSplitResponse;

typedef struct BTreeIndexInsertResponse {
    bool conflict;
    BTreeNodeInsertRequest insert_request;
} BTreeIndexInsertResponse;

///////////////////////
// Memory management //
///////////////////////

/**
 * Allocate new B-Tree index header with default parameters for the given registry type
 * @param registry_type index's registry type
 * @return the newly allocated header
 */
BTreeIndexHeader* new_b_tree_index_header(RegistryType registry_type);

/**
 * Deallocates the target b-tree index header and its root node if in memory
 * @param b_tree_index_header target B-Tree index header
 */
void destroy_b_tree_index_header(BTreeIndexHeader* b_tree_index_header);

/**
 * Allocates a new B-Tree index node for the given B-Tree
 * @param b_tree_index_header the header of the tree this node is part of
 * @return the newly allocated node
 */
BTreeIndexNode* new_btree_index_node(BTreeIndexHeader* b_tree_index_header);

/**
 * Deallocate the target B-Tree index node alongside its' internal arrays
 * @param b_tree_index_node target index node
 */
void destroy_b_tree_index_node(BTreeIndexNode* b_tree_index_node);

/**
 * Create a new B-Tree index for the given registry_type
 * @param registry_type tree's registry type
 * @return the new B-Tree's header
 */
BTreeIndexHeader* new_b_tree_index(RegistryType registry_type);

/////////////////////////////
// Public index operations //
/////////////////////////////

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element (id will be -1 if not found)
 */
IndexElement b_tree_index_query(BTreeIndexHeader* index_header, FILE* file, int32_t id);

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool b_tree_index_add(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference);

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool b_tree_index_remove(BTreeIndexHeader* index_header, FILE* file, int32_t id);

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool b_tree_index_update(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference);

//////////////
// File I/O //
//////////////

/**
 * Write entire index into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index(BTreeIndexHeader* index_header, FILE* dest);

/**
 * Read index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index(BTreeIndexHeader* index_header, FILE* src);

/**
 * Write index header into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index_header(BTreeIndexHeader* index_header, FILE* dest);

/**
 * Read index header from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index_header(BTreeIndexHeader* index_header, FILE* src);

/**
 * Write index element into the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* dest);

/**
 * Read index element from the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* src);

/////////////////
// Index utils //
/////////////////

/**
 * Update index status (memory only)
 * @param index_header target index header
 * @param status new status
 */
void set_b_tree_index_status(BTreeIndexHeader* index_header, char status);

/**
 * Get index status (from memory)
 * @param index_header target index header
 * @return index status
 */
char get_b_tree_index_status(BTreeIndexHeader* index_header);

/**
 * Write index status (only the status) to the file
 * @param index_header target index header
 * @param file target file
 */
void write_b_tree_index_status(BTreeIndexHeader* index_header, FILE* file);

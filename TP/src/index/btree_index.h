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

BTreeIndexHeader* new_b_tree_index_header(RegistryType registry_type);
void destroy_b_tree_index_header(BTreeIndexHeader* b_tree_index_header);

BTreeIndexNode* new_btree_index_node(BTreeIndexHeader* b_tree_index_header);
void destroy_b_tree_index_node(BTreeIndexNode* b_tree_index_node);

BTreeIndexHeader* new_b_tree_index(RegistryType registry_type);

/////////////////////////////
// Public index operations //
/////////////////////////////

IndexElement b_tree_index_query(BTreeIndexHeader* index_header, FILE* file, int32_t id);
bool b_tree_index_add(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference);
bool b_tree_index_remove(BTreeIndexHeader* index_header, FILE* file, int32_t id);
bool b_tree_index_update(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference);

//////////////
// File I/O //
//////////////

size_t write_b_tree_index(BTreeIndexHeader* index_header, FILE* dest);
size_t read_b_tree_index(BTreeIndexHeader* index_header, FILE* src);

size_t write_b_tree_index_header(BTreeIndexHeader* index_header, FILE* dest);
size_t read_b_tree_index_header(BTreeIndexHeader* index_header, FILE* src);

size_t write_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* dest);
size_t read_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* src);

/////////////////
// Index utils //
/////////////////

void set_b_tree_index_status(BTreeIndexHeader* index_header, char status);
char get_b_tree_index_status(BTreeIndexHeader* index_header);
void write_b_tree_index_status(BTreeIndexHeader* index_header, FILE* file);


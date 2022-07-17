#pragma once

#include <stdint.h>

#include "../struct/registry.h"
#include "index.h"

#define DEFAULT_BTREE_DEGREE 4
#define DEFAULT_BTREE_PAGE_SIZE_FIX_LEN 44
#define DEFAULT_BTREE_PAGE_SIZE_VAR_LEN 56
#define BTREE_HEADER_FIXED_SIZE 13

typedef char BTreeNodeType_t;
enum BTreeNodeType {
    ROOT_NODE = '0',
    MIDDLE_NODE = '1',
    LEAF_NODE = '2'
} BTreeNodeType;

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

typedef struct BTreeNodeInsertRequest {
    IndexElement target;
    int32_t right_edge;
} BTreeNodeInsertRequest;

typedef struct BTreeNodeInsertResponse {
    bool conflict;
} BTreeNodeInsertResponse;

//typedef struct BTreeNodeSplitRequest {
//} BTreeNodeSplitRequest;

typedef struct BTreeNodeSplitResponse {
    BTreeIndexNode* right_node;
    IndexElement promoted_element;
} BTreeNodeSplitResponse;

typedef struct BTreeIndexInsertResponse {
    bool conflict;
    BTreeNodeInsertRequest insert_request;
} BTreeIndexInsertResponse;

BTreeIndexHeader* new_b_tree_index_header(RegistryType registry_type);
void destroy_b_tree_index_header(BTreeIndexHeader* b_tree_index_header);

BTreeIndexNode* new_btree_index_node(BTreeIndexHeader* b_tree_index_header);
void destroy_b_tree_index_node(BTreeIndexNode* b_tree_index_node);

void seek_b_tree_node(BTreeIndexHeader* index_header, FILE* target, int32_t rrn);
int32_t get_current_b_tree_rrn(BTreeIndexHeader* index_header, FILE* target);

size_t write_b_tree_index_header(BTreeIndexHeader* index_header, FILE* dest);
size_t read_b_tree_index_header(BTreeIndexHeader* index_header, FILE* src);

size_t write_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* dest);
size_t read_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* src);

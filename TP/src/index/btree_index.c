/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "btree_index.h"

#include <stdlib.h>
#include <string.h>

#include "../exception/exception.h"

///////////////////////
// Memory management //
///////////////////////

BTreeIndexHeader* new_b_tree_index_header(RegistryType registry_type) {
    BTreeIndexHeader* header = malloc(sizeof(struct BTreeIndexHeader));

    header->status = STATUS_GOOD;
    header->no_raiz = -1;
    header->proxRRN = 0;
    header->nroNos = 0;

    header->registry_type = registry_type;

    switch (registry_type) {
        case RT_FIX_LEN:
            header->page_size = DEFAULT_BTREE_PAGE_SIZE_FIX_LEN;
            header->degree = DEFAULT_BTREE_DEGREE;
            break;
        case RT_VAR_LEN:
            header->page_size = DEFAULT_BTREE_PAGE_SIZE_VAR_LEN;
            header->degree = DEFAULT_BTREE_DEGREE;
            break;
        default:
            header->page_size = 0;
            header->degree = 0;
    }

    // Pre-compute occupation rules
    header->maximum_occupation = header->degree - 1;
    header->minimum_middle_occupation = header->degree / 2;
    header->minimum_leaf_occupation = header->minimum_middle_occupation - 1;

    header->root_node_ref = NULL;

    return header;
}

void destroy_b_tree_index_header(BTreeIndexHeader* b_tree_index_header) {
    if (b_tree_index_header == NULL) {
        return;
    }

    destroy_b_tree_index_node(b_tree_index_header->root_node_ref);
    free(b_tree_index_header);
}

BTreeIndexNode* new_btree_index_node(BTreeIndexHeader* b_tree_index_header) {
    /**
     * Why am I allocating a node with an extra space on each array:
     * the implementation becomes a lot simpler in exchange for a very very small memory footprint increase.
     * With the extra space, I can reuse all insertion operations and make the split condition a simple check
     * of exceeded size, followed by selecting the promoted element by a direct access to the middle and
     * creating the new right node on the sequence. I started the implementation without the extra space and
     * things just were a lot more messier to read and understand in exchange for ~16 less bytes allocated in
     * memory per layer (which in an average b-tree would result in less than 100 bytes total).
     */
    BTreeIndexNode* node = malloc(sizeof(struct BTreeIndexNode));

    node->tipoNo = LEAF_NODE;
    node->nroChaves = 0;
    node->elements = malloc(b_tree_index_header->degree * sizeof(struct IndexElement));
    node->edges = malloc((b_tree_index_header->degree + 1) * sizeof(int32_t));

    memset(node->elements, -1, (b_tree_index_header->degree) * sizeof(struct IndexElement));
    memset(node->edges, -1, (b_tree_index_header->degree + 1) * sizeof(int32_t));

    node->parent_node = NULL;
    node->rrn = -1;

    return node;
}

void destroy_b_tree_index_node(BTreeIndexNode* b_tree_index_node) {
    if (b_tree_index_node == NULL) {
        return;
    }

    free(b_tree_index_node->elements);
    free(b_tree_index_node->edges);
    free(b_tree_index_node);
}

BTreeIndexHeader* new_b_tree_index(RegistryType registry_type) {
    return new_b_tree_index_header(registry_type);
}

//////////////////////////////
// Private index operations //
//////////////////////////////

/**
 * Go to the given RRN on the B-Tree
 * @param index_header tree header
 * @param target target file
 * @param rrn destination RRN (-1 goes to the start of the file, aka the header)
 */
void seek_b_tree_node(BTreeIndexHeader* index_header, FILE* target, int32_t rrn) {
    ex_assert(rrn >= -1, EX_GENERIC_ERROR);
    fseek(target, (long) ((rrn + 1) * index_header->page_size), SEEK_SET);
}

/**
 * Retrieve the current RRN on the file
 * @param index_header tree header
 * @param target target file
 * @return the current RRN
 */
int32_t get_current_b_tree_rrn(BTreeIndexHeader* index_header, FILE* target) {
    return ((int32_t) (ftell(target) / index_header->page_size)) - 1;
}

/**
 * Search for the given ID in the tree node
 * @param node the node to be searched
 * @param id search target
 * @return the index the id would be in the array
 */
uint32_t b_tree_node_search(BTreeIndexNode* node, int32_t id) {
    uint32_t low = 0;
    uint32_t high = node->nroChaves;

    while (low < high) {
        uint32_t mid = (high + low) / 2;
        if (node->elements[mid].id < id) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return low;
}

/**
 * Check if a given node is a leaf (actual leafs and root-leaf)
 * @param index_header tree header
 * @param index_node target node
 * @return wheter the node is leaf or not
 */
bool b_tree_node_is_leaf(BTreeIndexHeader* index_header, BTreeIndexNode* index_node) {
    if (index_node->tipoNo == LEAF_NODE) {
        return true;
    }

    if (index_node->tipoNo == ROOT_NODE) {
        for (uint32_t i = 0; i < index_header->degree; i++) {
            if (index_node->edges[i] != -1) {
                return false;
            }
        }

        return true;
    }

    return false;
}

/**
 * Insert an element into the given node
 * @param index_header tree header
 * @param index_node target node
 * @param node_insert_request element insertion request
 * @return the insertion response
 */
BTreeNodeInsertResponse b_tree_node_insert_element(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, BTreeNodeInsertRequest node_insert_request) {
    uint32_t idx = b_tree_node_search(index_node, node_insert_request.target.id);

    // Check for existing id
    if (index_node->elements[idx].id == node_insert_request.target.id) {
        return (BTreeNodeInsertResponse){true};
    }


    // Shift all elements and edges after insertion to the right
    index_node->edges[index_node->nroChaves + 1] = index_node->edges[index_node->nroChaves];
    for (int64_t i = (int64_t) index_node->nroChaves - 1; i >= idx; i--) {
        index_node->elements[i + 1] = index_node->elements[i];
        index_node->edges[i + 1] = index_node->edges[i];
    }

    // Insert the new element
    index_node->elements[idx] = node_insert_request.target;
    index_node->edges[idx + 1] = node_insert_request.right_edge;
    index_node->nroChaves++;

    return (BTreeNodeInsertResponse){false};
}

/**
 * Split the given node
 * @param index_header tree header
 * @param index_node target node to split
 * @return the newly created right-node and promotion info
 */
BTreeNodeSplitResponse b_tree_node_split(BTreeIndexHeader* index_header, BTreeIndexNode* index_node) {
    BTreeNodeSplitResponse response;
    uint32_t promotion_idx = index_node->nroChaves / 2;

    // Store promotion target
    response.promoted_element = index_node->elements[promotion_idx];


    // Allocate right node
    BTreeIndexNode* right_node = new_btree_index_node(index_header);
    right_node->tipoNo = index_node->tipoNo;
    response.right_node = right_node;

    // Move left node data to right node
    uint32_t offset = promotion_idx + 1;
    right_node->edges[0] = index_node->edges[offset];
    index_node->edges[offset] = -1;// Remove copied edge
    for (uint32_t i = offset; i < index_node->nroChaves; i++) {
        // Add element to right node
        right_node->elements[i - offset] = index_node->elements[i];
        right_node->nroChaves++;
        right_node->edges[i - offset + 1] = index_node->edges[i + 1];

        // Remove element from left node
        index_node->elements[i] = (IndexElement){-1, -1};
        index_node->edges[i + 1] = -1;
    }
    index_node->nroChaves -= right_node->nroChaves + 1;
    index_node->elements[promotion_idx] = (IndexElement){-1, -1};

    // Reserve right node position
    right_node->rrn = index_header->proxRRN;
    index_header->proxRRN++;
    index_header->nroNos++;

    return response;
}

/**
 * Handle insertion of any given element into the tree through its various layers
 * @param index_header tree header
 * @param parent current insertion parent node
 * @param current_node_rrn current insertion node
 * @param file target file
 * @param index_element the element to be inserted
 * @return the results of the insertion (conflicts and if there is a insertion to be made on the parent node)
 */
BTreeIndexInsertResponse b_tree_index_insert(BTreeIndexHeader* index_header, BTreeIndexNode* parent, int32_t current_node_rrn, FILE* file, IndexElement index_element) {
    ex_assert(current_node_rrn <= index_header->proxRRN, EX_GENERIC_ERROR);

    // If no RRN is provided, search for root node
    if (current_node_rrn == -1) {

        // If the tree is empty, allocate a new root node
        if (index_header->no_raiz == -1) {
            int32_t new_root_rrn = index_header->proxRRN;

            // Create new root node
            seek_b_tree_node(index_header, file, new_root_rrn);
            index_header->root_node_ref = new_btree_index_node(index_header);
            index_header->root_node_ref->tipoNo = ROOT_NODE;
            index_header->root_node_ref->rrn = new_root_rrn;
            index_header->no_raiz = new_root_rrn;

            // TO-DO: Check if write is neccessary (as of my research, it's not)
            // write_b_tree_index_node(index_header, index_header->root_node_ref, file);

            // Increase next RRN ref
            index_header->proxRRN++;
            index_header->nroNos++;
        }

        // Pre-load root node if not present
        if (index_header->root_node_ref == NULL) {
            seek_b_tree_node(index_header, file, index_header->no_raiz);
            index_header->root_node_ref = new_btree_index_node(index_header);
            read_b_tree_index_node(index_header, index_header->root_node_ref, file);
        }

        // Update current node's RRN
        current_node_rrn = index_header->no_raiz;
    }

    // State tracking
    BTreeIndexInsertResponse response = {false, {{-1, -1}, -1}};
    BTreeIndexNode* current_node = NULL;

    // Load target node
    if (current_node_rrn == index_header->no_raiz) {
        // Root node
        current_node = index_header->root_node_ref;
    } else if (current_node_rrn == index_header->proxRRN) {
        index_header->proxRRN++;
        index_header->nroNos++;
        current_node = new_btree_index_node(index_header);
        current_node->tipoNo = LEAF_NODE;
        current_node->rrn = current_node_rrn;
    } else {
        // Other nodes
        seek_b_tree_node(index_header, file, current_node_rrn);
        current_node = new_btree_index_node(index_header);
        read_b_tree_index_node(index_header, current_node, file);
        current_node->parent_node = parent;
    }

    // Store any insertion request to be executed on current node
    BTreeNodeInsertRequest insert_request;

    // If leaf, create element insertion
    if (b_tree_node_is_leaf(index_header, current_node)) {
        insert_request = (BTreeNodeInsertRequest){index_element, -1};
    } else {
        // If not on leaf, recursevily call for child node and load insertions, if required
        uint32_t target_idx = b_tree_node_search(current_node, index_element.id);
        int32_t target_rrn = current_node->edges[target_idx];

        // If there is no current edge, create a new one
        if (target_rrn == -1) {
            target_rrn = index_header->proxRRN;
        }

        BTreeIndexInsertResponse insert_response = b_tree_index_insert(index_header, current_node, target_rrn, file, index_element);

        if (insert_response.conflict) {
            response.conflict = true;
        }

        insert_request = insert_response.insert_request;
    }

    // Execute insertion, if needed
    if (insert_request.target.id != -1) {
        // Handle leaf node (leaf-root included)

        // Try inserting element
        BTreeNodeInsertResponse insert_response = b_tree_node_insert_element(index_header, current_node, insert_request);
        if (insert_response.conflict) {
            // Current node cleanup
            if (current_node != index_header->root_node_ref) {
                destroy_b_tree_index_node(current_node);
            }
            return (BTreeIndexInsertResponse){true};
        }

        // Check for overflow
        if (current_node->nroChaves > index_header->maximum_occupation) {
            // Handle root splits pt.1
            if (current_node->tipoNo == ROOT_NODE) {
                // Allocate new root
                BTreeIndexNode* new_root = new_btree_index_node(index_header);
                new_root->tipoNo = ROOT_NODE;

                // Update root left edge
                new_root->edges[0] = current_node->rrn;

                // Change root referencess
                index_header->root_node_ref = new_root;
                if (b_tree_node_is_leaf(index_header, current_node)) {
                    current_node->tipoNo = LEAF_NODE;
                } else {
                    current_node->tipoNo = MIDDLE_NODE;
                }
            }

            // Create split
            BTreeNodeSplitResponse split_response = b_tree_node_split(index_header, current_node);

            // Update response to include the insert request
            response.insert_request = (BTreeNodeInsertRequest){split_response.promoted_element, split_response.right_node->rrn};

            // Write left-node
            seek_b_tree_node(index_header, file, current_node->rrn);
            write_b_tree_index_node(index_header, current_node, file);

            // Write new right-node
            seek_b_tree_node(index_header, file, split_response.right_node->rrn);
            write_b_tree_index_node(index_header, split_response.right_node, file);

            // Handle root splits pt.2
            if (index_header->root_node_ref->rrn == -1) {
                // Reserve new root RRN
                index_header->root_node_ref->rrn = index_header->no_raiz = index_header->proxRRN;
                index_header->proxRRN++;
                index_header->nroNos++;

                // Insert element on root
                index_header->root_node_ref->elements[0] = split_response.promoted_element;
                index_header->root_node_ref->edges[1] = split_response.right_node->rrn;
                index_header->root_node_ref->nroChaves++;

                // Write new root
                seek_b_tree_node(index_header, file, index_header->root_node_ref->rrn);
                write_b_tree_index_node(index_header, index_header->root_node_ref, file);

                // Void insert request
                response.insert_request = (BTreeNodeInsertRequest){{-1, -1}, -1};
            }

            // Cleanup right node
            destroy_b_tree_index_node(split_response.right_node);
        } else {
            seek_b_tree_node(index_header, file, current_node->rrn);
            write_b_tree_index_node(index_header, current_node, file);
        }
    }

    // Current node cleanup
    if (current_node != index_header->root_node_ref) {
        destroy_b_tree_index_node(current_node);
    }

    return response;
}

IndexElement b_tree_index_find(BTreeIndexHeader* index_header, int32_t current_node_rrn, FILE* file, int32_t id) {
    ex_assert(current_node_rrn < index_header->proxRRN, EX_GENERIC_ERROR);
    // If no RRN is provided, search for root node
    if (current_node_rrn == -1) {
        // If the tree is empty, return not found
        if (index_header->no_raiz == -1) {
            return (IndexElement){-1, -1};
        }

        // Pre-load root node if not present
        if (index_header->root_node_ref == NULL) {
            seek_b_tree_node(index_header, file, index_header->no_raiz);
            index_header->root_node_ref = new_btree_index_node(index_header);
            read_b_tree_index_node(index_header, index_header->root_node_ref, file);
        }

        // Update current node's RRN
        current_node_rrn = index_header->no_raiz;
    }

    // Not found response
    IndexElement response = {-1, -1};

    // Load target node
    BTreeIndexNode* current_node = NULL;
    if (current_node_rrn == index_header->no_raiz) {
        // Root node
        current_node = index_header->root_node_ref;
    } else {
        // Other nodes
        seek_b_tree_node(index_header, file, current_node_rrn);
        current_node = new_btree_index_node(index_header);
        read_b_tree_index_node(index_header, current_node, file);
    }

    // Store any insertion request to be executed on current node
    if (b_tree_node_is_leaf(index_header, current_node)) {
        // Search element index
        uint32_t target_idx = b_tree_node_search(current_node, id);

        // Element found
        if (target_idx < current_node->nroChaves && current_node->elements[target_idx].id == id) {
            response = current_node->elements[target_idx];
        }
    } else {
        // If not on leaf, recursevily call for child node and load insertions, if required
        uint32_t target_idx = b_tree_node_search(current_node, id);

        if (target_idx < current_node->nroChaves && current_node->elements[target_idx].id == id) {
            response = current_node->elements[target_idx];
        } else {
            int32_t target_rrn = current_node->edges[target_idx];

            // Check if there is an edge, if there is the target must be there
            if (target_rrn != -1) {
                response = b_tree_index_find(index_header, target_rrn, file, id);
            }
        }
    }

    // Current node cleanup
    if (current_node != index_header->root_node_ref) {
        destroy_b_tree_index_node(current_node);
    }

    return response;
}

/////////////////////////////
// Public index operations //
/////////////////////////////

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element (id will be -1 if not found)
 */
IndexElement b_tree_index_query(BTreeIndexHeader* index_header, FILE* file, int32_t id) {
    return b_tree_index_find(index_header, -1, file, id);
}

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool b_tree_index_add(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference) {
    BTreeIndexInsertResponse response = b_tree_index_insert(index_header, NULL, -1, file, (IndexElement){id, reference});
    return !response.conflict;
}

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool b_tree_index_remove(BTreeIndexHeader* index_header, FILE* file, int32_t id) {
    return false;
}

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool b_tree_index_update(BTreeIndexHeader* index_header, FILE* file, int32_t id, int64_t reference) {
    return false;
}

//////////////
// File I/O //
//////////////

/**
 * Write entire index into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index(BTreeIndexHeader* index_header, FILE* dest) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    // Write header
    seek_b_tree_node(index_header, dest, -1);
    written_bytes += write_b_tree_index_header(index_header, dest);

    // Write root node if present
    if (index_header->root_node_ref != NULL) {
        seek_b_tree_node(index_header, dest, index_header->root_node_ref->rrn);
        written_bytes += write_b_tree_index_node(index_header, index_header->root_node_ref, dest);
    }

    return written_bytes;
}

/**
 * Read index from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index(BTreeIndexHeader* index_header, FILE* src) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(src != NULL, EX_FILE_ERROR);

    size_t read_bytes = 0;

    // Read header
    seek_b_tree_node(index_header, src, -1);
    read_bytes += read_b_tree_index_header(index_header, src);

    return read_bytes;
}

/**
 * Write index header into the target file
 * @param index_header target index header
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index_header(BTreeIndexHeader* index_header, FILE* dest) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;
    written_bytes += fwrite_member_field(index_header, status, dest);
    written_bytes += fwrite_member_field(index_header, no_raiz, dest);
    written_bytes += fwrite_member_field(index_header, proxRRN, dest);
    written_bytes += fwrite_member_field(index_header, nroNos, dest);

    ex_assert(written_bytes == BTREE_HEADER_FIXED_SIZE, EX_FILE_ERROR);

    // Fill the remainder of the page with filler bytes
    written_bytes += fill_bytes(index_header->page_size - written_bytes, dest);
    return written_bytes;
}

/**
 * Read index header from the target file
 * @param index_header target index header
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index_header(BTreeIndexHeader* index_header, FILE* src) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(src != NULL, EX_FILE_ERROR);

    // Ensure that the root node ref will be invalidated, if present
    destroy_b_tree_index_node(index_header->root_node_ref);
    index_header->root_node_ref = NULL;

    // Read B-Tree header fields
    size_t read_bytes = 0;

    read_bytes += fread_member_field(index_header, status, src);
    read_bytes += fread_member_field(index_header, no_raiz, src);
    read_bytes += fread_member_field(index_header, proxRRN, src);
    read_bytes += fread_member_field(index_header, nroNos, src);

    // Go to the next disk page
    seek_b_tree_node(index_header, src, 0);

    return read_bytes;
}

/**
 * Write index element into the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* dest) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(index_node != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    ex_assert(index_node->nroChaves <= index_header->degree, EX_CORRUPTED_REGISTRY);

    size_t written_bytes = 0;

    // Write node metadata
    written_bytes += fwrite_member_field(index_node, tipoNo, dest);
    written_bytes += fwrite_member_field(index_node, nroChaves, dest);

    // Write elements
    for (uint32_t i = 0; i < index_header->degree - 1; i++) {
        // Fill empty elements with NULL indicator
        if (i >= index_node->nroChaves) {
            index_node->elements[i].id = -1;
            index_node->elements[i].reference = -1;
        }

        // Write ID
        written_bytes += fwrite_member_field(&index_node->elements[i], id, dest);

        // Write reference
        if (index_header->registry_type == RT_FIX_LEN) {
            int32_t reference = (int32_t) index_node->elements[i].reference;
            written_bytes += fwrite(&reference, 1, sizeof(reference), dest);
        } else {
            written_bytes += fwrite_member_field(&index_node->elements[i], reference, dest);
        }
    }

    // Write edge references
    for (uint32_t i = 0; i < index_header->degree; i++) {
        written_bytes += fwrite(&index_node->edges[i], 1, sizeof(index_node->edges[i]), dest);
    }

    // Shouldn't actually be used, since page_size - written_bytes will always be 0, but just for completioness
    if (index_header->page_size > written_bytes) {
        written_bytes += fill_bytes(index_header->page_size - written_bytes, dest);
    }

    return written_bytes;
}

/**
 * Read index element from the target file
 * @param index_header target index header
 * @param index_element target index element
 * @param src source file
 * @return amount of bytes read
 */
size_t read_b_tree_index_node(BTreeIndexHeader* index_header, BTreeIndexNode* index_node, FILE* src) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(index_node != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(src != NULL, EX_FILE_ERROR);

    int32_t rrn = get_current_b_tree_rrn(index_header, src);
    index_node->rrn = rrn;

    size_t read_bytes = 0;

    // Read node metadata
    read_bytes += fread_member_field(index_node, tipoNo, src);
    read_bytes += fread_member_field(index_node, nroChaves, src);

    // Read elements
    for (uint32_t i = 0; i < index_header->degree - 1; i++) {
        // Write ID
        read_bytes += fread_member_field(&index_node->elements[i], id, src);

        // Read reference
        if (index_header->registry_type == RT_FIX_LEN) {
            int32_t reference;
            read_bytes += fread(&reference, 1, sizeof(reference), src);
            index_node->elements[i].reference = reference;
        } else {
            read_bytes += fread_member_field(&index_node->elements[i], reference, src);
        }

        // Fill empty elements with NULL indicator
        if (i >= index_node->nroChaves) {
            index_node->elements[i].id = -1;
            index_node->elements[i].reference = -1;
        }
    }

    // Read edge references
    for (uint32_t i = 0; i < index_header->degree; i++) {
        read_bytes += fread(&index_node->edges[i], 1, sizeof(index_node->edges[i]), src);
    }

    return read_bytes;
}

/////////////////
// Index utils //
/////////////////

/**
 * Update index status (memory only)
 * @param index_header target index header
 * @param status new status
 */
void set_b_tree_index_status(BTreeIndexHeader* index_header, char status) {
    index_header->status = status;
}

/**
 * Get index status (from memory)
 * @param index_header target index header
 * @return index status
 */
char get_b_tree_index_status(BTreeIndexHeader* index_header) {
    return index_header->status;
}

/**
 * Write index status (only the status) to the file
 * @param index_header target index header
 * @param file target file
 */
void write_b_tree_index_status(BTreeIndexHeader* index_header, FILE* file) {
    ex_assert(index_header != NULL, EX_CORRUPTED_REGISTRY);
    ex_assert(file != NULL, EX_FILE_ERROR);

    seek_b_tree_node(index_header, file, -1);
    fwrite_member_field(index_header, status, file);
}

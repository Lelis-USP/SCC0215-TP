/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "index.h"

#include <stdlib.h>

#include "common.h"
#include "../exception/exception.h"

// Memory management //
IndexHeader* new_index_header() {
    IndexHeader* index_header = malloc(sizeof (struct IndexHeader));

    index_header->status = STATUS_GOOD;
    index_header->sorted = true;
    index_header->registry_type = UNKNOWN;
    index_header->index_pool = NULL;
    index_header->pool_size = 0;
    index_header->pool_used = 0;

    return index_header;
}

void destroy_index_header(IndexHeader* index_header) {
    if (index_header == NULL) {
        return;
    }

    // Free the pool itself
    free(index_header->index_pool);
    free(index_header);
}

IndexHeader* new_index(RegistryType registry_type) {
    IndexHeader* index_header = new_index_header();
    index_header->registry_type = registry_type;
    return index_header;
}

uint32_t reserve_pool_position(IndexHeader* index_header){
    // There is still space on the pool
    if (index_header->pool_used < index_header->pool_size) {
        index_header->pool_used++;
        return index_header->pool_used - 1;
    }

    // First pool reservation
    if (index_header->index_pool == NULL) {
        index_header->index_pool = calloc(INITIAL_POOL_SIZE, sizeof (struct IndexElement));
        index_header->pool_size = INITIAL_POOL_SIZE;
        index_header->pool_used = 1;
        return 0;
    }

    // Extend pool size (using exponential approach)
    IndexElement* new_pool = realloc(index_header->index_pool, index_header->pool_size * POOL_SCALING_FACTOR);

    if (new_pool == NULL) {
        ex_raise(EX_MEMORY_ERROR);
        return UINT32_MAX;
    }

    index_header->index_pool = new_pool;
    index_header->pool_size *= POOL_SCALING_FACTOR;
    index_header->pool_used += 1;
    return index_header->pool_used - 1;
}

// Index opertaions //

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the reference if found (return UINT32_MAX if not found)
 */
uint32_t index_find(IndexHeader* index_header, int32_t id) {
    if (index_header->pool_used == 0) {
        return UINT32_MAX;
    }

    // Guarantee index is sorted
    if (!index_header->sorted) {
        index_sort(index_header);
    }

    // Binary search the id
    int64_t low, high;
    low = 0;
    high = index_header->pool_used - 1;

    while (low <= high) {
        int64_t mid = (high + low) / 2;

        if (index_header->index_pool[mid].id == id) {
            low = high = mid;
            break;
        }

        if (index_header->index_pool[mid].id < id) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (low < index_header->pool_used && index_header->index_pool[low].id == id) {
        return low;
    }

    return UINT32_MAX;
}

/**
 * Search for a given ID in the index
 * @param index_header target index header
 * @param id target id
 * @return the index element if found (or else, NULL)
 */
IndexElement* index_query(IndexHeader* index_header, int32_t id) {
    uint32_t idx = index_find(index_header, id);

    // Not found
    if (idx == UINT32_MAX) {
        return NULL;
    }

    return &index_header->index_pool[idx];
}

/**
 * Removes the given id from index
 * @param index_header target index header
 * @param id target id
 * @return if the id was found and removed
 */
bool index_remove(IndexHeader* index_header, int32_t id) {
    uint32_t idx = index_find(index_header, id);

    // Not found
    if (idx == UINT32_MAX) {
        return false;
    }

    uint32_t last = index_header->pool_used - 1;
    index_header->pool_used--;

    if (idx != last) {
        index_header->index_pool[idx] = index_header->index_pool[last];
    }

    index_header->sorted = false;

    return true;
}

/**
 * Insert a new id into the index
 * @param index_header target index header
 * @param id target id
 * @param reference id's reference (RRN or byte offset)
 * @return if the id was inserted in the index (false indicates its already present)
 */
bool index_add(IndexHeader* index_header, int32_t id, uint64_t reference) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(index_header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    IndexElement* match = index_query(index_header, id);

    if (match != NULL) {
        return false;
    }

    index_header->sorted = false;

    uint32_t insertion_pos = reserve_pool_position(index_header);
    index_header->index_pool[insertion_pos].id = id;
    index_header->index_pool[insertion_pos].reference = reference;

    return true;
}

/**
 * Update and existing id's reference on the index
 * @param index_header target index header
 * @param id target id
 * @param reference new id's reference
 * @return if the index was updated (false indicates id was not found)
 */
bool index_update(IndexHeader* index_header, int32_t id, uint64_t reference) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(index_header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    IndexElement* match = index_query(index_header, id);

    if (match == NULL) {
        return false;
    }

    match->reference = reference;

    return true;
}

/**
 * Quick sort recursive implementation to sort the index
 *
 * Uses random pivoting
 * @param arr index element arr being sorted
 * @param low current low-end (inclusive)
 * @param high current high-end (inclusive)
 */
void index_qsort(IndexElement arr[], uint32_t low, uint32_t high) {
    if (low >= high) {
        return;
    }

    // Partitioning
    uint32_t pivot_index = low + rand() % (high - low);
    int32_t pivot = arr[pivot_index].id;

    // Move pivot to the end
    IndexElement tmp = arr[high];
    arr[high] = arr[pivot_index];
    arr[pivot_index] = tmp;

    // Swap elements smaller than the pivot with the known first element greater than the pivot
    uint32_t partition_idx = low;
    for (uint32_t i = low; i < high; i++) {
        if (arr[i].id <= pivot) {
            tmp = arr[i];
            arr[i] = arr[partition_idx];
            arr[partition_idx] = tmp;
            partition_idx++;
        }
    }

    // Swap pivot with the first element greater than it
    tmp = arr[partition_idx];
    arr[partition_idx] = arr[high];
    arr[high] = tmp;

    // Prevent unsigned underflow
    if (partition_idx != 0) {
        index_qsort(arr, low, partition_idx - 1);
    }

    index_qsort(arr, partition_idx + 1, high);
}

/**
 * Sorts the index for further searches
 * @param index_header target index header
 */
void index_sort(IndexHeader* index_header) {
    if (index_header->sorted) {
        return;
    }

    // If used size is 0, no need to sort (it woul also cause an awful overflow)
    if (index_header->pool_used != 0) {
        index_qsort(index_header->index_pool, 0, index_header->pool_used - 1);
    }

    index_header->sorted = true;
}

// File I/O //

size_t write_index(IndexHeader* index_header, FILE* dest) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    if (!index_header->sorted) {
        index_sort(index_header);
    }

    size_t written_bytes = 0;

    write_index_header(index_header, dest);

    // Write element by element
    for (uint32_t i = 0; i < index_header->pool_used; i++) {
        write_index_element(index_header, &index_header->index_pool[i], dest);
    }

    return written_bytes;
}

size_t read_index(IndexHeader* index_header, FILE* src) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    size_t read_bytes = 0;

    read_bytes += read_index_header(index_header, src);

    // Don't read elements in case of bad status
    if (index_header->status == STATUS_BAD) {
        return read_bytes;
    }

    // Load index elements
    while (!feof(src)) {
        uint32_t insertion_pos = reserve_pool_position(index_header);
        size_t last_read_bytes = read_index_element(index_header, &index_header->index_pool[insertion_pos], src);
        read_bytes += last_read_bytes;

        // Failed to completely read last element
        if (last_read_bytes < MIN_ELEMENT_SIZE) {
            index_header->pool_used--;
            break;
        }
    }

    index_header->sorted = true;

    return read_bytes;
}

size_t write_index_header(IndexHeader* index_header, FILE* dest) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    written_bytes += fwrite_member_field(index_header, status, dest);

    return written_bytes;
}

size_t read_index_header(IndexHeader* index_header, FILE* src) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    size_t read_bytes = 0;

    read_bytes += fread_member_field(index_header, status, src);

    return read_bytes;
}

size_t write_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* dest) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(index_element != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    written_bytes += fwrite_member_field(index_element, id, dest);

    if (index_header->registry_type == FIX_LEN) {
        uint32_t four_bit_value = index_element->reference;
        written_bytes += fwrite(&four_bit_value, 1, sizeof(uint32_t), dest);
    } else {
        written_bytes += fwrite_member_field(index_element, reference, dest);
    }

    return written_bytes;
}

size_t read_index_element(IndexHeader* index_header, IndexElement* index_element, FILE* src) {
    ex_assert(index_header != NULL, EX_GENERIC_ERROR);
    ex_assert(index_element != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    size_t read_bytes = 0;

    read_bytes += fread_member_field(index_element, id, src);

    if (index_header->registry_type == FIX_LEN) {
        uint32_t four_bit_value;
        read_bytes += fread(&four_bit_value, 1, sizeof(uint32_t), src);
        index_element->reference = four_bit_value;
    } else {
        read_bytes += fread_member_field(index_element, reference, src);
    }

    return read_bytes;
}

// Index info //
void set_index_status(IndexHeader* index_header, char status) {
    index_header->status = status;
}
char get_index_status(IndexHeader* index_header) {
    return index_header->status;
}


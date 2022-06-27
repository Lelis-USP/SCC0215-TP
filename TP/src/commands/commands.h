/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdbool.h>

#include "common.h"
#include "../struct/registry.h"
#include "../struct/registry_content.h"
#include "../struct/t1_registry.h"
#include "../struct/t2_registry.h"

// Commands //
/**
 * Parse csv file and build registry
 * @param args command args
 */
void c_parse_and_serialize(CommandArgs* args);

/**
 * Deserialize a registry and prints its contents
 * @param args command args
 */
void c_deserialize_and_print(CommandArgs* args);

/**
 * Deserialize a registry and print everyone matching the given filter
 * @param args command args
 */
void c_deserialize_filter_and_print(CommandArgs* args);

/**
 * Try access a registry by its RRN and print it
 * @param args command args
 */
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args);

/**
 * Build an index for the given registry
 * @param args command args
 */
void c_build_index_from_registry(CommandArgs* args);

/**
 * Logically remove a registry and update the given index
 * @param args command args
 */
void c_remove_registry(CommandArgs* args);

/**
 * Insert registries into an existing file and udpate index
 * @param args command args
 */
void c_insert_registry(CommandArgs* args);

/**
 * Update the given registgries and their associated indexes
 * @param args command args
 */
void c_update_registry(CommandArgs* args);

// Utilities //
/**
 * Print a fixed length string
 * @param desc target string
 * @param n string length
 */
void print_fixed_len_str(char* desc, size_t n);

/**
 * Print a given type 1 registry into stdout
 *
 * @param header current file header
 * @param registry current registry
 */
void print_registry(Header* header, Registry* registry);

/**
 * Parse filter to int32
 * @param filter target filter
 * @return corresponding int32 value
 */
int32_t parse_int32_filter(FilterArgs* filter);

/**
 * Checks if a registry matches the given filter list
 * @param registry target registry
 * @param filters target filters
 * @return if registry matches the filters or not
 */
bool registry_filter_match(Registry* registry, FilterArgs* filters);

// Macros //
// Macro for printing a column description
#define print_column_description(desc) print_fixed_len_str(desc, sizeof(desc) / sizeof(char))

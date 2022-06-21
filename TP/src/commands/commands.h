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
void c_parse_and_serialize(CommandArgs* args);
void c_deserialize_and_print(CommandArgs* args);
void c_deserialize_filter_and_print(CommandArgs* args);
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args);

// Utilities //
void print_fixed_len_str(char* desc, size_t n);
void print_registry(Header* header, Registry* registry);
int32_t parse_int32_filter(FilterArgs* filter);
bool registry_filter_match(Registry* registry, FilterArgs* filters);

// Macros //
// Macro for printing a column description
#define print_column_description(desc) print_fixed_len_str(desc, sizeof(desc) / sizeof(char))

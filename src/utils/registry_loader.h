/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/registry.h"
#include "../struct/registry_content.h"
#include "../struct/t1_registry.h"
#include "../struct/t2_registry.h"
#include "../utils/csv_parser.h"

/**
 * Load a registry from a CSV line
 * @param csv_content target csv file
 * @param csv_line current csv line
 */
void load_registry_from_csv_line(Registry* registry, CSVLine* csv_line);
/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/registry.h"
#include "../struct/registry_content.h"
#include "../struct/t1_registry.h"
#include "../struct/t2_registry.h"
#include "../utils/csv_parser.h"

void load_registry_from_csv_line(Registry* registry, CSVContent* csv_content, CSVLine* csv_line);
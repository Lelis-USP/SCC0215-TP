/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/t1_struct.h"
#include "../struct/t2_struct.h"
#include "../utils/csv_parser.h"

T1Registry* t1_build_from_csv_line(CSVContent* csv_content, CSVLine* csv_line);
T2Registry* t2_build_from_csv_line(CSVContent* csv_content, CSVLine* csv_line);
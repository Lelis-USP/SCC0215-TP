/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct CSVHeader {
    bool present;
    uint32_t n_cols;
    char** titles;
} CSVHeader;

typedef struct CSVField {
    char* content;
    size_t content_len;
    struct CSVField* next;
} CSVField;

typedef struct CSVLine {
    uint32_t n_fields;
    struct CSVField* head_field;
    struct CSVLine* next;
} CSVLine;

typedef struct CSVContent {
    CSVHeader* header;
    uint32_t n_rows;
    CSVLine* head_line;
} CSVContent;

CSVHeader* new_csvheader();
CSVField* new_csvfield();
CSVLine* new_csvline();
CSVContent* new_csvcontent();

void destroy_csvcontent(CSVContent* content);
void destroy_csvheader(CSVHeader* header);
void destroy_csvline(CSVLine* line);
void destroy_csvfield(CSVField* field);

CSVContent* read_csv(FILE* csv_file, bool has_header);

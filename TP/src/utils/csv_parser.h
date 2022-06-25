/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * CSV header struct
 */
typedef struct CSVHeader {
    bool present;
    uint32_t n_cols;
    char** titles;
} CSVHeader;

/**
 * CSV field struct
 */
typedef struct CSVField {
    char* content;
    size_t content_len;
    struct CSVField* next;
} CSVField;

/**
 * CSV line struct
 */
typedef struct CSVLine {
    uint32_t n_fields;
    struct CSVField* head_field;
    struct CSVLine* next;
} CSVLine;

/**
 * CSV content (full file) struct
 */
typedef struct CSVContent {
    CSVHeader* header;
    uint32_t n_rows;
    CSVLine* head_line;
} CSVContent;

// Memory manipulation //

/**
 * Allocate and preset a new CSVHeader
 * @return the newly allocated header
 */
CSVHeader* new_csvheader();

/**
 * Allocate and preset a new CSVField
 * @return the newly allocated field
 */
CSVField* new_csvfield();

/**
 * Allocate and preset a new CSVLine
 * @return the newly allocated line
 */
CSVLine* new_csvline();

/**
 * Allocate and preset a new CSVContent
 * @return the newly allocated content
 */
CSVContent* new_csvcontent();

/**
 * Free a CSVContent entity and all of its fields
 * @param content target ptr
 */
void destroy_csvcontent(CSVContent* content);

/**
 * Free a CSVHeader entity and all of its fields
 * @param header target ptr
 */
void destroy_csvheader(CSVHeader* header);

/**
 * Free a CSVLine entity and all of its fields
 * @param line target ptr
 */
void destroy_csvline(CSVLine* line);

/**
 * Free a CSVField entity and its contents
 * @param field target ptr
 */
void destroy_csvfield(CSVField* field);

// CSV reading //

/**
 * Parse a CSV from a file
 * @param csv_file target CSV file
 * @param has_header indicates if the file has a header or not
 * @return the parsed CSV file
 */
CSVContent* read_csv(FILE* csv_file, bool has_header);

/**
 * Stream a csv line by line
 * @param csv_file targe CSV file
 * @param it the iterating function called with each line
 * @param passthrough passthrough params to the iterating function
 */
void stream_csv(FILE* csv_file, void (*it) (int idx, CSVLine* cur_line, void* passthrough), void* passthrough);

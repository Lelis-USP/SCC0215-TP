/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "utils/csv_parser.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Allocate and preset a new CSVHeader
 * @return
 */
CSVHeader* new_csvheader() {
    CSVHeader* header = malloc(sizeof(struct CSVHeader));
    header->present = false;
    header->n_cols = 0;
    header->titles = NULL;
    return header;
}

/**
 * Allocate and preset a new CSVField
 * @return
 */
CSVField* new_csvfield() {
    CSVField* field = malloc(sizeof(struct CSVField));
    field->content_len = 0;
    field->content = NULL;
    field->next = NULL;
    return field;
}

/**
 * Allocate and preset a new CSVLine entity
 * @return
 */
CSVLine* new_csvline() {
    CSVLine* line = malloc(sizeof (struct CSVLine));
    line->n_fields = 0;
    line->head_field = NULL;
    line->next = NULL;
    return line;
}

/**
 * Allocate and preset a new CSVContent entity
 * @return
 */
CSVContent* new_csvcontent() {
    CSVContent* content = malloc(sizeof (struct CSVContent));
    content->header = new_csvheader();
    content->n_rows = 0;
    content->head_line = NULL;
    return content;
}

/**
 * Free a CSVContent entity and all of its fields
 * @param content
 */
void destroy_csvcontent(CSVContent* content) {
    if (content == NULL) return;

    if (content->header != NULL) {
        destroy_csvheader(content->header);
    }

    if (content->head_line != NULL) {
        CSVLine* cur = content->head_line;

        while (cur != NULL) {
            CSVLine* next = cur->next;
            destroy_csvline(cur);
            cur = next;
        }
    }

    free(content);
}

/**
 * Free a CSVHeader entity and all of its fields
 * @param header
 */
void destroy_csvheader(CSVHeader* header) {
    if (header == NULL) return;

    if (header->titles != NULL) {
        for (int i = 0; i < header->n_cols; i++) {
            free(header->titles[i]);
        }

        free(header->titles);
    }

    free(header);
}

/**
 * Free a CSVLine entity and all of its fields
 * @param line
 */
void destroy_csvline(CSVLine* line) {
    if (line == NULL) return;

    // Empty list
    CSVField* cur = line->head_field;

    while (cur != NULL) {
        CSVField* next = cur->next;
        destroy_csvfield(cur);
        cur = next;
    }

    // Free struct
    free(line);
}

/**
 * Free a CSVField entity and its contents
 * @param field
 */
void destroy_csvfield(CSVField* field) {
    free(field->content);
    free(field);
}

/**
 * Parse a CSV from a file
 * @param csv_file target CSV file
 * @param has_header indicates if the file has a header or not
 * @return the parsed CSV file
 */
CSVContent* read_csv(FILE* csv_file, bool has_header) {
    CSVContent* content = new_csvcontent();

    // Static buffer, since teoretically, a registry couldn't even surpass 100 bytes
    char buffer[512];

    uint16_t buffer_idx = 0;

    CSVLine* cur_line = new_csvline();
    CSVLine* tail_line = NULL;
    CSVField* cur_field = NULL;

    // Loop the entire file until EOF
    while (1) {
        int32_t cur_char = getc(csv_file);

        if (cur_char == '\r') {
            cur_char = getc(csv_file);
        }

        // Handle separators
        if (cur_char == ',' || cur_char == '\n' || cur_char == EOF) {

            // Handle field creation and insertion
            if (buffer_idx != 0 || cur_char == ',') {
                // Create the new field
                CSVField* new_field = new_csvfield();

                // Copy buffer into new field
                if (buffer_idx != 0) {
                    new_field->content = calloc(buffer_idx + 1, sizeof (char));
                    memcpy(new_field->content, buffer, buffer_idx);
                    new_field->content[buffer_idx] = '\0';
                    new_field->content_len = buffer_idx;
                    buffer_idx = 0;
                }

                // Insert new field in the line's field list
                if (cur_field == NULL) {
                    cur_field = new_field;
                    cur_line->head_field = cur_field;
                    cur_line->n_fields++;
                } else {
                    cur_field->next = new_field;
                    cur_field = new_field;
                    cur_line->n_fields++;
                }
            }

            // Handle line break/EOF
            if (cur_char == '\n' || cur_char == EOF) {
                if (cur_line->n_fields != 0) {
                    // Insert the line in the content's line list
                    if (content->head_line == NULL) {
                        content->head_line = cur_line;
                    }

                    // Tail ref
                    tail_line = cur_line;
                    // Iterate line count
                    content->n_rows++;

                    if (cur_char != EOF) {
                        // Create next line
                        cur_line->next = new_csvline();
                        cur_line = cur_line->next;
                        cur_field = NULL;
                    }
                } else if (cur_char == EOF) { // If last line is empty, remove it
                    tail_line->next = NULL;
                    destroy_csvline(cur_line);
                }
            }

            if (cur_char == EOF) {
                break;
            }

            continue ;
        }

        // Prevent buffer overflow
        assert(buffer_idx < 512);

        // Write char to buffer
        buffer[buffer_idx] = (char) cur_char;
        buffer_idx++;
    }


    // Header loading
    if (content->n_rows != 0 && has_header) {
        // Retrieve top line as header
        CSVLine* header_line = content->head_line;

        // Allocate char* titles array
        content->header->titles = malloc(sizeof(void *) * header_line->n_fields);

        // Load titles into CSVHeader
        cur_field = header_line->head_field;
        uint32_t idx = 0;
        while (cur_field != NULL) {
            content->header->titles[idx] = cur_field->content;
            cur_field->content = NULL; // Prevent freeing the string

            // Iterate
            cur_field = cur_field->next;
            idx++;
        }

        content->header->present = true;
        content->header->n_cols = idx;


        // Remove header as line
        content->head_line = content->head_line->next;
        content->n_rows--;
        destroy_csvline(header_line);
    }

    return content;
}

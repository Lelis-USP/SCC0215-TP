/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "common.h"

#include <assert.h>
#include <stdlib.h>

/**
 * Write n filler bytes into a file
 * @param n number of bytes to write
 * @param file target file
 * @return number of bytes written
 */
size_t fill_bytes(size_t n, FILE* file) {
    size_t written_bytes = 0;
    for (int i = 0; i < n/sizeof(char); i++) {
        written_bytes += fwrite(FILLER_BYTE, sizeof (char), 1, file);
    }
    return written_bytes;
}

/**
 * Write a variable length string into a file
 * @param str the string to be written
 * @param len the string length
 * @param code the column code
 * @param file the target file
 * @return number of bytes written
 */
size_t fwrite_var_len_str(char* str, strlen_t len, char* code, FILE* file) {
    assert(file != NULL);

    size_t written_bytes = 0;

    if (len == 0 || str == NULL) {
        return written_bytes;
    }

    written_bytes += fwrite(&len, 1, sizeof(strlen_t), file);
    if (code == NULL) {
        for (int i = 0; i < CODE_FIELD_LEN; i++) {
            written_bytes += fwrite(FILLER_BYTE, 1, sizeof (char), file);
        }
    } else {
        written_bytes += fwrite(code, 1, sizeof(char) * CODE_FIELD_LEN, file);
    }
    written_bytes += fwrite(str, 1, sizeof (char) * len, file);

    return written_bytes;
}

/**
 * Reads a variable length string from a file
 * @param file the source file
 * @return the variable length spring data
 */
VarLenStrField fread_var_len_str(FILE* file) {
    assert(file != NULL);

    size_t read_bytes = 0;
    VarLenStrField str_field = {0, {0}, NULL, 0};

    // Read string length
    read_bytes += fread(&str_field.size, 1, sizeof(strlen_t), file);

    if (str_field.size <= 0) {
        return (VarLenStrField){-1, {0}, NULL, read_bytes};
    }

    // Read field code
    read_bytes += fread(str_field.code, 1, sizeof (char) * CODE_FIELD_LEN, file);

    if (str_field.code[0] == '$') {
        return (VarLenStrField){-1, {0}, NULL, read_bytes};
    }

    // Read the string itself
    str_field.data = calloc(str_field.size + 1, sizeof(char));
    read_bytes += fread(str_field.data, 1, sizeof (char) * str_field.size, file);


    str_field.read_bytes = read_bytes;
    return str_field;
}
#include "common.h"

#include <assert.h>
#include <stdlib.h>

size_t fill_bytes(size_t n, FILE* file) {
    size_t written_bytes = 0;
    for (int i = 0; i < n/sizeof(char); i++) {
        written_bytes += fwrite(FILLER_BYTE, sizeof (char), 1, file);
    }
    return written_bytes;
}

size_t write_var_len_str(char* str, strlen_t len, char* code, FILE* file) {
    assert(file != NULL);

    size_t written_bytes = 0;

    if (len == 0 || str == NULL) {
        size_t missing_bytes = sizeof(strlen_t) + sizeof(char) * CODE_FIELD_LEN;
        return fill_bytes(missing_bytes, file);
    }

    written_bytes += fwrite(&len, sizeof(strlen_t), 1, file);
    if (code == NULL) {
        for (int i = 0; i < CODE_FIELD_LEN; i++) {
            written_bytes += fwrite(FILLER_BYTE, sizeof (char), 1, file);
        }
    } else {
        written_bytes += fwrite(code, sizeof(char), CODE_FIELD_LEN, file);
    }
    written_bytes += fwrite(str, sizeof (char), (size_t) len, file);

    return written_bytes;
}

VarLenStrField read_var_len_str(FILE* file) {
    assert(file != NULL);

    size_t read_bytes = 0;
    VarLenStrField str_field = {0, {0}, NULL};

    // Read string length
    read_bytes += fread(&str_field.size, sizeof(strlen_t), 1, file);

    if (str_field.size <= 0) {
        return (VarLenStrField){-1, {0}, NULL};
    }

    // Read field code
    read_bytes += fread(str_field.code, sizeof (char), CODE_FIELD_LEN, file);

    // Read the string itself
    str_field.data = calloc(str_field.size + 1, sizeof(char));
    read_bytes += fread(str_field.data, sizeof (char), str_field.size, file);

    return str_field;
}
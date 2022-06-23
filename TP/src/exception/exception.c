/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "exception.h"

#include <stdio.h>
#include <stdlib.h>

void ex_raise(const char* message) {
    fprintf(EX_FILE_OUT, "%s\n", message);
    exit(EX_EXIT_CODE);
}

void ex_assert(int cond, const char* message) {
    if (!cond) {
        ex_raise(message);
    }
}

#include <assert.h>
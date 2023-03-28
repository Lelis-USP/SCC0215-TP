/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

/**
 * Maximum of two elements macro
 * @param a first element
 * @param b second element
 * @return the biggest of a and b
 */
#define max(a, b) ((a) > (b) ? (a) : (b))

/**
 * Minimum of two elements macro
 * @param a first element
 * @param b second element
 * @return the smallest of a and b
 */
#define min(a, b) ((a) < (b) ? (a) : (b))


// Buffer size used for strings
#define COMMANDS_BUFFER_SIZE 512
#define COMMANDS_BUFFER_FORMAT "%511s"

/**
 * Read a string into the given buffer (must follow commands buffer size)
 * @param source source file
 * @param buffer destination buffer
 */
static inline void read_buffer_string(FILE* source, char* buffer) {
    char format[32];
    sprintf(format, "%%%ds", COMMANDS_BUFFER_SIZE - 1);
    fscanf(source, format, buffer);
}
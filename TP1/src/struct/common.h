/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdio.h>
#include <stdint.h>

// Types //
#define strlen_t uint32_t


// Consts //
// Length of column's short-code indicator
#define CODE_FIELD_LEN 1

// Type used to represent variable string lengths
// Size of the sigla fields
#define REGISTRY_SIGLA_SIZE 2

// Header integrity status indicators
#define STATUS_GOOD '1'
#define STATUS_BAD '0'

// Field's logically removed status
#define REMOVED '1'
#define NOT_REMOVED '0'

// Reference to filler byte
static const char FILLER_BYTE[1] = {'$'};


// Structs //
// Struct to represent the results of reading a variable length string
typedef struct VarLenStrField {
    strlen_t size;
    char code[CODE_FIELD_LEN];
    char* data;
    size_t read_bytes;
} VarLenStrField;


// Functions //
size_t fwrite_var_len_str(char* str, strlen_t len, char* code, FILE* file);
VarLenStrField fread_var_len_str(FILE* file);
size_t fill_bytes(size_t n, FILE* file);


// Macros //

// Seamlessly retrieve the size of a struct member in compile-time
#define member_size(type, member) sizeof(((type *)0)->member)

// File manipulation macros
/**
 * Macro for writing a structs field into a file
 *
 * @param struct_ptr target struct's pointer
 * @param member struct's member to be written
 * @param file target file to write the data into
 */
#define fwrite_member_field(struct_ptr, member, file) fwrite(&((struct_ptr)->member), 1, sizeof((struct_ptr)->member), file)

/**
 * Macro for reading a structs field into a file
 *
 * @param struct_ptr target struct's pointer
 * @param member struct's member to be read
 * @param file target file to read the data from
 */
#define fread_member_field(struct_ptr, member, file) fread(&((struct_ptr)->member), 1, sizeof((struct_ptr)->member), file)

/**
 * Macro for writing variable length string struct fields into a file in a cleaner way to read
 *
 * @param struct_ptr target struct's pointer
 * @param str_pointer_field struct's string pointer field name
 * @param len_field struct's string length field name
 * @param code_field struct's string column code field name
 * @param file target file to write the data to
 */
#define fwrite_member_var_len_str(struct_ptr, str_pointer_field, len_field, code_field, file) fwrite_var_len_str((struct_ptr)->str_pointer_field, (struct_ptr)->len_field, (struct_ptr)->code_field, file)
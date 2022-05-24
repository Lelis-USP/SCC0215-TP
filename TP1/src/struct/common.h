#pragma once

#include <stdio.h>
#include <stdint.h>

// Seamlessly retrieve the size of a struct member in compile-time
// https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
#ifndef member_size
#define member_size(type, member) sizeof(((type *)0)->member)
#endif

#define CODE_FIELD_LEN 1
#define strlen_t uint32_t
#define REGISTRY_SIGLA_SIZE 2

#define STATUS_GOOD '1'
#define STATUS_BAD '0'


static const char FILLER_BYTE[1] = {'$'};

typedef struct VarLenStrField {
    strlen_t size;
    char code[CODE_FIELD_LEN];
    char* data;
} VarLenStrField;

size_t fill_bytes(size_t n, FILE* file);
size_t write_var_len_str(char* str, strlen_t len, char* code, FILE* file);
VarLenStrField read_var_len_str(FILE* file);
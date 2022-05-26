#pragma once

#include <stdint.h>
#include <stdio.h>

#include "common.h"

// T2Header field sizes
#define T2_HEADER_DESCRIPTION_SIZE 40
#define T2_HEADER_DES_C1_SIZE 22
#define T2_HEADER_DES_C2_SIZE 19
#define T2_HEADER_DES_C3_SIZE 24
#define T2_HEADER_DES_C4_SIZE 8
#define T2_HEADER_COD_C5_SIZE 1
#define T2_HEADER_DES_C5_SIZE 16
#define T2_HEADER_COD_C6_SIZE 1
#define T2_HEADER_DES_C6_SIZE 18
#define T2_HEADER_COD_C7_SIZE 1
#define T2_HEADER_DES_C7_SIZE 19
#define T2_HEADER_SIZE 190


typedef struct T2Header {
    char status;
    int64_t topo;
    char descricao[T2_HEADER_DESCRIPTION_SIZE];
    char desC1[T2_HEADER_DES_C1_SIZE];
    char desC2[T2_HEADER_DES_C2_SIZE];
    char desC3[T2_HEADER_DES_C3_SIZE];
    char desC4[T2_HEADER_DES_C4_SIZE];
    char codC5[T2_HEADER_COD_C5_SIZE];
    char desC5[T2_HEADER_DES_C5_SIZE];
    char codC6[T2_HEADER_COD_C6_SIZE];
    char desC6[T2_HEADER_DES_C6_SIZE];
    char codC7[T2_HEADER_COD_C7_SIZE];
    char desC7[T2_HEADER_DES_C7_SIZE];
    int64_t proxByteOffset;
    uint32_t nroRegRem;
} T2Header;

typedef struct T2Registry {
    char removido;
    uint64_t tamanhoRegistro;
    int64_t prox;
    int32_t id;
    int32_t ano;
    int32_t qtt;
    char sigla[REGISTRY_SIGLA_SIZE];
    strlen_t tamCidade;
    char codC5[1];
    char* cidade;
    strlen_t tamMarca;
    char codC6[1];
    char* marca;
    strlen_t tamModelo;
    char codC7[1];
    char* modelo;
} T2Registry;

size_t t2_write_header(T2Header* header, FILE* dest);
T2Header* t2_read_header(FILE* src);

size_t t2_registry_size(T2Registry* registry);
size_t t2_write_registry(T2Registry* registry, FILE* dest);
T2Registry* t2_read_registry(FILE* src);

T2Header* t2_new_header();
T2Registry* t2_new_registry();

void t2_destroy_header(T2Header* header);
void t2_destroy_registry(T2Registry* registry);

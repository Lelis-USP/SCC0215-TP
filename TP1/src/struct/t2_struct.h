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
    uint64_t proxByteOffset;
    uint32_t nroRegRem;
} T2Header;

typedef struct T2Registry {
    char removido;
    uint64_t tamanhoRegistro;
    int64_t prox;
    uint32_t id;
    uint32_t ano;
    uint32_t qtt;
    char sigla[REGISTRY_SIGLA_SIZE];
    uint32_t tamCidade;
    char codC5[1];
    char* cidade;
    uint32_t tamMarca;
    char codC6[1];
    char* marca;
    uint32_t tamModelo;
    char codC7[1];
    char* modelo;
} T2Registry;

void t2_write_header(FILE* dest, T2Header* header);
void t2_read_header(FILE* src, T2Header* header);

void t2_write_registry(FILE* dest, T2Registry* registry);
#pragma once

#include<stdio.h>
#include <stdint.h>

#include "common.h"

// T1Header field sizes
#define T1_HEADER_DESCRIPTION_SIZE 40
#define T1_HEADER_DES_C1_SIZE 22
#define T1_HEADER_DES_C2_SIZE 19
#define T1_HEADER_DES_C3_SIZE 24
#define T1_HEADER_DES_C4_SIZE 8
#define T1_HEADER_COD_C5_SIZE 1
#define T1_HEADER_DES_C5_SIZE 16
#define T1_HEADER_COD_C6_SIZE 1
#define T1_HEADER_DES_C6_SIZE 18
#define T1_HEADER_COD_C7_SIZE 1
#define T1_HEADER_DES_C7_SIZE 19
#define T1_HEADER_SIZE 182

#define T1_TOTAL_REGISTRY_SIZE 97

typedef struct T1Header {
  char status;
  int32_t topo;
  char descricao[T1_HEADER_DESCRIPTION_SIZE];
  char desC1[T1_HEADER_DES_C1_SIZE];
  char desC2[T1_HEADER_DES_C2_SIZE];
  char desC3[T1_HEADER_DES_C3_SIZE];
  char desC4[T1_HEADER_DES_C4_SIZE];
  char codC5[T1_HEADER_COD_C5_SIZE];
  char desC5[T1_HEADER_DES_C5_SIZE];
  char codC6[T1_HEADER_COD_C6_SIZE];
  char desC6[T1_HEADER_DES_C6_SIZE];
  char codC7[T1_HEADER_COD_C7_SIZE];
  char desC7[T1_HEADER_DES_C7_SIZE];
  uint32_t proxRRN;
  uint32_t nroRegRem;
} T1Header;

typedef struct T1Registry {
  char removido;
  int32_t prox;
  uint32_t id;
  uint32_t ano;
  uint32_t qtt;
  char sigla[REGISTRY_SIGLA_SIZE];
  strlen_t tamCidade;
  char codC5[CODE_FIELD_LEN];
  char* cidade;
  strlen_t tamMarca;
  char codC6[CODE_FIELD_LEN];
  char* marca;
  strlen_t tamModelo;
  char codC7[CODE_FIELD_LEN];
  char* modelo;
} T1Registry;

static const size_t T1_STATIC_REGISTRY_SIZE = member_size(T1Registry, removido)
                                       + member_size(T1Registry, prox)
                                       + member_size(T1Registry, id)
                                       + member_size(T1Registry, ano)
                                       + member_size(T1Registry, qtt)
                                       + member_size(T1Registry, sigla)
                                       + member_size(T1Registry, tamCidade)
                                       + member_size(T1Registry, codC5)
                                       + member_size(T1Registry, tamMarca)
                                       + member_size(T1Registry, codC6)
                                       + member_size(T1Registry, tamModelo)
                                       + member_size(T1Registry, codC7);


size_t t1_write_header(T1Header* header, FILE* dest);
T1Header* t1_read_header(FILE* src);

size_t t1_write_registry(FILE* dest, T1Registry* registry);
T1Registry* t1_read_registry(FILE* src);

T1Header* t1_new_header();
T1Registry* t1_new_registry();

void t1_destroy_header(T1Header* header);
void t1_destroy_registry(T1Registry* registry);

#pragma once

#include<stdio.h>
#include <stdint.h>

// Seamlessly retrieve the size of a struct member in compile-time
// https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
#ifndef member_size
#define member_size(type, member) sizeof(((type *)0)->member)
#endif

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

#define T1_REGISTRY_SIGLA_SIZE 2
#define T1_TOTAL_REGISTRY_SIZE 97

typedef struct T1Header {
  char status;
  uint32_t topo;
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
  uint32_t prox;
  uint32_t id;
  uint32_t ano;
  uint32_t qtt;
  char sigla[T1_REGISTRY_SIGLA_SIZE];
  uint32_t tamCidade;
  char codC5[1];
  char* cidade;
  uint32_t tamMarca;
  char codC6[1];
  char* marca;
  uint32_t tamModelo;
  char codC7[1];
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

static const char FILLER_BYTE[1] = {'$'};

void t1_write_header(FILE* dest, T1Header* header);
void t1_read_header(FILE* src, T1Header* header);

void t1_write_registry(FILE* dest, T1Registry* registry);
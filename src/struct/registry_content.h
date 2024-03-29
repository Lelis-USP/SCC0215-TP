/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "common.h"

#define HEADER_DESCRIPTION_SIZE 40
#define HEADER_DES_C1_SIZE 22
#define HEADER_DES_C2_SIZE 19
#define HEADER_DES_C3_SIZE 24
#define HEADER_DES_C4_SIZE 8
#define HEADER_COD_C5_SIZE 1
#define HEADER_DES_C5_SIZE 16
#define HEADER_COD_C6_SIZE 1
#define HEADER_DES_C6_SIZE 18
#define HEADER_COD_C7_SIZE 1
#define HEADER_DES_C7_SIZE 19

/**
 * Registry content struct
 */
typedef struct RegistryContent {
    int32_t id;
    int32_t ano;
    int32_t qtt;
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
} RegistryContent;

/**
 * Header content struct
 */
typedef struct HeaderContent {
    char descricao[HEADER_DESCRIPTION_SIZE];
    char desC1[HEADER_DES_C1_SIZE];
    char desC2[HEADER_DES_C2_SIZE];
    char desC3[HEADER_DES_C3_SIZE];
    char desC4[HEADER_DES_C4_SIZE];
    char codC5[HEADER_COD_C5_SIZE];
    char desC5[HEADER_DES_C5_SIZE];
    char codC6[HEADER_COD_C6_SIZE];
    char desC6[HEADER_DES_C6_SIZE];
    char codC7[HEADER_COD_C7_SIZE];
    char desC7[HEADER_DES_C7_SIZE];
} HeaderContent;

// File I/O //

/**
 * Writes the header contents into the given file
 * @param header_content target header contents
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_header_content(HeaderContent* header_content, FILE* dest);

/**
 * Reads the header contents from the given file
 * @param header_content target header contents
 * @param src source file
 * @return amount of bytes read
 */
size_t read_header_content(HeaderContent* header_content, FILE* src);

/**
 * Writes the registry contents into the given file
 * @param registry_content target registry contents
 * @param dest destination file
 * @return amount of bytes written
 */
size_t write_registry_content(RegistryContent* registry_content, FILE* dest);

/**
 * Reads the registry contents from the given file
 * @param registry_content target registry contents on which the data will be read into
 * @param src source file
 * @param max_read_bytes maximum amount of bytes to be read
 * @return amount of bytes read
 */
size_t read_registry_content(RegistryContent* registry_content, FILE* src, size_t max_read_bytes);

// Setups //

/**
 * Setups a given header content
 * @param header_content
 */
void setup_header_content(HeaderContent* header_content);

/**
 * Setups a given registry content
 * @param registry_content
 */
void setup_registry_content(RegistryContent* registry_content);

// Allocators //

/**
 * Allocates and setups a new header content
 * @return the new header content ptr
 */
HeaderContent* new_header_content();

/**
 * Allocates and setups a new registry content
 * @return the new registry content ptr
 */
RegistryContent* new_registry_content();

// Destroy //

/**
 * Destroys (frees) the given header content
 * @param header_content target header content
 */
void destroy_header_content(HeaderContent* header_content);

/**
 * Destroys (frees) the given registry content
 * @param registry_content target registry content
 */
void destroy_registry_content(RegistryContent* registry_content);
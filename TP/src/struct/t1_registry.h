#pragma once

#include "common.h"
#include "registry_content.h"
#include "registry.h"

#define T1_HEADER_SIZE 182
#define T1_REGISTRY_SIZE 97

typedef struct T1HeaderMetadata {
    char status;
    int32_t topo;
    int32_t proxRRN;
    int32_t nroRegRem;
} T1HeaderMetadata;

typedef struct T1RegistryMetadata {
    char removido;
    int32_t prox;
} T1RegistryMetadata;

size_t t1_write_header(Header* header, FILE* dest);
size_t t1_read_header(Header* header, FILE* src);

size_t t1_write_registry(Registry* registry, FILE* dest);
size_t t1_read_registry(Registry* registry, FILE* src);

void t1_setup_header_metadata(T1HeaderMetadata* header_metadata);
void t1_setup_registry_metadata(T1RegistryMetadata* registry_metadata);

T1HeaderMetadata* t1_new_header_metadata();
T1RegistryMetadata* t1_new_registry_metadata();

void t1_destroy_header_metadata(T1HeaderMetadata* header_metadata);
void t1_destroy_registry_metadata(T1RegistryMetadata* registry_metadata);

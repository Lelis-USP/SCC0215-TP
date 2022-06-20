#pragma once

#include "common.h"
#include "registry_content.h"
#include "registry.h"

typedef struct T2HeaderMetadata {
    char status;
    int64_t topo;
    int64_t proxByteOffset;
    int32_t nroRegRem;
} T2HeaderMetadata;

typedef struct T2RegistryMetadata {
    char removido;
    uint32_t tamanhoRegistro;
    int64_t prox;
} T2RegistryMetadata;

static const size_t T2_IGNORED_SIZE = member_size(T2RegistryMetadata, removido) + member_size(T2RegistryMetadata, tamanhoRegistro);

size_t t2_write_header(Header* header, FILE* dest);
size_t t2_read_header(Header* header, FILE* src);

size_t t2_registry_size(Registry* registry);
size_t t2_write_registry(Registry* registry, FILE* dest);
size_t t2_read_registry(Registry* registry, FILE* src);

void t2_setup_header_metadata(T2HeaderMetadata* header_metadata);
void t2_setup_registry_metadata(T2RegistryMetadata* registry_metadata);

T2HeaderMetadata* t2_new_header_metadata();
T2RegistryMetadata* t2_new_registry_metadata();

void t2_destroy_header_metadata(T2HeaderMetadata* header_metadata);
void t2_destroy_registry_metadata(T2RegistryMetadata* registry_metadata);

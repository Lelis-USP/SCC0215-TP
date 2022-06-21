#pragma once

#include <stdbool.h>

#include "common.h"
#include "registry_content.h"

typedef enum RegistryType {
    UNKNOWN = 0,
    FIX_LEN = 1,
    VAR_LEN = 2
} RegistryType;

typedef struct Header {
    void* header_metadata;
    HeaderContent* header_content;
    RegistryType registry_type;
} Header;

typedef struct Registry {
    void* registry_metadata;
    RegistryContent* registry_content;
    RegistryType registry_type;
} Registry;

// Memory management
void setup_header(Header* header);
void setup_registry(Registry* registry);

Header* new_header();
Registry* new_registry();

void destroy_header(Header* header);
void destroy_registry(Registry* registry);

Header* build_header(RegistryType registry_type);
Header* build_default_header(RegistryType registry_type);
Registry* build_registry(Header* header);
Registry* build_registry_from_type(RegistryType registry_type);

// File I/O
size_t write_header(Header* header, FILE* dest);
size_t read_header(Header* header, FILE* src);

size_t write_registry(Registry* registry, FILE* dest);
size_t read_registry(Registry* registry, FILE* src);

// Utils
bool is_registry_removed(Registry* registry);

void set_header_status(Header* header, char status);

void header_increment_next(Header* header, size_t appended_bytes);

#include "registry.h"

#include <stdlib.h>
#include <string.h>

#include "../const/const.h"
#include "../exception/exception.h"
#include "t1_registry.h"
#include "t2_registry.h"

void setup_header(Header* header) {
    switch (header->registry_type) {
        case FIX_LEN:
            // Setup header content
            if (header->header_content == NULL) {
                header->header_content = new_header_content();
            } else {
                setup_header_content(header->header_content);
            }

            // Setup header metadata
            if (header->header_metadata == NULL) {
                header->header_metadata = t1_new_header_metadata();
            } else {
                t1_setup_header_metadata(header->header_metadata);
            }
            break;
        case VAR_LEN:
            // Setup header content
            if (header->header_content == NULL) {
                header->header_content = new_header_content();
            } else {
                setup_header_content(header->header_content);
            }

            // Setup header metadata
            if (header->header_metadata == NULL) {
                header->header_metadata = t2_new_header_metadata();
            } else {
                t2_setup_header_metadata(header->header_metadata);
            }
            break;
        default:
            header->registry_type = UNKNOWN;
            header->header_metadata = NULL;
            header->header_content = NULL;
            break;
    }
}

void setup_registry(Registry* registry) {
    switch (registry->registry_type) {
        case FIX_LEN:
            // Setup registry content
            if (registry->registry_content == NULL) {
                registry->registry_content = new_registry_content();
            } else {
                setup_registry_content(registry->registry_content);
            }

            // Setup registry metadata
            if (registry->registry_metadata == NULL) {
                registry->registry_metadata = t1_new_registry_metadata();
            } else {
                t1_setup_registry_metadata(registry->registry_metadata);
            }
            break;
        case VAR_LEN:
            // Setup registry content
            if (registry->registry_content == NULL) {
                registry->registry_content = new_registry_content();
            } else {
                setup_registry_content(registry->registry_content);
            }

            // Setup registry metadata
            if (registry->registry_metadata == NULL) {
                registry->registry_metadata = t2_new_registry_metadata();
            } else {
                t2_setup_registry_metadata(registry->registry_metadata);
            }
            break;
        default:
            registry->registry_type = UNKNOWN;
            registry->registry_content = NULL;
            registry->registry_metadata = NULL;
            break;
    }
}

Header* new_header() {
    Header* header = malloc(sizeof (struct Header));
    header->registry_type = UNKNOWN;
    setup_header(header);
    return header;
}
Registry* new_registry() {
    Registry* registry = malloc(sizeof (struct Registry));
    registry->registry_type = UNKNOWN;
    setup_registry(registry);
    return registry;
}

void destroy_header(Header* header) {
    if (header == NULL) {
        return;
    }

    // Destroy content
    destroy_header_content(header->header_content);

    // Destroy metadata
    switch (header->registry_type) {
        case FIX_LEN:
            t1_destroy_header_metadata(header->header_metadata);
            break;
        case VAR_LEN:
            t2_destroy_header_metadata(header->header_metadata);
            break;
        default:
            break;
    }

    // Destroy container
    free(header);
}

void destroy_registry(Registry* registry) {
    if (registry == NULL) {
        return;
    }

    // Destroy content
    destroy_registry_content(registry->registry_content);

    // Destroy metadata
    switch (registry->registry_type) {
        case FIX_LEN:
            t1_destroy_registry_metadata(registry->registry_metadata);
            break;
        case VAR_LEN:
            t2_destroy_registry_metadata(registry->registry_metadata);
            break;
        default:
            break;
    }

    // Destroy container
    free(registry);
}

Header* build_header(RegistryType registry_type) {
    Header* header = new_header();
    header->registry_type = registry_type;
    setup_header(header);
    return header;
}

Header* build_default_header(RegistryType registry_type) {
    Header* header = new_header();
    header->registry_type = registry_type;
    setup_header(header);

    switch (registry_type) {
        case FIX_LEN:
            memcpy(header->header_metadata, &DEFAULT_T1_HEADER_METADATA, sizeof (struct T1HeaderMetadata));
            memcpy(header->header_content, &DEFAULT_HEADER_CONTENT, sizeof (struct HeaderContent));
            break;
        case VAR_LEN:
            memcpy(header->header_metadata, &DEFAULT_T2_HEADER_METADATA, sizeof (struct T2HeaderMetadata));
            memcpy(header->header_content, &DEFAULT_HEADER_CONTENT, sizeof (struct HeaderContent));
            break;
        default:
            ex_raise(EX_GENERIC_ERROR);
            break;
    }

    return header;
}

Registry* build_registry(Header* header) {
    Registry* registry = new_registry();
    registry->registry_type = header->registry_type;
    setup_registry(registry);
    return registry;
}

Registry* build_registry_from_type(RegistryType registry_type) {
    Registry* registry = new_registry();
    registry->registry_type = registry_type;
    setup_registry(registry);
    return registry;
}

// File I/O
size_t write_header(Header* header, FILE* dest) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;
    switch (header->registry_type) {
        case FIX_LEN:
            written_bytes += t1_write_header(header, dest);
            break;
        case VAR_LEN:
            written_bytes += t2_write_header(header, dest);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return written_bytes;
}

size_t read_header(Header* header, FILE* src) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    setup_header(header);

    size_t read_bytes = 0;

    switch (header->registry_type) {
        case FIX_LEN:
            read_bytes += t1_read_header(header, src);
            break;
        case VAR_LEN:
            read_bytes += t2_read_header(header, src);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return read_bytes;
}

size_t write_registry(Registry* registry, FILE* dest) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;

    switch (registry->registry_type) {
        case FIX_LEN:
            written_bytes += t1_write_registry(registry, dest);
            break;
        case VAR_LEN:
            written_bytes += t2_write_registry(registry, dest);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return written_bytes;
}

size_t read_registry(Registry* registry, FILE* src) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    setup_registry(registry);

    size_t read_bytes = 0;

    switch (registry->registry_type) {
        case FIX_LEN:
            read_bytes += t1_read_registry(registry, src);
            break;
        case VAR_LEN:
            read_bytes += t2_read_registry(registry, src);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
    }

    return read_bytes;
}

bool is_registry_removed(Registry* registry) {
    ex_assert(registry->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (registry->registry_type == FIX_LEN) {
        return ((T1RegistryMetadata*) registry->registry_metadata)->removido == REMOVED;
    }

    if (registry->registry_type == VAR_LEN) {
        return ((T2RegistryMetadata*) registry->registry_metadata)->removido == REMOVED;
    }

    return false;
}

void set_header_status(Header* header, char status) {
    ex_assert(header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->status = status;
    }

    if (header->registry_type == VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->status = status;
    }
}

char get_header_status(Header* header) {
    ex_assert(header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->status;
    }

    if (header->registry_type == VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->status;
    }

    return -1;
}

void header_increment_next(Header* header, size_t appended_bytes) {
    ex_assert(header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);


    if (header->registry_type == FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        if (appended_bytes != 0) {
            header_metadata->proxRRN++;
        }
    }

    if (header->registry_type == VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->proxByteOffset += (int64_t) appended_bytes;
    }
}

size_t get_max_offset(Header* header) {
    ex_assert(header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        return T1_HEADER_SIZE + ((size_t) header_metadata->proxRRN * T1_REGISTRY_SIZE);
    }

    if (header->registry_type == VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->proxByteOffset;
    }

    return -1;
}

bool seek_registry(Header* header, FILE* file, size_t target) {
    ex_assert(header->registry_type != UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        size_t offset = T1_HEADER_SIZE + (target * T1_REGISTRY_SIZE);
        size_t max_offset = T1_HEADER_SIZE + (header_metadata->proxRRN * T1_REGISTRY_SIZE);
        if (offset >= max_offset) {
            fseek(file, (long) max_offset, SEEK_SET);
            return false;
        }

        fseek(file, (long) offset, SEEK_SET);
        return true;
    }

    if (header->registry_type == VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;

        if (header_metadata->proxByteOffset >= target) {
            fseek(file, (long) header_metadata->proxByteOffset, SEEK_SET);
            return false;
        }

        fseek(file, (long) target, SEEK_SET);
        return true;
    }

    return false;
}
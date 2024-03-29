/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "registry.h"

#include <stdlib.h>
#include <string.h>

#include "../const/const.h"
#include "../exception/exception.h"
#include "../utils/utils.h"
#include "t1_registry.h"
#include "t2_registry.h"

/**
 * Setup an already allocated header with default NULL-equivalent data
 * @param header the target header pointer
 */
void setup_header(Header* header) {
    switch (header->registry_type) {
        case RT_FIX_LEN:
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
        case RT_VAR_LEN:
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
            header->registry_type = RT_UNKNOWN;
            header->header_metadata = NULL;
            header->header_content = NULL;
            break;
    }
}

/**
 * Setup an already allocated registry with default NULL-equivalent data
 * @param registry the target registry pointer
 */
void setup_registry(Registry* registry) {
    registry->offset = SIZE_MAX;
    switch (registry->registry_type) {
        case RT_FIX_LEN:
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
        case RT_VAR_LEN:
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
            registry->registry_type = RT_UNKNOWN;
            registry->registry_content = NULL;
            registry->registry_metadata = NULL;
            break;
    }
}

/**
 * Allocates a new header with an unknown type
 * @return the allocated header
 */
Header* new_header() {
    Header* header = malloc(sizeof(struct Header));
    header->registry_type = RT_UNKNOWN;
    setup_header(header);
    return header;
}

/**
 * Allocates a new registry with an unkwnown type
 * @return the allocated registry
 */
Registry* new_registry() {
    Registry* registry = malloc(sizeof(struct Registry));
    registry->registry_type = RT_UNKNOWN;
    setup_registry(registry);
    return registry;
}

/**
 * Deallocates an existing header and all of its components
 * @param header the target header
 */
void destroy_header(Header* header) {
    if (header == NULL) {
        return;
    }

    // Destroy content
    destroy_header_content(header->header_content);

    // Destroy metadata
    switch (header->registry_type) {
        case RT_FIX_LEN:
            t1_destroy_header_metadata(header->header_metadata);
            break;
        case RT_VAR_LEN:
            t2_destroy_header_metadata(header->header_metadata);
            break;
        default:
            break;
    }

    // Destroy container
    free(header);
}

/**
 * Deallocates an existing registry and all of its components
 * @param registry the target registry
 */
void destroy_registry(Registry* registry) {
    if (registry == NULL) {
        return;
    }

    // Destroy content
    destroy_registry_content(registry->registry_content);

    // Destroy metadata
    switch (registry->registry_type) {
        case RT_FIX_LEN:
            t1_destroy_registry_metadata(registry->registry_metadata);
            break;
        case RT_VAR_LEN:
            t2_destroy_registry_metadata(registry->registry_metadata);
            break;
        default:
            break;
    }

    // Destroy container
    free(registry);
}

/**
 * Allocates a header and setup it (allocates its components) for a target registry type
 * @param registry_type the header registry type
 * @return the allocated and set-up header
 */
Header* build_header(RegistryType registry_type) {
    Header* header = new_header();
    header->registry_type = registry_type;
    setup_header(header);
    return header;
}

/**
 * Allocates a header and setup it with the default data for a target registry type
 * @param registry_type the header registry type
 * @return the allocated and set-up header
 */
Header* build_default_header(RegistryType registry_type) {
    Header* header = new_header();
    header->registry_type = registry_type;
    setup_header(header);

    switch (registry_type) {
        case RT_FIX_LEN:
            memcpy(header->header_metadata, &DEFAULT_T1_HEADER_METADATA, sizeof(struct T1HeaderMetadata));
            memcpy(header->header_content, &DEFAULT_HEADER_CONTENT, sizeof(struct HeaderContent));
            break;
        case RT_VAR_LEN:
            memcpy(header->header_metadata, &DEFAULT_T2_HEADER_METADATA, sizeof(struct T2HeaderMetadata));
            memcpy(header->header_content, &DEFAULT_HEADER_CONTENT, sizeof(struct HeaderContent));
            break;
        default:
            ex_raise(EX_GENERIC_ERROR);
            break;
    }

    return header;
}

/**
 * Allocates a registry and setup it (allocates its components) for a target header spec (and registry type)
 * @param header the header of which the registry is associated
 * @return the allocated and set-up registry
 */
Registry* build_registry(Header* header) {
    Registry* registry = new_registry();
    registry->registry_type = header->registry_type;
    setup_registry(registry);
    return registry;
}

/**
 * Allocates a registry and setup it (allocates its components) for a target registry type
 * @param registry_type the registry type
 * @return the allocated and set-up registry
 */
Registry* build_registry_from_type(RegistryType registry_type) {
    Registry* registry = new_registry();
    registry->registry_type = registry_type;
    setup_registry(registry);
    return registry;
}

// File I/O

/**
 * Writes a generic header into the given file (must be already on the top position)
 * @param header the header to be written
 * @param dest destination file
 * @return the amount of bytes written
 */
size_t write_header(Header* header, FILE* dest) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    size_t written_bytes = 0;
    switch (header->registry_type) {
        case RT_FIX_LEN:
            written_bytes += t1_write_header(header, dest);
            break;
        case RT_VAR_LEN:
            written_bytes += t2_write_header(header, dest);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
            break;
    }

    return written_bytes;
}

/**
 * Reads the header from a given file
 * @param header the header ptr on which the data will be written
 * @param src the source file
 * @return the amount of bytes read
 */
size_t read_header(Header* header, FILE* src) {
    ex_assert(header != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    setup_header(header);

    size_t read_bytes = 0;

    switch (header->registry_type) {
        case RT_FIX_LEN:
            read_bytes += t1_read_header(header, src);
            break;
        case RT_VAR_LEN:
            read_bytes += t2_read_header(header, src);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
            break;
    }

    return read_bytes;
}

/**
 * Writes a registry into the given file (at the current position)
 * @param registry the registry to be written
 * @param dest the destination file
 * @return the amount of bytes written
 */
size_t write_registry(Registry* registry, FILE* dest) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(dest != NULL, EX_FILE_ERROR);

    registry->offset = current_offset(dest);

    size_t written_bytes = 0;

    switch (registry->registry_type) {
        case RT_FIX_LEN:
            written_bytes += t1_write_registry(registry, dest);
            break;
        case RT_VAR_LEN:
            written_bytes += t2_write_registry(registry, dest);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
            break;
    }

    return written_bytes;
}

/**
 * Reads a registry from the given file
 * @param registry the registry ptr on which the data will be read into
 * @param src the source file
 * @return the amount of bytes read
 */
size_t read_registry(Registry* registry, FILE* src) {
    ex_assert(registry != NULL, EX_GENERIC_ERROR);
    ex_assert(src != NULL, EX_FILE_ERROR);

    setup_registry(registry);

    registry->offset = current_offset(src);

    size_t read_bytes = 0;

    switch (registry->registry_type) {
        case RT_FIX_LEN:
            read_bytes += t1_read_registry(registry, src);
            break;
        case RT_VAR_LEN:
            read_bytes += t2_read_registry(registry, src);
            break;
        default:
            ex_raise(EX_CORRUPTED_REGISTRY);
            break;
    }

    return read_bytes;
}

/**
 * Calculates the total registry size (actual number of bytes on the file).
 *
 * Includes ghost bytes, from re-using previously removed registries
 * @param registry target registry
 * @return the amount of bytes used by the registry
 */
size_t total_registry_size(Registry* registry) {
    ex_assert(registry->registry_type != RT_UNKNOWN, EX_GENERIC_ERROR);

    if (registry->registry_type == RT_FIX_LEN) {
        return T1_REGISTRY_SIZE;
    }

    if (registry->registry_type == RT_VAR_LEN) {
        T2RegistryMetadata* registry_metadata = registry->registry_metadata;
        size_t registry_size = max(t2_minimum_registry_size(registry), registry_metadata->tamanhoRegistro);
        return T2_IGNORED_SIZE + registry_size;
    }

    return 0;
}

/**
 * Remove registry from file, updating the removal list and making any required updates
 * @param header target file header
 * @param registry target registry to remove (must've been read from the file)
 * @param file target file
 */
void remove_registry(Header* header, Registry* registry, FILE* file) {
    ex_assert(registry->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);
    ex_assert(registry->offset != SIZE_MAX, EX_CORRUPTED_REGISTRY);
    ex_assert(header->registry_type == registry->registry_type, EX_CORRUPTED_REGISTRY);
    ex_assert(file != NULL, EX_FILE_ERROR);

    if (registry->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        T1RegistryMetadata* registry_metadata = registry->registry_metadata;

        // Update metadata for removal status
        registry_metadata->removido = REMOVED;
        registry_metadata->prox = header_metadata->topo;

        // Go to the beginning of the target registry
        go_to_registry(registry, file);
        write_registry(registry, file);

        // Update header removal references
        header_metadata->topo = (int32_t) get_registry_reference(header, registry->offset);
        header_metadata->nroRegRem++;
    }

    if (registry->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        T2RegistryMetadata* registry_metadata = registry->registry_metadata;

        // Go to the beginning of the target registry
        int64_t current_top = header_metadata->topo;

        if (current_top != -1) {
            // Load queue front registry
            seek_registry(header, file, current_top);
            Registry* prev_registry = NULL;
            Registry* cur_registry = build_registry(header);
            read_registry(cur_registry, file);

            // Ordered-insert the removed registry in the queue
            while (cur_registry != NULL && ((T2RegistryMetadata*) cur_registry->registry_metadata)->tamanhoRegistro >= registry_metadata->tamanhoRegistro) {
                T2RegistryMetadata* cur_registry_metadata = cur_registry->registry_metadata;

                // If prev doesn't exist, allocate new registry for it
                if (prev_registry == NULL) {
                    prev_registry = build_registry(header);
                }

                // Swap pointers to keep past copy
                Registry* tmp = cur_registry;
                cur_registry = prev_registry;
                prev_registry = tmp;

                if (cur_registry_metadata->prox == -1) {
                    destroy_registry(cur_registry);
                    cur_registry = NULL;
                } else {
                    // Read new registry on the queue
                    seek_registry(header, file, cur_registry_metadata->prox);
                    read_registry(cur_registry, file);
                }
            }

            // Biggest size
            if (prev_registry == NULL) {
                header_metadata->topo = (int64_t) get_registry_reference(header, registry->offset);
                registry_metadata->prox = (int64_t) get_registry_reference(header, cur_registry->offset);
            } else {
                // Update previous registry's next reference
                T2RegistryMetadata* prev_registry_metadata = prev_registry->registry_metadata;
                prev_registry_metadata->prox = (int64_t) get_registry_reference(header, registry->offset);
                go_to_registry(prev_registry, file);
                write_registry(prev_registry, file);

                // Update removed registry prox reference
                if (cur_registry == NULL) {// List end
                    registry_metadata->prox = -1;
                } else {// Middle of list
                    registry_metadata->prox = (int64_t) get_registry_reference(header, cur_registry->offset);
                }
            }

            destroy_registry(cur_registry);
            destroy_registry(prev_registry);
        } else {
            header_metadata->topo = (int64_t) get_registry_reference(header, registry->offset);
            registry_metadata->prox = -1;
        }

        // Update header removed reg count
        header_metadata->nroRegRem++;

        // Update removed registry
        go_to_registry(registry, file);
        registry_metadata->removido = REMOVED;
        write_registry(registry, file);
    }
}

/**
 * Add registry to the given file (reuses previously removed ones if possible)
 * @param header target file header
 * @param registry target registry to add
 * @param file target file
 */
void add_registry(Header* header, Registry* registry, FILE* file) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);
    ex_assert(file != NULL, EX_FILE_ERROR);

    if (header->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;

        // Reset registry metadata
        t1_setup_registry_metadata(registry->registry_metadata);

        int32_t stack_top = header_metadata->topo;
        uint32_t write_location = header_metadata->proxRRN;

        if (stack_top != -1) {
            // Load top registry
            Registry* top_registry = build_registry(header);
            seek_registry(header, file, stack_top);
            read_registry(top_registry, file);

            T1RegistryMetadata* top_registry_metadata = top_registry->registry_metadata;
            ex_assert(top_registry_metadata->removido == REMOVED, EX_FILE_ERROR);

            // Update header
            header_metadata->topo = top_registry_metadata->prox;
            header_metadata->nroRegRem--;

            // Overwrite deleted registry
            write_location = stack_top;

            destroy_registry(top_registry);
        } else {
            // Appending to the end of file, so increase the next RRN
            header_metadata->proxRRN++;
        }

        // Go to the beginning of the target registry
        seek_registry(header, file, write_location);
        write_registry(registry, file);
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        T2RegistryMetadata* registry_metadata = registry->registry_metadata;

        // Reset registry metadata
        t2_setup_registry_metadata(registry->registry_metadata);

        int64_t queue_front = header_metadata->topo;
        size_t write_offset = header_metadata->proxByteOffset;

        if (queue_front != -1) {
            // Load top registry
            Registry* front_registry = build_registry(header);
            seek_registry(header, file, queue_front);
            read_registry(front_registry, file);

            T2RegistryMetadata* front_registry_metadata = front_registry->registry_metadata;
            ex_assert(front_registry_metadata->removido == REMOVED, EX_FILE_ERROR);

            // Check if registry fits
            if (front_registry_metadata->tamanhoRegistro + T2_IGNORED_SIZE >= total_registry_size(registry)) {
                header_metadata->topo = front_registry_metadata->prox;
                header_metadata->nroRegRem--;

                write_offset = front_registry->offset;
                // Copy registry size
                registry_metadata->tamanhoRegistro = front_registry_metadata->tamanhoRegistro;
            }
            destroy_registry(front_registry);
        }

        // Go to the beginning of the target registry
        seek_registry(header, file, write_offset);
        write_registry(registry, file);

        // Update header's proxByteOffset reference, if needed
        header_metadata->proxByteOffset = (int64_t) max(header_metadata->proxByteOffset, current_offset(file));
    }
}

/**
 * Update an already existing registry on the file (handle size changes and other shenanigans)
 * @param header target file header
 * @param registry target registry to update (with modified data and appropriate offset)
 * @param file target file
 * @return if the update resulted in a new registry offset
 */
bool update_registry(Header* header, Registry* registry, FILE* file) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);
    ex_assert(registry->offset != SIZE_MAX, EX_CORRUPTED_REGISTRY);
    ex_assert(file != NULL, EX_FILE_ERROR);

    // Fixed Len always can overwrite
    if (header->registry_type == RT_FIX_LEN) {
        // Go to the beginning of the target registry
        go_to_registry(registry, file);
        write_registry(registry, file);
        return false;
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2RegistryMetadata* registry_metadata = registry->registry_metadata;

        size_t new_size = total_registry_size(registry);

        // Can overwrite
        if (new_size <= T2_IGNORED_SIZE + registry_metadata->tamanhoRegistro) {
            go_to_registry(registry, file);
            write_registry(registry, file);
            return false;
        } else {
            remove_registry(header, registry, file);
            add_registry(header, registry, file);
            return true;
        }
    }

    return false;
}

/**
 * Jump to the target registry on the file (based on its read offset)
 * @param registry target registry
 * @param file target file
 */
void go_to_registry(Registry* registry, FILE* file) {
    go_to_offset(registry->offset, file);
}

/**
 * Jump to the given offset in the file
 *
 * yes, this is just an fseek alias
 * @param offset target offset
 * @param file target file
 */
void go_to_offset(size_t offset, FILE* file) {
    fseek(file, (long) offset, SEEK_SET);
}

/**
 * Retrieve current offset on the file
 *
 * yes, this is just an ftell alias
 * @param file target file
 * @return current file offset
 */
size_t current_offset(FILE* file) {
    return (size_t) ftell(file);
}

/**
 * Utility to check if a registry was removed
 * @param registry target registry
 * @return if the registry is removed
 */
bool is_registry_removed(Registry* registry) {
    ex_assert(registry->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (registry->registry_type == RT_FIX_LEN) {
        return ((T1RegistryMetadata*) registry->registry_metadata)->removido == REMOVED;
    }

    if (registry->registry_type == RT_VAR_LEN) {
        return ((T2RegistryMetadata*) registry->registry_metadata)->removido == REMOVED;
    }

    return false;
}

/**
 * Utility to update a header status
 * @param header the target header
 * @param status the new header status
 */
void set_header_status(Header* header, char status) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->status = status;
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->status = status;
    }
}

/**
 * Utility to retrieve a header status
 * @param header the target header
 * @return the current header status
 */
char get_header_status(Header* header) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->status;
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->status;
    }

    return -1;
}

/**
 * Increments the header's next open position reference (RRN or byte offset, depending on file type)
 * @param header the target header
 * @param appended_bytes the number of bytes appended to the end of the file on the last write
 */
void header_increment_next(Header* header, size_t appended_bytes) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);


    if (header->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        if (appended_bytes != 0) {
            header_metadata->proxRRN++;
        }
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        header_metadata->proxByteOffset += (int64_t) appended_bytes;
    }
}

/**
 * Computes the file's end reference based off the next open position reference
 * @param header the target header
 * @return the next offset after the end (aka, loop until less than, not equal)
 */
size_t get_max_offset(Header* header) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == RT_FIX_LEN) {
        T1HeaderMetadata* header_metadata = header->header_metadata;
        return T1_HEADER_SIZE + ((size_t) header_metadata->proxRRN * T1_REGISTRY_SIZE);
    }

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;
        return header_metadata->proxByteOffset;
    }

    return -1;
}

/**
 * Seek for a given registry based on its RRN or offset. If the seek fails, the file position goes to the end.
 * @param header the file's header
 * @param file the target file
 * @param target the target position
 * @return if the seek was successful
 */
bool seek_registry(Header* header, FILE* file, size_t target) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == RT_FIX_LEN) {
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

    if (header->registry_type == RT_VAR_LEN) {
        T2HeaderMetadata* header_metadata = header->header_metadata;

        if (header_metadata->proxByteOffset <= target) {
            fseek(file, (long) header_metadata->proxByteOffset, SEEK_SET);
            return false;
        }

        fseek(file, (long) target, SEEK_SET);
        return true;
    }

    return false;
}

/**
 * Retrieves the registries relative position (RRN or byte offset, depending on registry type)
 * @param header the registry header
 * @param total_bytes_before_read the total amount of bytes read before the target element
 * @return the registry's relative position
 */
size_t get_registry_reference(Header* header, size_t total_bytes_before_read) {
    ex_assert(header->registry_type != RT_UNKNOWN, EX_CORRUPTED_REGISTRY);

    if (header->registry_type == RT_FIX_LEN) {
        size_t registries_read = total_bytes_before_read - T1_HEADER_SIZE;
        size_t rrn = registries_read / T1_REGISTRY_SIZE;
        return rrn;
    }

    return total_bytes_before_read;
}

/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "commands.h"

#include <stdlib.h>
#include <string.h>

#include "../const/const.h"
#include "../exception/exception.h"
#include "../utils/csv_parser.h"
#include "../utils/provided_functions.h"
#include "../utils/registry_loader.h"
#include "common.h"
#include "../index/index.h"


// Commands //

/**
 * Struct to hold shared information for the csv_stream processing function
 */
typedef struct CSVParseArgs {
    Header* header;
    Registry* registry;
    FILE* dest_file;
} CSVParseArgs;

/**
 * Function used for csv_stream, receives each CSV line, parse it into a registry and serialize it into the dest file
 * @param idx the line index
 * @param line the CSV line info
 * @param passthrough a CSVParseArgs pointer with the shared information from the original calling function
 */
void parse_csv_line(int idx, CSVLine* line, void* passthrough) {
    CSVParseArgs* args = passthrough;

    // Ignore header
    if (idx == 0) {
        return;
    }

    setup_registry(args->registry);
    load_registry_from_csv_line(args->registry, line);
    size_t appended_bytes = write_registry(args->registry, args->dest_file);
    header_increment_next(args->header, appended_bytes);
}

/**
 * Parse csv file and build registry
 * @param args command args
 */
void c_parse_and_serialize(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->secondary_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Open CSV file
    FILE* csv_file = fopen(args->primary_file, "r");
    if (csv_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Open destination file
    FILE* dest_file = fopen(args->secondary_file, "wb");
    if (dest_file == NULL) {
        puts(EX_FILE_ERROR);
        fclose(csv_file);
        return;
    }

    // Write default header with a bad status
    Header* header = build_default_header(args->registry_type);
    set_header_status(header, STATUS_BAD);
    write_header(header, dest_file);

    // Write registries
    Registry* registry = new_registry();
    registry->registry_type = args->registry_type;

    // Create shared data for stream passthrough
    CSVParseArgs csv_parse_args = {
            header,
            registry,
            dest_file};

    // CSV streaming: loop each line calling the before-defined parse_csv_line function
    stream_csv(csv_file, parse_csv_line, &csv_parse_args);

    // Cleanup
    destroy_registry(registry);
    fclose(csv_file);

    // Update status at beginning
    set_header_status(header, STATUS_GOOD);
    fseek(dest_file, 0, SEEK_SET);
    write_header(header, dest_file);

    // Cleanup
    fclose(dest_file);
    destroy_header(header);

    // Autocorrection stuff
    print_autocorrection_checksum(args->secondary_file);
}

/**
 * Deserialize a registry and prints its contents
 * @param args command args
 */
void c_deserialize_and_print(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Open source file
    FILE* file = fopen(args->primary_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);
    bool printed = false;

    // Check for read failure or bad status
    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
    } else {
        // Allocate shared registry (freed only at the end, information is always reset on the read_registry call)
        Registry* registry = build_registry(header);
        size_t max_offset = get_max_offset(header);

        // Loop each registry until reaching the file limit (defined on header)
        while (read_bytes < max_offset) {
            read_bytes += read_registry(registry, file);

            // On read failure or removal, skip
            if (registry == NULL || is_registry_removed(registry)) {
                continue;
            }

            print_registry(header, registry);
            printed = true;
        }

        // Cleanup
        destroy_registry(registry);

        // No registry found
        if (!printed) {
            puts(EX_REGISTRY_NOT_FOUND);
        }
    }

    // Cleanup
    destroy_header(header);
    fclose(file);
}

/**
 * Deserialize a registry and print everyone matching the given filter
 * @param args command args
 */
void c_deserialize_filter_and_print(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Extract filters from args (and type-cast it)
    FilterArgs* filters = args->specific_data;

    // Open source file
    FILE* file = fopen(args->primary_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);
    bool printed = false;

    // Check for read failure or bad status
    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
    } else {
        // Allocate shared registry (freed only at the end, information is always reset on the read_registry call)
        Registry* registry = build_registry(header);
        size_t max_offset = get_max_offset(header);

        // Loop each registry until reaching the file limit (defined on header)
        while (read_bytes < max_offset) {
            read_bytes += read_registry(registry, file);

            // On read failure, removal or no filter match, skip
            if (registry == NULL || is_registry_removed(registry) || !registry_filter_match(registry, filters)) {
                continue;
            }

            print_registry(header, registry);
            printed = true;
        }

        // Cleanup
        destroy_registry(registry);

        // No registry found
        if (!printed) {
            puts(EX_REGISTRY_NOT_FOUND);
        }
    }

    // Cleanup
    destroy_header(header);
    fclose(file);
}

/**
 * Try access a registry by its RRN and print it
 * @param args command args
 */
void c_deserialize_direct_access_rrn_and_print(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Check for registry type
    if (args->registry_type != FIX_LEN) {
        puts(EX_COMMAND_PARSE_ERROR);
        return;
    }

    // Extract rrn from args (and type-cast it)
    SearchByRRNArgs* rrn_args = args->specific_data;

    // Open source file
    FILE* file = fopen(args->primary_file, "rb");
    if (file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, file);

    // Check for read failure or bad status
    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(file);
        destroy_header(header);
        return;
    }

    // Jump to target position
    bool found_registry = seek_registry(header, file, rrn_args->rrn);
    // If the position is out of bounds, the registry is marked as not found
    if (!found_registry) {
        puts(EX_REGISTRY_NOT_FOUND);
        fclose(file);
        destroy_header(header);
        return;
    }

    // Read registry from target position
    Registry* registry = build_registry(header);
    read_registry(registry, file);

    // Check if the registry exists
    if (!is_registry_removed(registry)) {
        print_registry(header, registry);
    } else {
        puts(EX_REGISTRY_NOT_FOUND);
    }

    // Cleanup
    destroy_registry(registry);
    destroy_header(header);
    fclose(file);
}

/**
 * Build an index for the given registry
 * @param args command args
 */
void c_build_index_from_registry(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->secondary_file != NULL, EX_COMMAND_PARSE_ERROR);

    // Open registry_file
    FILE* registry_file = fopen(args->primary_file, "rb");
    if (registry_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t read_bytes = read_header(header, registry_file);

    // Create index header
    IndexHeader* index_header = new_index(args->registry_type);
    set_index_status(index_header, STATUS_BAD);

    // Check for read failure or bad status
    if (read_bytes == 0 || get_header_status(header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(registry_file);
        destroy_header(header);
        destroy_index_header(index_header);
        return;
    } else {
        // Allocate shared registry (freed only at the end, information is always reset on the read_registry call)
        Registry* registry = build_registry(header);
        size_t max_offset = get_max_offset(header);

        // Loop each registry until reaching the file limit (defined on header)
        while (read_bytes < max_offset) {
            size_t registry_reference = get_registry_reference(header, read_bytes);
            read_bytes += read_registry(registry, registry_file);

            // On read failure or removal, skip
            if (registry == NULL || is_registry_removed(registry)) {
                continue;
            }

            // Add registry to index
            bool success = index_add(index_header, registry->registry_content->id, registry_reference);

            // If the registry already exists
            if (!success) {
                puts(EX_FILE_ERROR);
                destroy_header(header);
                destroy_registry(registry);
                destroy_index_header(index_header);
                fclose(registry_file);
                return;
            }
        }

        // Cleanup
        destroy_registry(registry);
    }

    // Cleanup registry
    destroy_header(header);
    fclose(registry_file);

    // Open index_file
    FILE* index_file = fopen(args->secondary_file, "wb");
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Write index
    write_index(index_header, index_file);

    // Update index status
    fseek(index_file, 0, SEEK_SET);
    set_index_status(index_header, STATUS_GOOD);
    write_index_header(index_header, index_file);

    // Cleanup
    destroy_index_header(index_header);
    fclose(index_file);

    // Autocorrection stuff
    print_autocorrection_checksum(args->secondary_file);
}

/**
 * Logically remove a registry and update the given index
 * @param args command args
 */
void c_remove_registry(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->secondary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->specific_data != NULL, EX_COMMAND_PARSE_ERROR);

    RemovalArgs* removal_args = args->specific_data;

    // Open registry_file
    FILE* registry_file = fopen(args->primary_file, "rb+");
    if (registry_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Open index_file
    FILE* index_file = fopen(args->secondary_file, "rb+");
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t first_registry_offset = read_header(header, registry_file); // Store the offset of the first registry

    // Load index
    IndexHeader* index_header = new_index(args->registry_type);
    size_t read_bytes_index = read_index(index_header, index_file);

    // Check for read failure or bad status
    if (first_registry_offset == 0 || get_header_status(header) == STATUS_BAD || read_bytes_index == 0 || get_index_status(index_header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(registry_file);
        fclose(index_file);
        destroy_header(header);
        destroy_index_header(index_header);
        return;
    }

    // Update registry header status
    set_header_status(header, STATUS_BAD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Update index header status
    set_index_status(index_header, STATUS_BAD);
    fseek(index_file, 0, SEEK_SET);
    write_index_header(index_header, index_file);

    // Do each removal in the given order
    for (uint32_t i = 0; i < removal_args->n_removals; i++) {
        RemovalTarget current_removal = removal_args->removal_targets[i];

        if (current_removal.indexed_filter_args != NULL) {
            // Indexed cases //

            FilterArgs* filter_args = current_removal.indexed_filter_args;

            ex_assert(strcmp(filter_args->key, ID_FIELD_NAME) == 0, EX_COMMAND_PARSE_ERROR);
            ex_assert(filter_args->next == NULL, EX_COMMAND_PARSE_ERROR);

            // Retrieve filtered id
            int32_t id = parse_int32_filter(filter_args);

            // Search for ID on index
            IndexElement* index_match = index_query(index_header, id);

            // If not found, skip removal
            if (index_match == NULL) {
                continue;
            }

            // Load target registry
            Registry* registry = build_registry(header);
            seek_registry(header, registry_file, index_match->reference);
            read_registry(registry, registry_file);

            // Check if the registry exists and match the filters
            if (is_registry_removed(registry) || !registry_filter_match(registry, current_removal.unindexed_filter_args)) {
                destroy_registry(registry);
                continue;
            }

            // Remove the registry and update the index
            remove_registry(header, registry, registry_file);
            index_remove(index_header, id);

            // Clenaup
            destroy_registry(registry);
        } else {
            // Non-indexed cases //

            // Go to top of the registry for iteration
            go_to_offset(first_registry_offset, registry_file);
            size_t read_bytes_registry = first_registry_offset;

            // Allocate registry for reading
            Registry* registry = build_registry(header);

            // Compute the max offset for iteration
            size_t max_offset = get_max_offset(header);

            // Load filterls
            FilterArgs* filter_args = current_removal.unindexed_filter_args;

            // Loop each registry until reaching the file limit
            while (read_bytes_registry < max_offset) {
                // Load the registry
                read_bytes_registry += read_registry(registry, registry_file);

                // Check if registry is present and filter matches
                if (registry == NULL || is_registry_removed(registry) || !registry_filter_match(registry, filter_args)) {
                    continue;
                }

                // Remove matched registry
                size_t cur_offset = current_offset(registry_file); // Keep current offset to return to it in iteration
                remove_registry(header, registry, registry_file);
                go_to_offset(cur_offset, registry_file); // Return to offset to continue iteration

                // Removed registry from index
                index_remove(index_header, registry->registry_content->id);
            }

            // Cleanup
            destroy_registry(registry);
        }
    }

    // Update registry header
    set_header_status(header, STATUS_GOOD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Reopen the index file to update it (reopen is neccessary since truncation isn't part of the C ANSI standard, although there are POSIX specific implementations)
    index_file = freopen(args->secondary_file, "wb", index_file);
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Write the updated index
    fseek(index_file, 0, SEEK_SET);
    write_index(index_header, index_file);


    // Update index status
    fseek(index_file, 0, SEEK_SET);
    set_index_status(index_header, STATUS_GOOD);
    write_index_header(index_header, index_file);

    // Cleanup
    destroy_header(header);
    destroy_index_header(index_header);
    fclose(registry_file);
    fclose(index_file);

    // Autocorrection stuff
    print_autocorrection_checksum(args->primary_file);
    print_autocorrection_checksum(args->secondary_file);
}


/**
 * Insert registries into an existing file and udpate index
 * @param args command args
 */
void c_insert_registry(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->secondary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->specific_data != NULL, EX_COMMAND_PARSE_ERROR);

    InsertionArgs* insertion_args = args->specific_data;

    // Open registry_file
    FILE* registry_file = fopen(args->primary_file, "rb+");
    if (registry_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Open index_file
    FILE* index_file = fopen(args->secondary_file, "rb+");
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t first_registry_offset = read_header(header, registry_file);

    // Load index
    IndexHeader* index_header = new_index(args->registry_type);
    size_t read_bytes_index = read_index(index_header, index_file);

    // Check for read failure or bad status
    if (first_registry_offset == 0 || get_header_status(header) == STATUS_BAD || read_bytes_index == 0 || get_index_status(index_header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(registry_file);
        fclose(index_file);
        destroy_header(header);
        destroy_index_header(index_header);
        return;
    }

    // Update registry header status
    set_header_status(header, STATUS_BAD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Update index header status
    set_index_status(index_header, STATUS_BAD);
    fseek(index_file, 0, SEEK_SET);
    write_index_header(index_header, index_file);

    Registry* registry = build_registry(header);

    // Do each insertion in the given order
    for (uint32_t i = 0; i < insertion_args->n_insertions; i++) {
        InsertionTarget current_insertion = insertion_args->insertion_targets[i];

        // Load registry with insertion data //
        setup_registry(registry);
        registry->registry_content->id = current_insertion.id;
        registry->registry_content->ano = current_insertion.ano;
        registry->registry_content->qtt = current_insertion.qtt;

        for (uint32_t j = 0; j < REGISTRY_SIGLA_SIZE; j++) {
            registry->registry_content->sigla[j] = current_insertion.sigla[j];
        }

        if (current_insertion.cidade != NULL) {
            // Len
            size_t len_cidade = strlen(current_insertion.cidade);
            registry->registry_content->tamCidade = len_cidade;
            // Code
            memcpy(registry->registry_content->codC5, header->header_content->codC5, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->cidade = malloc((len_cidade + 1) * sizeof (char));
            memcpy(registry->registry_content->cidade, current_insertion.cidade, len_cidade * sizeof (char));
            registry->registry_content->cidade[len_cidade] = '\0';
        }

        if (current_insertion.marca != NULL) {
            // Len
            size_t len_marca = strlen(current_insertion.marca);
            registry->registry_content->tamMarca = len_marca;
            // Code
            memcpy(registry->registry_content->codC6, header->header_content->codC6, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->marca = malloc((len_marca + 1) * sizeof (char));
            memcpy(registry->registry_content->marca, current_insertion.marca, len_marca * sizeof (char));
            registry->registry_content->marca[len_marca] = '\0';
        }

        if (current_insertion.modelo != NULL) {
            // Len
            size_t len_modelo = strlen(current_insertion.modelo);
            registry->registry_content->tamModelo = len_modelo;
            // Code
            memcpy(registry->registry_content->codC7, header->header_content->codC7, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->modelo = malloc((len_modelo + 1) * sizeof (char));
            memcpy(registry->registry_content->modelo, current_insertion.modelo, len_modelo * sizeof (char));
            registry->registry_content->modelo[len_modelo] = '\0';
        }

        // Check for already existing ID
        if (index_query(index_header, current_insertion.id) == NULL) {
            // Write the registry
            add_registry(header, registry, registry_file);
            // Update the index
            index_add(index_header, registry->registry_content->id, get_registry_reference(header, registry->offset));
        }
    }

    // Cleanup
    destroy_registry(registry);

    // Update registry header
    set_header_status(header, STATUS_GOOD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Reopen index file
    index_file = freopen(args->secondary_file, "wb", index_file);
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Write updated index
    fseek(index_file, 0, SEEK_SET);
    write_index(index_header, index_file);


    // Update index status
    fseek(index_file, 0, SEEK_SET);
    set_index_status(index_header, STATUS_GOOD);
    write_index_header(index_header, index_file);

    // Cleanup
    destroy_header(header);
    destroy_index_header(index_header);
    fclose(registry_file);
    fclose(index_file);

    // Autocorrection stuff
    print_autocorrection_checksum(args->primary_file);
    print_autocorrection_checksum(args->secondary_file);
}

/**
 * Apply updates to the target registry
 * @param header the registry header
 * @param registry the target registry
 * @param update_target the update info
 * @return if a re-index is necessary (index key change)
 */
bool apply_registry_updates(Header* header, Registry* registry, UpdateTarget* update_target) {
    bool re_index = false;

    if (update_target->update_id) {
        registry->registry_content->id = update_target->id;
        re_index = true;
    }

    if (update_target->update_ano) {
        registry->registry_content->ano = update_target->ano;
    }

    if (update_target->update_qtt) {
        registry->registry_content->qtt = update_target->qtt;
    }

    if (update_target->update_sigla) {
        for (uint32_t j = 0; j < REGISTRY_SIGLA_SIZE; j++) {
            registry->registry_content->sigla[j] = update_target->sigla[j];
        }
    }

    if (update_target->update_cidade) {
        free(registry->registry_content->cidade);
        if (update_target->cidade != NULL) {
            // Len
            size_t len_cidade = strlen(update_target->cidade);
            registry->registry_content->tamCidade = len_cidade;
            // Code
            memcpy(registry->registry_content->codC5, header->header_content->codC5, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->cidade = malloc((len_cidade + 1) * sizeof (char));
            memcpy(registry->registry_content->cidade, update_target->cidade, len_cidade * sizeof (char));
            registry->registry_content->cidade[len_cidade] = '\0';
        } else {
            if (registry->registry_content->cidade != NULL) {
                registry->registry_content->tamCidade = 0;
                registry->registry_content->cidade = NULL;
            }
        }
    }

    if (update_target->update_marca) {
        free(registry->registry_content->marca);
        if (update_target->marca != NULL) {
            // Len
            size_t len_marca = strlen(update_target->marca);
            registry->registry_content->tamMarca = len_marca;
            // Code
            memcpy(registry->registry_content->codC6, header->header_content->codC6, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->marca = malloc((len_marca + 1) * sizeof (char));
            memcpy(registry->registry_content->marca, update_target->marca, len_marca * sizeof (char));
            registry->registry_content->marca[len_marca] = '\0';
        } else {
            if (registry->registry_content->marca != NULL) {
                registry->registry_content->tamMarca = 0;
                registry->registry_content->marca = NULL;
            }
        }
    }

    if (update_target->update_modelo) {
        free(registry->registry_content->modelo);
        if (update_target->modelo != NULL) {
            // Len
            size_t len_modelo = strlen(update_target->modelo);
            registry->registry_content->tamModelo = len_modelo;
            // Code
            memcpy(registry->registry_content->codC7, header->header_content->codC7, CODE_FIELD_LEN * sizeof (char));
            // Str Data
            registry->registry_content->modelo = malloc((len_modelo + 1) * sizeof (char));
            memcpy(registry->registry_content->modelo, update_target->modelo, len_modelo * sizeof (char));
            registry->registry_content->modelo[len_modelo] = '\0';
        } else {
            if (registry->registry_content->modelo!= NULL) {
                registry->registry_content->tamModelo = 0;
                registry->registry_content->modelo = NULL;
            }
        }
    }

    return re_index;
}

/**
 * Execute the update for a given registry, changing the registry file and index
 * @param header target registry's header
 * @param registry target registry
 * @param current_update the update to be applied
 * @param registry_file the registry's file
 * @param index_header the index associated to the registry
 * @return if the update was executed
 */
bool execute_update(Header* header, Registry* registry, UpdateTarget* current_update, FILE* registry_file, IndexHeader* index_header) {
    // Check if there will be conflicting ids
    if (current_update->update_id) {
        if (current_update->id != registry->registry_content->id && index_query(index_header, current_update->id) != NULL) {
            return false;
        }
    }

    // Keep track of old id, in case of an update
    int32_t old_id = registry->registry_content->id;
    bool reindex = apply_registry_updates(header, registry, current_update);

    // Updates the registry
    bool rereference = update_registry(header, registry, registry_file);

    // Updates the index
    if (reindex) {
        // ID changed, so we need to remove the old one from the index and insert a new one
        index_remove(index_header, old_id);
        index_add(index_header, registry->registry_content->id, get_registry_reference(header, registry->offset));
    } else if (rereference) {
        // ID is the same, but the offset changed, so we need to update the index reference
        index_update(index_header, registry->registry_content->id, get_registry_reference(header, registry->offset));
    }

    return true;
}

/**
 * Update the given registgries and their associated indexes
 * @param args command args
 */
void c_update_registry(CommandArgs* args) {
    ex_assert(args->primary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->secondary_file != NULL, EX_COMMAND_PARSE_ERROR);
    ex_assert(args->specific_data != NULL, EX_COMMAND_PARSE_ERROR);

    UpdateArgs* update_args = args->specific_data;

    // Open registry_file
    FILE* registry_file = fopen(args->primary_file, "rb+");
    if (registry_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Open index_file
    FILE* index_file = fopen(args->secondary_file, "rb+");
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Allocate and read header
    Header* header = build_header(args->registry_type);
    size_t first_registry_offset = read_header(header, registry_file);

    // Load index
    IndexHeader* index_header = new_index(args->registry_type);
    size_t read_bytes_index = read_index(index_header, index_file);

    // Check for read failure or bad status
    if (first_registry_offset == 0 || get_header_status(header) == STATUS_BAD || read_bytes_index == 0 || get_index_status(index_header) == STATUS_BAD) {
        puts(EX_FILE_ERROR);
        fclose(registry_file);
        fclose(index_file);
        destroy_header(header);
        destroy_index_header(index_header);
        return;
    }

    // Update registry header status
    set_header_status(header, STATUS_BAD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Update index header status
    set_index_status(index_header, STATUS_BAD);
    fseek(index_file, 0, SEEK_SET);
    write_index_header(index_header, index_file);

    // Allocate registry
    Registry* registry = build_registry(header);

    // Do each insertion in the given order
    for (uint32_t i = 0; i < update_args->n_updates; i++) {
        UpdateTarget current_update = update_args->update_targets[i];

        // Search for the registries to update //
        if (current_update.indexed_filter_args != NULL) {
            // Indexed cases //
            FilterArgs* filter_args = current_update.indexed_filter_args;

            ex_assert(strcmp(filter_args->key, ID_FIELD_NAME) == 0, EX_COMMAND_PARSE_ERROR);
            ex_assert(filter_args->next == NULL, EX_COMMAND_PARSE_ERROR);

            // Target id
            int32_t id = parse_int32_filter(filter_args);

            // Search for id on index
            IndexElement* index_match = index_query(index_header, id);

            // If id not found, skip
            if (index_match == NULL) {
                continue;
            }

            // Load target registry
            seek_registry(header, registry_file, index_match->reference);
            read_registry(registry, registry_file);

            // If registry is not present or filters don't match, skip
            if (is_registry_removed(registry) || !registry_filter_match(registry, current_update.unindexed_filter_args)) {
                continue;
            }

            // Execute the update
            execute_update(header, registry, &current_update, registry_file, index_header);
        } else {
            // Non-indexed cases //

            // Go to top of the registry for iteration
            go_to_offset(first_registry_offset, registry_file);
            size_t read_bytes_registry = first_registry_offset;
            // Max iteration offset
            size_t max_offset = get_max_offset(header);

            // Filters
            FilterArgs* filter_args = current_update.unindexed_filter_args;

            // Loop each registry until reaching the file limit
            while (read_bytes_registry < max_offset) {
                // Read the current registry
                read_bytes_registry += read_registry(registry, registry_file);

                // On read failure, removal or no filter match, skip
                if (registry == NULL || is_registry_removed(registry) || !registry_filter_match(registry, filter_args)) {
                    continue;
                }

                // Store current offset for later recovery
                size_t cur_offset = current_offset(registry_file);

                // Execute the update
                execute_update(header, registry, &current_update, registry_file, index_header);

                // Recover to iteration position
                go_to_offset(cur_offset, registry_file);
            }
        }
    }

    // Cleanup
    destroy_registry(registry);

    // Update registry header
    set_header_status(header, STATUS_GOOD);
    fseek(registry_file, 0, SEEK_SET);
    write_header(header, registry_file);

    // Reopen the index
    index_file = freopen(args->secondary_file, "wb", index_file);
    if (index_file == NULL) {
        puts(EX_FILE_ERROR);
        return;
    }

    // Write the updated index
    fseek(index_file, 0, SEEK_SET);
    write_index(index_header, index_file);

    // Update index status
    fseek(index_file, 0, SEEK_SET);
    set_index_status(index_header, STATUS_GOOD);
    write_index_header(index_header, index_file);

    // Cleanup
    destroy_header(header);
    destroy_index_header(index_header);
    fclose(registry_file);
    fclose(index_file);

    // Autocorrection stuff
    print_autocorrection_checksum(args->primary_file);
    print_autocorrection_checksum(args->secondary_file);
}

// Utils //

/**
 * Print a fixed length string
 * @param desc target string
 * @param n string length
 */
void print_fixed_len_str(char* desc, size_t n) {
    for (size_t i = 0; i < n; i++) {
        putchar(desc[i]);
    }
}

/**
 * Print a given type 1 registry into stdout
 *
 * @param header current file header
 * @param registry current registry
 */
void print_registry(Header* header, Registry* registry) {
    HeaderContent* header_content = header->header_content;
    RegistryContent* registry_content = registry->registry_content;
    // Marca
    print_column_description(header_content->desC6);
    if (registry_content->marca == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->marca);
    }

    // Modelo
    print_column_description(header_content->desC7);
    if (registry_content->modelo == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->modelo);
    }

    // Ano Fabricacao
    print_column_description(header_content->desC2);
    if (registry_content->ano == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry_content->ano);
    }

    // Cidade
    print_column_description(header_content->desC5);
    if (registry_content->cidade == NULL) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%s\n", registry_content->cidade);
    }

    // Qtt
    print_column_description(header_content->desC3);
    if (registry_content->qtt == -1) {
        puts(NULL_FIELD_REPR);
    } else {
        printf("%d\n", registry_content->qtt);
    }

    putchar('\n');
}

/**
 * Parse filter to int32
 * @param filter target filter
 * @return corresponding int32 value
 */
int32_t parse_int32_filter(FilterArgs* filter) {
    int32_t value = -1;

    // Filter parsing
    if (filter->parsed_value != NULL) {
        value = *((int32_t*) filter->parsed_value);
    } else {
        if (filter->value != NULL && filter->value[0] != '\0') {
            value = (int32_t) strtol(filter->value, NULL, 10);
        }
        filter->parsed_value = malloc(sizeof(value));
        *((int32_t*) filter->parsed_value) = value;
    }

    return value;
}

/**
 * Checks if a registry matches the given filter list
 * @param registry target registry
 * @param filters target filters
 * @return if registry matches the filters or not
 */
bool registry_filter_match(Registry* registry, FilterArgs* filters) {
    if (is_registry_removed(registry)) {
        return false;
    }

    if (filters == NULL) {
        return true;
    }

    RegistryContent* registry_content = registry->registry_content;
    FilterArgs* cur_filter = filters;

    while (cur_filter != NULL) {
        bool is_null = cur_filter->value == NULL || cur_filter->value[0] == '\0';
        if (strcmp(ID_FIELD_NAME, cur_filter->key) == 0) {// id
            int32_t id_filter = parse_int32_filter(cur_filter);
            if (id_filter != registry_content->id) {
                return false;
            }
        } else if (strcmp(ANO_FIELD_NAME, cur_filter->key) == 0) {// ano
            int32_t ano_filter = parse_int32_filter(cur_filter);
            if (ano_filter != registry_content->ano) {
                return false;
            }
        } else if (strcmp(QTT_FIELD_NAME, cur_filter->key) == 0) {// qtt
            int32_t qtt_filter = parse_int32_filter(cur_filter);
            if (qtt_filter != registry_content->qtt) {
                return false;
            }
        } else if (strcmp(SIGLA_FIELD_NAME, cur_filter->key) == 0) {// sigla
            // In case filter is NULL, check sigla for null value
            if (cur_filter->value == NULL) {
                if (registry_content->sigla[0] != '$') {
                    return false;
                }
            } else {
                // check if filter is smaller than or equal to the sigla
                size_t len = strlen(cur_filter->value);
                if (len > REGISTRY_SIGLA_SIZE) {
                    return false;
                }

                // compare char-by-char sigla and filter (if filter is smaller, ensure remaining bytes are NULL)
                for (size_t i = 0; i < REGISTRY_SIGLA_SIZE; i++) {
                    if (i >= len && registry_content->sigla[i] != FILLER_BYTE[0]) {
                        return false;
                    } else if (cur_filter->value[i] != registry_content->sigla[i]) {
                        return false;
                    }
                }
            }
        } else if (strcmp(CIDADE_FIELD_NAME, cur_filter->key) == 0) {// cidade
            // Check for null fields
            if (is_null || registry_content->cidade == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->cidade != NULL) || (registry_content->cidade == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->cidade) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MARCA_FIELD_NAME, cur_filter->key) == 0) {// marca
            // Check for null fields
            if (is_null || registry_content->marca == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->marca != NULL) || (registry_content->marca == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->marca) != 0) {// Compare non-null values directly
                return false;
            }
        } else if (strcmp(MODELO_FIELD_NAME, cur_filter->key) == 0) {// modelo
            // Check for null fields
            if (is_null || registry_content->modelo == NULL) {
                // Check for non-matching null fields
                if ((is_null && registry_content->modelo != NULL) || (registry_content->modelo == NULL && !is_null)) {
                    return false;
                }
            } else if (strcmp(cur_filter->value, registry_content->modelo) != 0) {// Compare non-null values directly
                return false;
            }
        }

        cur_filter = cur_filter->next;
    }

    return true;
}
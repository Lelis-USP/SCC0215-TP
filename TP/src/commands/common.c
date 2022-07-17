/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include "common.h"

#include <stdlib.h>

/**
 * Create command args struct
 * @param command target command
 * @return the struct ptr
 */
CommandArgs* new_command_args(enum Command command) {
    CommandArgs* args = malloc(sizeof(struct CommandArgs));

    args->command = command;
    args->primary_file = NULL;
    args->secondary_file = NULL;
    args->source = NULL;
    args->specific_data = NULL;

    return args;
}

void destroy_filter_args(FilterArgs* filter_args) {
    FilterArgs* cur = filter_args;
    while (cur != NULL) {
        FilterArgs* next = cur->next;
        free(cur->key);
        free(cur->value);
        free(cur->parsed_value);
        free(cur);
        cur = next;
    }
}

/**
 * Destroy command args struct
 * @param args target struct
 */
void destroy_command_args(CommandArgs* args) {
    if (args == NULL) return;

    free(args->primary_file);
    free(args->secondary_file);

    if (args->specific_data != NULL) {
        switch (args->command) {
            case DESERIALIZE_FILTER_AND_PRINT:
                destroy_filter_args((FilterArgs*) args->specific_data);
                break;

            case DESERIALIZE_SEARCH_RRN_AND_PRINT:
            case QUERY_REGISTRY_WITH_BTREE_INDEX:
                free(args->specific_data);
                break;

            case REMOVE_REGISTRY_WITH_LINEAR_INDEX:
            case REMOVE_REGISTRY_WITH_BTREE_INDEX:
                ;
                RemovalArgs* removal_args = args->specific_data;
                // Free filter chains
                for (uint32_t i = 0; i < removal_args->n_removals; i++) {
                    RemovalTarget removal_target = removal_args->removal_targets[i];
                    destroy_filter_args(removal_target.indexed_filter_args);
                    destroy_filter_args(removal_target.unindexed_filter_args);
                }
                free(removal_args->removal_targets);
                free(removal_args);
                break;

            case INSERT_REGISTRY_WITH_LINEAR_INDEX:
            case INSERT_REGISTRY_WITH_BTREE_INDEX:
                ;
                InsertionArgs* insertion_args = args->specific_data;

                for (uint32_t i = 0; i < insertion_args->n_insertions; i++) {
                    InsertionTarget insertion_element = insertion_args->insertion_targets[i];
                    free(insertion_element.cidade);
                    free(insertion_element.marca);
                    free(insertion_element.modelo);
                }

                free(insertion_args->insertion_targets);
                free(insertion_args);
                break;

            case UPDATE_REGISTRY_WITH_LINEAR_INDEX:
            case UPDATE_REGISTRY_WITH_BTREE_INDEX:
                ;
                UpdateArgs* update_args = args->specific_data;

                for (uint32_t i = 0; i < update_args->n_updates; i++) {
                    UpdateTarget update_target = update_args->update_targets[i];
                    destroy_filter_args(update_target.indexed_filter_args);
                    destroy_filter_args(update_target.unindexed_filter_args);
                    free(update_target.cidade);
                    free(update_target.marca);
                    free(update_target.modelo);
                }

                free(update_args->update_targets);
                free(update_args);
                break;

            default:
                break;
        }
    }

    free(args);
}

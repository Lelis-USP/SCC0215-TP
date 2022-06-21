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
    args->source_file = NULL;
    args->dest_file = NULL;
    args->source = NULL;
    args->specific_data = NULL;

    return args;
}

/**
 * Destroy command args struct
 * @param args target struct
 */
void destroy_command_args(CommandArgs* args) {
    if (args == NULL) return;

    free(args->source_file);
    free(args->dest_file);

    if (args->specific_data != NULL) {
        // RRN cleanup
        if (args->command == DESERIALIZE_SEARCH_RRN_AND_PRINT) {
            free(args->specific_data);
        }

        // Filter cleanup
        if (args->command == DESERIALIZE_FILTER_AND_PRINT) {
            FilterArgs* cur = (FilterArgs*) args->specific_data;
            while (cur != NULL) {
                FilterArgs* next = cur->next;
                free(cur->key);
                free(cur->value);
                free(cur->parsed_value);
                free(cur);
                cur = next;
            }
        }
    }

    free(args);
}

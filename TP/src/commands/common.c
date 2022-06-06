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
    args->sourceFile = NULL;
    args->destFile = NULL;
    args->source = NULL;
    args->specificData = NULL;

    return args;
}

/**
 * Destroy command args struct
 * @param args target struct
 */
void destroy_command_args(CommandArgs* args) {
    if (args == NULL) return;

    free(args->sourceFile);
    free(args->destFile);

    if (args->specificData != NULL) {
        // RRN cleanup
        if (args->command == DESERIALIZE_SEARCH_RRN_AND_PRINT) {
            free(args->specificData);
        }

        // Filter cleanup
        if (args->command == DESERIALIZE_FILTER_AND_PRINT) {
            FilterArgs* cur = (FilterArgs*) args->specificData;
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

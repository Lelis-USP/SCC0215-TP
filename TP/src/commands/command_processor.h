/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "common.h"

/**
 * Initiate the command processor
 * @param data_in the data input stream (usualy stdin)
 */
void execute(FILE* data_in);

/**
 * Read and parse command information
 * @param source the source of the command information
 * @return the parsed command
 */
CommandArgs* read_command(FILE* source);

/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "common.h"

void execute(FILE* data_in);
CommandArgs* read_command(FILE* source);
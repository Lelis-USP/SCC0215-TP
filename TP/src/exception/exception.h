/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdio.h>

#define EX_EXIT_CODE 0
#define EX_FILE_OUT stdout

// Common error messages
static const char EX_GENERIC_ERROR[] = "Erro desconhecido.";
static const char EX_REGISTRY_NOT_FOUND[] = "Registro inexistente.";
static const char EX_FILE_ERROR[] = "Falha no processamento do arquivo.";
static const char EX_COMMAND_PARSE_ERROR[] = "Falha ao processar comando.";

// Raising errors
void ex_raise(const char message[]);
void ex_assert(int cond, const char message[]);


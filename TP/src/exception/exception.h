/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

#define EX_FILE_OUT stdout

// Common error messages
static const char EX_GENERIC_ERROR[] = "Erro desconhecido.";
static const char EX_REGISTRY_NOT_FOUND[] = "Registro inexistente.";
static const char EX_FILE_ERROR[] = "Falha no processamento do arquivo.";
static const char EX_COMMAND_PARSE_ERROR[] = "Falha ao processar comando.";
static const char EX_MEMORY_ERROR[] = "Falha ao alocar memória.";
static const char EX_CORRUPTED_REGISTRY[] = "Registro corrompido.";

// Raising errors

// Only activate assertions on debug
// DEBUG //
#ifdef DEBUG
/**
 * Simple assertion implementation that checks for a condition, in case it fails log the error location and message
 *
 * Should only be accessed through ex_assert macro
 */
static inline void ex_assert_dbg_func(int cond, const char* message, int line, const char* file, const char* func) {
    if (!cond) {
        fprintf(EX_FILE_OUT, "%s (%s:%d) => %s\n", func, file, line, message);
        exit(1);
    }
}
/**
 * Assertion macro for DEBUG environment
 *
 * Simple assertion implementation that checks for a condition, in case it fails log the error location and message
 */
#define ex_assert(cond, message) ex_assert(cond, message, __LINE__, __FILE__, __func__);

// PROD //
#else
/**
 * Disable assertions out of debug
 */
#define ex_assert(cond, message)
#endif

/**
 * Alias raise to an assert(false)
 */
#define ex_raise(message) ex_assert(0, message)
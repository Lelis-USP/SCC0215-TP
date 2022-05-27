/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "struct/t1_struct.h"
#include "struct/t2_struct.h"

static const T1Header DEFAULT_T1_HEADER = {
        STATUS_GOOD,
        -1,
        "LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
        "CODIGO IDENTIFICADOR: ",
        "ANO DE FABRICACAO: ",
        "QUANTIDADE DE VEICULOS: ",
        "ESTADO: ",
        "0",
        "NOME DA CIDADE: ",
        "1",
        "MARCA DO VEICULO: ",
        "2",
        "MODELO DO VEICULO: ",
        0,
        0};

static const T2Header DEFAULT_T2_HEADER = {
        STATUS_GOOD,
        -1,
        "LISTAGEM DA FROTA DOS VEICULOS NO BRASIL",
        "CODIGO IDENTIFICADOR: ",
        "ANO DE FABRICACAO: ",
        "QUANTIDADE DE VEICULOS: ",
        "ESTADO: ",
        "0",
        "NOME DA CIDADE: ",
        "1",
        "MARCA DO VEICULO: ",
        "2",
        "MODELO DO VEICULO: ",
        T2_HEADER_SIZE,
        0};

static const char EMPTY_REGISTRY_MSG[] = "Registro inexistente.";
static const char FILE_ERROR_MSG[] = "Falha no processamento do arquivo.";
static const char NULL_FIELD_REPR[] = "NAO PREENCHIDO";

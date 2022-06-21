/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#pragma once

#include "../struct/registry.h"
#include "../struct/registry_content.h"
#include "../struct/t1_registry.h"
#include "../struct/t2_registry.h"

static const HeaderContent DEFAULT_HEADER_CONTENT = {
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
};

static const T1HeaderMetadata DEFAULT_T1_HEADER_METADATA = {
        STATUS_GOOD,
        -1,
        0,
        0
};

static const T2HeaderMetadata DEFAULT_T2_HEADER_METADATA = {
        STATUS_GOOD,
        -1,
        (int64_t) T2_HEADER_SIZE,
        0
};


static const char NULL_FIELD_REPR[] = "NAO PREENCHIDO";

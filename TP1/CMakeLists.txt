cmake_minimum_required(VERSION 3.20)
project(ARQUIVOS C)

set(CMAKE_C_STANDARD 11)

include_directories(src src/struct, src/const)
set(SOURCES src/struct/t1_struct.h src/struct/t1_struct.c src/const/const.h src/utils/provided_functions.h src/utils/provided_functions.c src/commands/command_processor.h src/utils/csv_parser.h src/utils/csv_parser.c src/commands/command_processor.c src/struct/t2_struct.h src/struct/common.h src/struct/common.c src/utils/registry_builder.h src/utils/registry_builder.c)

add_executable(ARQUIVOS main.c ${SOURCES})


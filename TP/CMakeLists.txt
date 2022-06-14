cmake_minimum_required(VERSION 3.20)
project(ARQUIVOS C)

set(CMAKE_C_STANDARD 99)

include_directories(src src/struct, src/const)
set(SOURCES src/struct/t1_struct.h src/struct/t1_struct.c src/const/const.h src/utils/provided_functions.h src/utils/provided_functions.c src/commands/command_processor.h src/utils/csv_parser.h src/utils/csv_parser.c src/commands/command_processor.c src/struct/t2_struct.h src/struct/common.h src/struct/common.c src/utils/registry_builder.h src/utils/registry_builder.c src/struct/t2_struct.c src/commands/common.h src/commands/common.c src/commands/commands.c src/commands/commands.h src/exception/exception.c src/exception/exception.h src/struct/registry_content.c src/struct/registry_content.h src/struct/registry.c src/struct/registry.h src/struct/t1_registry.c src/struct/t1_registry.h)

add_executable(ARQUIVOS src/main.c ${SOURCES})


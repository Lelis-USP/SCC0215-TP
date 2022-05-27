cmake_minimum_required(VERSION 3.20)
project(ARQUIVOS C)

set(CMAKE_C_STANDARD 11)

include_directories(src src/struct, src/const)
set(SOURCES src/struct/t1_struct.h src/struct/t1_struct.c src/const/const.h src/utils/provided_functions.h src/utils/provided_functions.c src/commands/command_processor.h src/utils/csv_parser.h src/utils/csv_parser.c src/commands/command_processor.c src/struct/t2_struct.h src/struct/common.h src/struct/common.c src/utils/registry_builder.h src/utils/registry_builder.c src/struct/t2_struct.c cmake-build-debug/bruh/common.c src/commands/common.h cmake-build-debug/bruh/parse_and_serialize.c cmake-build-debug/bruh/parse_and_serialize.h cmake-build-debug/bruh/deserialize_and_print.c cmake-build-debug/bruh/deserialize_and_print.h cmake-build-debug/bruh/deserialize_filter_and_print.c cmake-build-debug/bruh/deserialize_filter_and_print.h cmake-build-debug/bruh/deserialize_direct_access_rrn_print.c cmake-build-debug/bruh/deserialize_direct_access_rrn_print.h src/commands/common.c src/commands/commands.c src/commands/commands.h)

add_executable(ARQUIVOS src/main.c ${SOURCES})


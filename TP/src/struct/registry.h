//#pragma once
//
//#include "common.h"
//#include "base_registry.h"
//#include "t1_struct.h"
//#include "t2_struct.h"
//
//typedef enum RegistryType {
//    UNKNOWN = 0,
//    FIX_LEN = 1,
//    VAR_LEN = 2
//} RegistryType;
//
//typedef struct GenericHeader {
//    void* header;
//    RegistryType registry_type;
//} GenericHeader;
//
//typedef struct GenericRegistry {
//    void* registry;
//    void* header;
//    RegistryType registry_type;
//} GenericRegistry;
//
//// Memory management
//void setup_generic_header(GenericHeader* generic_header);
//void setup_generic_registry(GenericRegistry* generic_registry);
//
//GenericHeader* new_generic_header();
//GenericRegistry* new_generic_registry();
//
//void destroy_generic_header(GenericHeader* generic_header);
//void destroy_generic_registry(GenericRegistry* generic_registry);
//
//GenericHeader* build_generic_header(RegistryType registry_type);
//GenericRegistry* build_generic_registry(GenericHeader* generic_header);
//GenericRegistry* build_generic_registry_from_type(RegistryType* registry_type);
//
//// File I/O
//size_t write_generic_header(GenericHeader* generic_header, FILE* dest);
//size_t read_generic_header(GenericHeader* generic_header, FILE* src);
//
//size_t write_registry_header(GenericRegistry* generic_registry, FILE* dest);
//size_t read_registry_header(GenericRegistry* generic_registry, FILE* src);

#include "t1_registry.h"

#include <stdlib.h>

#include "../exception/exception.h"

//size_t t1_write_header(Header* header, FILE* dest);
//size_t t1_read_header(Header* header, FILE* src);
//
//size_t t1_write_registry(Registry* registry, FILE* dest);
//size_t t1_read_registry(Registry* registry, FILE* src);

void t1_setup_header_metadata(T1HeaderMetadata* header_metadata) {
    header_metadata->status = STATUS_BAD;
    header_metadata->topo = -1;
    header_metadata->proxRRN = 0;
    header_metadata->nroRegRem = 0;
}

void t1_setup_registry_metadata(T1RegistryMetadata* registry_metadata) {
    registry_metadata->removido = NOT_REMOVED;
    registry_metadata->prox = -1;
}

T1HeaderMetadata* t1_new_header_metadata() {
    T1HeaderMetadata* header_metadata = malloc(sizeof (struct T1HeaderMetadata));
    t1_setup_header_metadata(header_metadata);
    return header_metadata;
}

T1RegistryMetadata* t1_new_registry_metadata() {
    T1RegistryMetadata* registry_metadata = malloc(sizeof (struct T1RegistryMetadata));
    t1_setup_registry_metadata(registry_metadata);
    return registry_metadata;
}

void t1_destroy_header_metadata(T1HeaderMetadata* header_metadata) {
    if (header_metadata == NULL) {
        return;
    }

    free(header_metadata);
}
void t1_destroy_registry_metadata(T1RegistryMetadata* registry_metadata) {
    if (registry_metadata == NULL) {
        return;
    }

    free(registry_metadata);
}


#include <assert.h>

#include "t1_struct.h"

// TO-DO: should I do custom exception handling instead of asserts?

/**
 * Writes the header into a file pointer.
 *
 * @param dest target file (must be writable and in binary mode)
 * @param header header to be serialized into the file
 */
void t1_write_header(T1Header* header, FILE* dest) {
  // TO-DO: Should I use a local memory buffer here to do a single write call?
  // TO-DO: Should I check for fwrite failures (return < 1)?

  // Basic input validation
  assert(dest != NULL);
  assert(header != NULL);

  // Write data in order to the file
  fwrite(&header->status, member_size(T1Header, status), 1, dest);
  fwrite(&header->topo, member_size(T1Header, topo), 1, dest);
  fwrite(header->descricao, member_size(T1Header, descricao), 1, dest);
  fwrite(header->desC1, member_size(T1Header, desC1), 1, dest);
  fwrite(header->desC2, member_size(T1Header, desC2), 1, dest);
  fwrite(header->desC3, member_size(T1Header, desC3), 1, dest);
  fwrite(header->desC4, member_size(T1Header, desC4), 1, dest);
  fwrite(header->codC5, member_size(T1Header, codC5), 1, dest);
  fwrite(header->desC5, member_size(T1Header, desC5), 1, dest);
  fwrite(header->codC6, member_size(T1Header, codC6), 1, dest);
  fwrite(header->desC6, member_size(T1Header, desC6), 1, dest);
  fwrite(header->codC7, member_size(T1Header, codC7), 1, dest);
  fwrite(header->desC7, member_size(T1Header, desC7), 1, dest);
  fwrite(&header->proxRRN, member_size(T1Header, proxRRN), 1, dest);
  fwrite(&header->nroRegRem, member_size(T1Header, nroRegRem), 1, dest);
}

/**
 * Reads the header from a file into the given header struct
 *
 * @param src source file (must be readable and in binary mode)
 * @param header target header
 */
void t1_read_header(T1Header* header, FILE* src) {
  // Basic input validation
  assert(src != NULL);
  assert(header != NULL);

  // Read data into header
  fread(&header->status, member_size(T1Header, status), 1, src);
  fread(&header->topo, member_size(T1Header, topo), 1, src);
  fread(header->descricao, member_size(T1Header, descricao), 1, src);
  fread(header->desC1, member_size(T1Header, desC1), 1, src);
  fread(header->desC2, member_size(T1Header, desC2), 1, src);
  fread(header->desC3, member_size(T1Header, desC3), 1, src);
  fread(header->desC4, member_size(T1Header, desC4), 1, src);
  fread(header->codC5, member_size(T1Header, codC5), 1, src);
  fread(header->desC5, member_size(T1Header, desC5), 1, src);
  fread(header->codC6, member_size(T1Header, codC6), 1, src);
  fread(header->desC6, member_size(T1Header, desC6), 1, src);
  fread(header->codC7, member_size(T1Header, codC7), 1, src);
  fread(header->desC7, member_size(T1Header, desC7), 1, src);
  fread(&header->proxRRN, member_size(T1Header, proxRRN), 1, src);
  fread(&header->nroRegRem, member_size(T1Header, nroRegRem), 1, src);
}

/**
 * Writes the a registry into the given file pointer.
 *
 * @param dest destination file pointer
 * @param registry the registry to be serializes into file
 */
void t1_write_registry(FILE* dest, T1Registry* registry) {
  // Basic input validation
  assert(dest != NULL);
  assert(registry != NULL);

  size_t len_cidade = sizeof(char) * registry->tamCidade;
  size_t len_marca = sizeof(char) * registry->tamCidade;
  size_t len_modelo = sizeof(char) * registry->tamCidade;

  // Write data to file
  fwrite(&registry->removido, member_size(T1Registry, removido), 1, dest);
  fwrite(&registry->prox, member_size(T1Registry, prox), 1, dest);
  fwrite(&registry->id, member_size(T1Registry, id), 1, dest);
  fwrite(&registry->ano, member_size(T1Registry, ano), 1, dest);
  fwrite(&registry->qtt, member_size(T1Registry, qtt), 1, dest);

  fwrite(registry->sigla, sizeof(char), member_size(T1Registry, sigla), dest);

  write_var_len_str(registry->cidade, registry->tamCidade, registry->codC5, dest);
  write_var_len_str(registry->marca, registry->tamMarca, registry->codC6, dest);
  write_var_len_str(registry->modelo, registry->tamModelo, registry->codC7, dest);

  size_t filled_bytes = T1_STATIC_REGISTRY_SIZE + len_modelo + len_marca + len_cidade;
  size_t missing_bytes = T1_TOTAL_REGISTRY_SIZE - filled_bytes;

  // Fill remaining bytes for run.codes autocorrect
  fill_bytes(missing_bytes, dest);
}
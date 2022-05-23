#include <stdio.h>
#include <assert.h>

#include "struct/t1_struct.h"
#include "const/const.h"
#include "utils/csv_parser.h"

int main() {
  printf("Hello, World!\n");
  FILE* arquivoooo = fopen("oi.bin", "wb");
  T1Header header = DEFAULT_HEADER;
  header.proxRRN = 26;
    t1_write_header(arquivoooo, &header);
//  t1_write_header(arquivoooo, &header);
  fclose(arquivoooo);

  FILE* arquiveeee = fopen("oi.bin", "rb");
  T1Header novoHeader;
    t1_read_header(arquiveeee, &novoHeader);
  assert(novoHeader.proxRRN == 26);
  fclose(arquiveeee);


  printf("Val: %lu\n", member_size(T1Header, proxRRN));

  return 0;
}

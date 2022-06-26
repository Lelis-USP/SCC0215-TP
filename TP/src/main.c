/*
*  Daniel Henrique Lelis de Almeida - 12543822
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "commands/command_processor.h"

int main() {
    srand(time(NULL));
    execute(stdin);
    return 0;
}

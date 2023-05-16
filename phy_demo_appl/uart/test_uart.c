#include "uart_venice.h"
#include <stdio.h>
#include <stdlib.h>

void main() 
{
    int fp = uart_venice_init("/dev/ttyUART0");
    char* output = uart_excute(fp, "help");
    printf("%s", output);
    free(output);
}
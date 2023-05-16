#include "uart_venice.h"
#include <stdio.h>
#include <stdlib.h>

void main() 
{
    int fp = uart_venice_init("/dev/ttyUSB0");
    char* output = malloc(1024*sizeof(char));
    uart_excute(fp, "help", NULL);
    uart_excute(fp, "poke 0x83100400 0x34 0x0001", NULL);
    uart_excute(fp, "poke 0x83100400 0x3c 0xf224", NULL);
    uart_excute(fp, "poke 0x83100400 0x34 0x4001", NULL);
    uart_excute(fp, "peek 0x83100400 0x38", output);
    printf("%s\n", output);
    uart_excute(fp, "poke 0x83100400 0x34 0x4001", NULL);
    uart_excute(fp, "peek 0x83100400 0x38", output);
    printf("%s\n", output);
    free(output);
}
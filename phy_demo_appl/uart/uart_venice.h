#ifndef UART_VENICE_H
#define UART_VENICE_H


int uart_venice_init(const char* device);
void uart_excute(int fp, const char * command, char* output);

#endif

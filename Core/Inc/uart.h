#ifndef __UART_H
#define __UART_H

#include <stdint.h>

void UART_Init(void);
void UART_Process(void);
void UART_SendString(char *str);
void UART_HandleRxData(uint8_t data);

#endif


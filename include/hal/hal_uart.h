#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

void hal_uart_init(uint32_t baudrate);
void hal_uart_puts(const char *str);

#endif /* HAL_UART_H */

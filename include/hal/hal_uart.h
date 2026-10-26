#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include "hal/hal_types.h"

hal_status_t hal_uart_init(uint32_t baudrate);
hal_status_t hal_uart_puts(const char *str);

#endif /* HAL_UART_H */

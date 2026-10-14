#include <stdio.h>
#include <stdint.h>
#include "hal/hal_uart.h"

void hal_uart_init(uint32_t baudrate) {
    (void)baudrate;
    /* En PC la salida serial es simplemente la terminal (stdout) */
}

void hal_uart_puts(const char *str) {
    if (str != NULL) {
        printf("%s", str);
    }
}

#include "config/system_constants.h"
#include "hal/hal_uart.h"

int main(void) {
    hal_uart_init(TCS_UART_BAUDRATE);
    hal_uart_puts("Hello, World!\n");

    return 0;
}

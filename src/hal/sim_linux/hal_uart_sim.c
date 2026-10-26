#include <stdio.h>
#include <stdint.h>
#include "hal/hal_uart.h"

/* Estado interno privado del modulo UART simulado */
static volatile uint8_t s_uart_initialized = 0u;    // Indica si la UART ha sido inicializada (1) o no (0)
static uint32_t s_uart_baudrate = 0UL;              // Velocidad de transmision configurada para la UART simulada (en bps)

/** Función para inicializar la UART con una velocidad de transmisión */
hal_status_t hal_uart_init(uint32_t baudrate) {
    if (baudrate == 0UL) {
        return HAL_ERR_PARAM;
    }

    s_uart_baudrate    = baudrate; // Asignar la velocidad de transmisión
    s_uart_initialized = 1u;       // Marcar la UART como inicializada

    printf("[HAL-UART-SIM] Consola serial inicializada a %lu bps\n",
           (unsigned long)s_uart_baudrate);

    return HAL_OK;
}

/** Función para enviar una cadena de caractéres por UART */
hal_status_t hal_uart_puts(const char *str) {
    if (str == NULL) { // Caso nulo: cadena de caractéres no válida
        return HAL_ERR_PARAM;
    }
    if (s_uart_initialized == 0u) { // Verificar si la UART ha sido inicializada
        return HAL_ERR_STATE;
    }

    if (fputs(str, stdout) == EOF) { // Intentar escribir la cadena en la salida estándar (simulación de UART)
        return HAL_ERROR;
    }

    return HAL_OK;
}

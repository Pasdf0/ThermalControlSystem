#include <stdint.h>
#include "config/system_constants.h"
#include "hal/hal_gpio.h"
#include "hal/hal_uart.h"

int main(void) {
    // Inicializar la UART con la velocidad de transmisión definida en las constantes del sistema
    hal_status_t status = hal_uart_init(TCS_UART_BAUDRATE);
    if (status != HAL_OK) {
        return (int)status;
    }

    // Inicializar el pin GPIO para el LED de estado como salida
    status = hal_gpio_init(TCS_STATUS_LED_PIN, HAL_GPIO_MODE_OUTPUT);
    if (status != HAL_OK) {
        (void)hal_uart_puts("[TCS-ERR] Fallo en inicializacion de GPIO.\n");
        return (int)status;
    }

    // Enviar mensaje de inicio por UART
    status = hal_uart_puts("[TCS-Main] Hello World - Sistema de Control Termico inicializado.\n");
    if (status != HAL_OK) {
        return (int)status;
    }

    // Bucle acotado con cota superior constante
    // Alternar el estado del LED de estado un número fijo de veces
    for (uint32_t cycle = 0u; cycle < TCS_EV1_BLINK_CYCLES; cycle++) {
        status = hal_gpio_write(TCS_STATUS_LED_PIN, HAL_GPIO_LEVEL_HIGH);
        if (status != HAL_OK) {
            return (int)status;
        }

        status = hal_gpio_toggle(TCS_STATUS_LED_PIN);
        if (status != HAL_OK) {
            return (int)status;
        }
    }

    status = hal_uart_puts("[TCS-Main] Prueba funcional basica finalizada con estado OK.\n");
    return (int)status;
}

#ifdef ESP_PLATFORM
void app_main(void) {
    (void)main();
}
#endif

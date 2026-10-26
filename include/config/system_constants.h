#ifndef SYSTEM_CONSTANTS_H
#define SYSTEM_CONSTANTS_H

#include <stdint.h>

#define TCS_UART_BAUDRATE     115200UL // Velocidad serial UART (32-bit unsigned long)
#define TCS_STATUS_LED_PIN    2u       // GPIO2: LED integrado en placa ESP32 (Temporal)
#define TCS_GPIO_MAX_PINS     32u      // Ancho de registro de puerto GPIO (32 bits)

#define TCS_EV1_BLINK_CYCLES  3u       // Cota superior fija para bucle de prueba

#endif /* SYSTEM_CONSTANTS_H */

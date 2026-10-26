#include <stdio.h>
#include <stdint.h>
#include "config/system_constants.h"
#include "hal/hal_gpio.h"

// Arreglos privados para pins inicializados, modos y niveles de pin GPIO (simulación)
static volatile uint8_t          s_pin_initialized[TCS_GPIO_MAX_PINS];  
static volatile hal_gpio_mode_t  s_pin_mode[TCS_GPIO_MAX_PINS];          
static volatile hal_gpio_level_t s_pin_level[TCS_GPIO_MAX_PINS];         

// Funcion inline para verificar que el numero de pin este dentro del arreglo
static inline uint8_t gpio_is_valid_pin(uint8_t pin) {
    return (pin < TCS_GPIO_MAX_PINS) ? 1u : 0u;
}

/* Función para inicializar un pin GPIO con un modo */
hal_status_t hal_gpio_init(uint8_t pin, hal_gpio_mode_t mode) {
    if (gpio_is_valid_pin(pin) == 0u) {
        return HAL_ERR_PARAM;
    }
    if ((mode != HAL_GPIO_MODE_INPUT) && (mode != HAL_GPIO_MODE_OUTPUT)) {
        return HAL_ERR_PARAM;
    }

    s_pin_mode[pin]        = mode;                  // Guardar modo de pin
    s_pin_level[pin]       = HAL_GPIO_LEVEL_LOW;    // Estado inicial apagado
    s_pin_initialized[pin] = 1u;                    // Marcar pin como inicializado

    printf("[HAL-GPIO-SIM] Pin %u inicializado como %s (Estado inicial=%s)\n",
           (unsigned int)pin,
           (mode == HAL_GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT",
           (s_pin_level[pin] == HAL_GPIO_LEVEL_HIGH) ? "HIGH" : "LOW");

    return HAL_OK;
}

/* Función para escribir un nivel en un pin GPIO */
hal_status_t hal_gpio_write(uint8_t pin, hal_gpio_level_t level) {
    if (gpio_is_valid_pin(pin) == 0u) { // Verificar validez del pin
        return HAL_ERR_PARAM;
    }
    if ((s_pin_initialized[pin] == 0u) || (s_pin_mode[pin] != HAL_GPIO_MODE_OUTPUT)) { // Verificar que el pin este inicializado y en modo OUTPUT
        return HAL_ERR_STATE;
    }
    if ((level != HAL_GPIO_LEVEL_LOW) && (level != HAL_GPIO_LEVEL_HIGH)) { // Verificar que el nivel sea valido
        return HAL_ERR_PARAM;
    }

    s_pin_level[pin] = level;

    printf("[HAL-GPIO-SIM] Pin %u -> Nivel %u (%s)\n",  // Mostrar el nivel del pin
           (unsigned int)pin,
           (unsigned int)s_pin_level[pin],
           (level == HAL_GPIO_LEVEL_HIGH) ? "HIGH" : "LOW");

    return HAL_OK;
}

/* Función para alternar el nivel de un pin GPIO */
hal_status_t hal_gpio_toggle(uint8_t pin) {
    if (gpio_is_valid_pin(pin) == 0u) { // Verificar validez del pin
        return HAL_ERR_PARAM;
    }
    if ((s_pin_initialized[pin] == 0u) || (s_pin_mode[pin] != HAL_GPIO_MODE_OUTPUT)) { // Verificar que el pin este inicializado y en modo OUTPUT
        return HAL_ERR_STATE;
    }

    if (s_pin_level[pin] == HAL_GPIO_LEVEL_LOW) { // Cambiar el nivel del pin al opuesto
        s_pin_level[pin] = HAL_GPIO_LEVEL_HIGH;
    } else {
        s_pin_level[pin] = HAL_GPIO_LEVEL_LOW;
    }

    printf("[HAL-GPIO-SIM] Pin %u TOGGLE -> Nivel %u (%s)\n", // Mostrar el nuevo nivel del pin
           (unsigned int)pin,
           (unsigned int)s_pin_level[pin],
           (s_pin_level[pin] == HAL_GPIO_LEVEL_HIGH) ? "HIGH" : "LOW");

    return HAL_OK;
}

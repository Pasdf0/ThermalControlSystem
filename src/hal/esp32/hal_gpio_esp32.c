#include <stdint.h>
#include "config/system_constants.h"
#include "hal/hal_gpio.h"
#include "driver/gpio.h"

// Arreglos privados para pins inicializados, modos y niveles de pin GPIO (ESP32)
static volatile uint8_t          s_pin_initialized[TCS_GPIO_MAX_PINS];
static volatile hal_gpio_mode_t  s_pin_mode[TCS_GPIO_MAX_PINS];
static volatile hal_gpio_level_t s_pin_level[TCS_GPIO_MAX_PINS];

/* Verifica que el pin no pertenezca al bus SPI Flash interno del ESP32 (GPIO 6-11) */
static inline uint8_t esp32_is_safe_pin(uint8_t pin) {
    if ((pin >= 6u) && (pin <= 11u)) {
        return 0u;
    }
    return (pin < TCS_GPIO_MAX_PINS) ? 1u : 0u;
}

/* Función para inicializar un pin GPIO con un modo en ESP32 */
hal_status_t hal_gpio_init(uint8_t pin, hal_gpio_mode_t mode) {
    if (esp32_is_safe_pin(pin) == 0u) { // Verificar validez del pin en ESP32
        return HAL_ERR_PARAM;
    }
    if ((mode != HAL_GPIO_MODE_INPUT) && (mode != HAL_GPIO_MODE_OUTPUT)) { // Verificar validez del modo
        return HAL_ERR_PARAM;
    }

    const gpio_num_t gpio_pin = (gpio_num_t)pin; // Conversión a tipo gpio_num_t para funciones del ESP32
    if ((mode == HAL_GPIO_MODE_OUTPUT) && (!GPIO_IS_VALID_OUTPUT_GPIO(gpio_pin))) { // Verificar si el pin soporta modo OUTPUT
        return HAL_ERR_PARAM;
    }

    if (gpio_reset_pin(gpio_pin) != ESP_OK) { // Reiniciar configuración previa del pin
        return HAL_ERROR;
    }

    const gpio_mode_t idf_mode = (mode == HAL_GPIO_MODE_OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    if (gpio_set_direction(gpio_pin, idf_mode) != ESP_OK) { // Configurar dirección en hardware ESP32
        return HAL_ERROR;
    }

    s_pin_mode[pin]        = mode;                  // Guardar modo de pin
    s_pin_level[pin]       = HAL_GPIO_LEVEL_LOW;    // Estado inicial apagado
    s_pin_initialized[pin] = 1u;                    // Marcar pin como inicializado

    if (mode == HAL_GPIO_MODE_OUTPUT) {
        if (gpio_set_level(gpio_pin, 0u) != ESP_OK) { // Establecer nivel inicial bajo en hardware
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/* Función para escribir un nivel en un pin GPIO en ESP32 */
hal_status_t hal_gpio_write(uint8_t pin, hal_gpio_level_t level) {
    if (esp32_is_safe_pin(pin) == 0u) { // Verificar validez del pin
        return HAL_ERR_PARAM;
    }
    if ((s_pin_initialized[pin] == 0u) || (s_pin_mode[pin] != HAL_GPIO_MODE_OUTPUT)) { // Verificar que el pin este inicializado y en modo OUTPUT
        return HAL_ERR_STATE;
    }
    if ((level != HAL_GPIO_LEVEL_LOW) && (level != HAL_GPIO_LEVEL_HIGH)) { // Verificar que el nivel sea valido
        return HAL_ERR_PARAM;
    }

    const uint32_t out_val = (level == HAL_GPIO_LEVEL_HIGH) ? 1u : 0u;
    if (gpio_set_level((gpio_num_t)pin, out_val) != ESP_OK) { // Escribir nivel físico en el GPIO del ESP32
        return HAL_ERROR;
    }

    s_pin_level[pin] = level; // Guardar nivel actual del pin

    return HAL_OK;
}

/* Función para alternar el nivel de un pin GPIO en ESP32 */
hal_status_t hal_gpio_toggle(uint8_t pin) {
    if (esp32_is_safe_pin(pin) == 0u) { // Verificar validez del pin
        return HAL_ERR_PARAM;
    }
    if ((s_pin_initialized[pin] == 0u) || (s_pin_mode[pin] != HAL_GPIO_MODE_OUTPUT)) { // Verificar que el pin este inicializado y en modo OUTPUT
        return HAL_ERR_STATE;
    }

    const hal_gpio_level_t next_level = (s_pin_level[pin] == HAL_GPIO_LEVEL_LOW) ? // Cambiar el nivel del pin al opuesto
                                         HAL_GPIO_LEVEL_HIGH : HAL_GPIO_LEVEL_LOW;
    const uint32_t out_val = (next_level == HAL_GPIO_LEVEL_HIGH) ? 1u : 0u;

    if (gpio_set_level((gpio_num_t)pin, out_val) != ESP_OK) { // Cambiar nivel físico en el GPIO del ESP32
        return HAL_ERROR;
    }

    s_pin_level[pin] = next_level; // Guardar nuevo nivel del pin

    return HAL_OK;
}

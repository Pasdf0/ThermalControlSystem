#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#include <stdint.h>

typedef enum {
    HAL_OK          =  0,
    HAL_ERROR       = -1,
    HAL_ERR_PARAM   = -2,
    HAL_ERR_TIMEOUT = -3,
    HAL_ERR_STATE   = -4
} hal_status_t;

typedef enum {
    HAL_GPIO_MODE_INPUT  = 0u,  // Modo de pin GPIO como entrada (Input)
    HAL_GPIO_MODE_OUTPUT = 1u   // Modo de pin GPIO como salida (Output)
} hal_gpio_mode_t;

typedef enum {
    HAL_GPIO_LEVEL_LOW  = 0u,   // Estado apagado para pin GPIO
    HAL_GPIO_LEVEL_HIGH = 1u    // Estado encendido para pin GPIO
} hal_gpio_level_t;

#endif /* HAL_TYPES_H */

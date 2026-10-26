#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include "hal/hal_types.h"

hal_status_t hal_gpio_init(uint8_t pin, hal_gpio_mode_t mode);
hal_status_t hal_gpio_write(uint8_t pin, hal_gpio_level_t level);
hal_status_t hal_gpio_toggle(uint8_t pin);

#endif /* HAL_GPIO_H */

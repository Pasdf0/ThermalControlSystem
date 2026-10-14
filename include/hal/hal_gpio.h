#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

void hal_gpio_init(uint8_t pin, uint8_t mode);
void hal_gpio_write(uint8_t pin, uint8_t level);

#endif /* HAL_GPIO_H */

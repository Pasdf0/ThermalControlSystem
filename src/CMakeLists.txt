idf_component_register(
    SRCS
        "main.c"
        "hal/esp32/hal_uart_esp32.c"
        "hal/esp32/hal_gpio_esp32.c"
    INCLUDE_DIRS
        "../include"
    REQUIRES
        driver
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall -Wextra -Werror
    -Wshadow -Wconversion -Wdouble-promotion
)

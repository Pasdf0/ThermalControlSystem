cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{IDF_PATH} OR DEFINED IDF_TARGET OR ESP_PLATFORM)
    # ============================================================
    # Modo 1: Compilacion Cruzada con ESP-IDF (Target: ESP32)
    # ============================================================
    set(EXTRA_COMPONENT_DIRS src)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(tcs_firmware C)
else()
    # ============================================================
    # Modo 2: Simulacion Nativa en PC (Target: sim_linux / SIL)
    # ============================================================
    project(sim_tcs_firmware VERSION 0.1.0 LANGUAGES C)

    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)

    option(ENABLE_SANITIZERS "Habilitar AddressSanitizer y UndefinedBehaviorSanitizer" OFF)

    add_executable(sim_tcs_firmware
        src/main.c
        src/hal/sim_linux/hal_uart_sim.c
        src/hal/sim_linux/hal_gpio_sim.c
    )

    target_include_directories(sim_tcs_firmware PRIVATE include)

    target_compile_options(sim_tcs_firmware PRIVATE
        -Wall -Wextra -Werror -Wpedantic
        -Wshadow -Wconversion -Wdouble-promotion
        -Og -g3
    )

    if(ENABLE_SANITIZERS)
        target_compile_options(sim_tcs_firmware PRIVATE -fsanitize=address,undefined)
        target_link_options(sim_tcs_firmware PRIVATE -fsanitize=address,undefined)
    endif()
endif()

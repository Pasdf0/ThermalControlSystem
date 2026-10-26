# ============================================================
#  TCS - Sistema de Control Térmico y Modos de Vuelo
#  Departamento de Ingeniería Informática — USACH
# ------------------------------------------------------------
#  make           compila la simulación del firmware (SIL)
#  make run       ejecuta la simulación
#  make warnings  verifica compilación con flags estrictos
#  make sanitize  compila y ejecuta bajo ASan + UBSan
#  make size      muestra el uso de memoria (.text, .data, .bss)
#  make nm        lista los símbolos y sus secciones de memoria
#  make map       genera el archivo Linker Map (.map)
#  make clean     limpia binarios y artefactos
# ============================================================

CC     ?= gcc
ifeq ($(CC),cc)
  CC := gcc
endif

CFLAGS ?= -Wall -Wextra -std=c11 -Og -g3 -Iinclude
WARN   ?= -Werror -Wpedantic -Wshadow -Wconversion -Wdouble-promotion

ifeq ($(OS),Windows_NT)
  EXE := .exe
else
  EXE :=
endif

BASE_BIN := sim_tcs_firmware
BIN      := $(BASE_BIN)$(EXE)

# Fuentes del Hello World (SIL - sim_linux)
SRCS   := src/main.c \
          src/hal/sim_linux/hal_uart_sim.c \
          src/hal/sim_linux/hal_gpio_sim.c

HDRS   := include/config/system_constants.h \
          include/hal/hal_types.h \
          include/hal/hal_uart.h \
          include/hal/hal_gpio.h

.PHONY: all run warnings sanitize size nm map clean cmake-build

all: $(BIN)

$(BIN): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(WARN) $(SRCS) -o $(BIN)

run: $(BIN)
	@echo ================ $(BIN) ================
	@./$(BIN)

warnings:
	@echo "--- Verificación con Zero Warnings estrictos ---"
	$(CC) $(CFLAGS) $(WARN) $(SRCS) -o $(BIN)

sanitize:
ifeq ($(OS),Windows_NT)
	@echo "--- ASan + UBSan vía WSL Ubuntu ---"
	@wsl -d Ubuntu -- bash -c "gcc $(CFLAGS) $(WARN) -fsanitize=address,undefined $(SRCS) -o $(BASE_BIN)_sanitize && ./$(BASE_BIN)_sanitize && rm -f $(BASE_BIN)_sanitize"
else
	@echo "--- ASan + UBSan (análisis dinámico) ---"
	$(CC) $(CFLAGS) $(WARN) -fsanitize=address,undefined $(SRCS) -o $(BASE_BIN)_sanitize
	./$(BASE_BIN)_sanitize
endif

size: $(BIN)
	@echo "--- Footprint de Memoria (Flash: .text | SRAM: .data, .bss) ---"
	size $(BIN)

nm: $(BIN)
	@echo "--- Tabla de Símbolos del Proyecto (nm) ---"
	nm -S $(BIN)

map:
	@echo "--- Generando Linker Map ($(BASE_BIN).map) ---"
	$(CC) $(CFLAGS) $(WARN) -Wl,-Map=$(BASE_BIN).map $(SRCS) -o $(BIN)

cmake-build:
	@cmake -B build -S .
	@cmake --build build

clean:
ifeq ($(OS),Windows_NT)
	@cmd /c "del /f /q $(BASE_BIN) $(BASE_BIN).exe $(BASE_BIN)_sanitize $(BASE_BIN)_sanitize.exe $(BASE_BIN).map 2>NUL || exit /b 0"
else
	rm -f $(BASE_BIN) $(BASE_BIN).exe $(BASE_BIN)_sanitize $(BASE_BIN)_sanitize.exe $(BASE_BIN).map
endif

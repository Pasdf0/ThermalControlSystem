# ============================================================
#  TCS - Sistema de Control Térmico y Modos de Vuelo
#  Departamento de Ingeniería Informática — USACH
# ------------------------------------------------------------
#  make           compila la simulación del firmware (SIL)
#  make run       ejecuta la simulación
#  make warnings  verifica compilación con flags estrictos
#  make sanitize  compila y ejecuta bajo ASan + UBSan
#  make clean     limpia binarios y artefactos
# ============================================================

CC     ?= gcc
ifeq ($(CC),cc)
  CC := gcc
endif

CFLAGS ?= -Wall -Wextra -std=c11 -Og -g3 -Iinclude
WARN   ?= -Werror -Wpedantic -Wshadow -Wconversion -Wdouble-promotion

BIN    := sim_tcs_firmware

# Fuentes del Hello World
SRCS   := src/main.c \
          src/hal/sim_linux/hal_uart_sim.c

HDRS   := include/config/system_constants.h \
          include/hal/hal_uart.h

RM     := rm -f
ifeq ($(OS),Windows_NT)
  ifeq ($(shell where rm 2>NUL),)
    RM := cmd /c del /f /q 2>NUL || exit 0
  endif
endif

.PHONY: all run warnings sanitize clean

all: $(BIN)

$(BIN): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(WARN) $(SRCS) -o $@

run: $(BIN)
	@echo ================ $(BIN) ================
	@./$(BIN)

warnings:
	@echo "--- Verificación con Zero Warnings estrictos ---"
	$(CC) $(CFLAGS) $(WARN) $(SRCS) -o $(BIN)

sanitize:
	@echo "--- ASan + UBSan (análisis dinámico) ---"
	$(CC) $(CFLAGS) -fsanitize=address,undefined -g3 $(SRCS) -o $(BIN)_sanitize
	-./$(BIN)_sanitize

clean:
	$(RM) $(BIN) $(BIN).exe $(BIN)_sanitize $(BIN)_sanitize.exe

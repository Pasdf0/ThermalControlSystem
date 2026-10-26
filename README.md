# Sistema de Control Térmico y Modos de Vuelo para Nanosatélites (TCS)

* **Asignatura:** Programación de Sistemas Embebidos (2026-2) - Departamento de Ingeniería Informática, USACH.
* **Profesor:** Carlos González Cortés.
* **Autor:** Amaru Monje Rojas (`amaru.monje@usach.cl`).
* **Documentación Detallada:** Consulta [DOCS.md](DOCS.md) para estándares de código (JPL Power of 10), decisiones de diseño, mitigación de riesgos, estrategia de pruebas y cronograma.

---

## 1. Descripción y Alcance del Proyecto

En el entorno de órbita baja terrestre (LEO), los nanosatélites experimentan oscilaciones térmicas extremas producto de la transición periódica entre radiación solar directa y eclipse en la sombra terrestre. Estas variaciones pueden provocar congelamiento, degradación prematura de baterías o sobrecalentamiento crítico de la electrónica de abordo.

Este proyecto implementa un **Sistema de Control Térmico (TCS)** en lenguaje **C** sobre **FreeRTOS** para el microcontrolador **ESP32-WROOM-32D**. El firmware regula continuamente el entorno térmico, gestiona cambios de modo mediante una máquina de estados finitos (FSM) y opera bajo restricciones estrictas de energía mediante tareas concurrentes independientes.

---

## 2. Especificaciones de Hardware y Periféricos

El sistema integra los siguientes módulos de hardware:

* **Microcontrolador (MCU):** Placa basada en **ESP32-WROOM-32D** de 32 bits (arquitectura Xtensa Dual-Core LX6), con soporte para FreeRTOS, bus I2C, puertos GPIO (entradas y salidas digitales), temporizadores/PWM y puerto UART.
* **Sensor 1 - Detección de Luz (Sol / Eclipse):** Módulo sensor **LDR genérico** alimentado mediante pin VCC y conectado a una entrada digital **GPIO** (salida digital D0 con umbral binario Sol / Sombra de 0 / 3.3V), implementado así debido a que el rango de voltaje del sensor no coincidía con las especificaciones del conversor ADC.
* **Sensor 2 - Monitoreo de Temperatura:** Sensor digital **BMP280** conectado mediante bus **I2C** para medir la temperatura interna en grados Celsius (°C).
* **Actuador 1 - Calefactor:** Módulo **Relé de 5V (1 canal, SRD-05VDC-SL-C)** con aislamiento optoacoplado y control lógico compatible con 3.3V, accionado mediante señal digital **GPIO** (ON/OFF).
* **Actuador 2 - Persiana de Disipación Térmica:** Micro servomotor **SG90** controlado por modulación de ancho de pulso (**PWM**) para abrir o cerrar la persiana disipadora.
* **Interfaz de Monitoreo:** Consola serial hacia computadora mediante cable **USB / UART** (115200 bps) para telemetría, registro y alertas.

---

## 3. Arquitectura de Software y Estructura del Repositorio

El software se organiza en capas desacopladas: **Sistema Operativo en Tiempo Real (FreeRTOS)**, **Software de Aplicación (FSM y Control)**, **Drivers de Periféricos** y **Capa de Abstracción de Hardware (HAL)**.

### 3.1. Máquina de Estados Finitos (FSM)
El algoritmo de control organiza el comportamiento del satélite en 5 modos:
1. **Inicio:** Configuración de periféricos, comprobación de buses y establecimiento de actuadores en estado seguro (apagados).
2. **Modo Sol Normal:** Presencia de luz solar y temperatura en rango nominal; relé apagado y persiana cerrada.
3. **Modo Eclipse / Frío:** Satélite en sombra y temperatura bajo el umbral mínimo; se activa el relé calefactor para prevenir congelamiento.
4. **Modo Sobrecalentamiento:** Temperatura superior al umbral máximo; el servomotor acciona la apertura de la persiana para disipar calor.
5. **Modo Seguro:** Activado ante desconexión o fallo de lectura de sensores; desconecta inmediatamente el calefactor y emite una alerta periódica por UART.

### 3.2. Tareas Concurrentes (FreeRTOS)
Las operaciones se distribuyen en 3 tareas concurrentes comunicadas mediante colas y semáforos:
* **Tarea 1 - Adquisición de Sensores (`task_sensors`):** Lectura periódica del estado de iluminación (entrada digital GPIO) y temperatura interna (bus I2C).
* **Tarea 2 - Control Térmico y FSM (`task_control_fsm`):** Procesamiento de eventos térmicos, cálculo de histéresis y control del relé y servomotor.
* **Tarea 3 - Telemetría Serial (`task_telemetry_uart`):** Transmisión no bloqueante de datos, estados y alertas vía UART.

### 3.3. Estructura de Archivos
```text
ThermalControlSystem/
├── CMakeLists.txt                  # Build dual: ESP-IDF (ESP32) / CMake nativo (PC) [EV1]
├── Makefile                        # Reglas de compilación, Sanitizers e inspección de memoria [EV1]
├── README.md                       # Especificaciones principales y flujo de build/flash [EV1]
├── DOCS.md                         # Estándares JPL, decisiones de diseño, pruebas y plan [EV1]
├── .gitignore                      # Exclusión de binarios y temporales de build [EV1]
│
├── include/                        # Cabeceras públicas e interfaces
│   ├── config/
│   │   ├── system_constants.h      # Constantes globales, pines y baudrates [EV1]
│   │   └── pinout.h                # Mapeo físico de pines ESP32 [EV2]
│   ├── app/                        # Interfaces de FSM y telemetría [EV2-EV3]
│   ├── drivers/                    # Interfaces de sensores y actuadores [EV2-EV4]
│   │   ├── bmp280.h                # Interfaz del sensor digital I2C [EV2]
│   │   ├── ldr.h                   # Interfaz del sensor digital de luz (GPIO) [EV2]
│   │   ├── heater_relay.h          # Control ON/OFF de relé calefactor [EV3]
│   │   └── servo_shutter.h         # Control PWM del servomotor SG90 [EV3]
│   ├── hal/
│   │   ├── hal_types.h             # Tipos enumerados de estado y niveles HAL [EV1]
│   │   ├── hal_gpio.h              # Contrato de abstracción digital GPIO (E/S) [EV1]
│   │   ├── hal_uart.h              # Contrato de abstracción serial UART [EV1]
│   │   ├── hal_i2c.h               # Contrato para bus serial I2C [EV2]
│   │   └── hal_pwm.h               # Contrato para modulación PWM [EV3]
│   └── os/                         # Configuración de tareas FreeRTOS [EV3-EV4]
│
├── src/                            # Implementación del código fuente (.c)
│   ├── CMakeLists.txt              # Registro de componentes para ESP-IDF [EV1]
│   ├── main.c                      # Punto de entrada y prueba observable acotada EV1 [EV1]
│   ├── app/                        # Lógica de FSM y algoritmo de control [EV2-EV3]
│   ├── drivers/                    # Drivers de BMP280, LDR, relé y SG90 [EV2-EV4]
│   ├── os/                         # Implementación de tareas concurrentes [EV3-EV4]
│   └── hal/
│       ├── esp32/                  # Implementación de HAL sobre hardware ESP32
│       │   ├── hal_uart_esp32.c    # UART para ESP-IDF con validación de estado [EV1]
│       │   └── hal_gpio_esp32.c    # GPIO para ESP-IDF con protección de pines Flash [EV1]
│       └── sim_linux/              # Implementación de HAL para simulación PC (SIL)
│           ├── hal_uart_sim.c      # Emulación de consola serial en terminal [EV1]
│           └── hal_gpio_sim.c      # Emulación de estado de pines en RAM con arreglos legibles [EV1]
│
└── tests/                          # Pruebas SIL e inyección de fallos [EV2-EV5]
```

---

## 4. Entorno de Desarrollo y Herramientas

* **IDE:** Visual Studio Code con extensión oficial **ESP-IDF**.
* **Sistema de Construcción:** **CMake** (v3.16+) y **GNU Make**.
* **Toolchains:**
  * **Target ESP32 (Hardware / HIL):** `xtensa-esp32-elf-gcc` provisto por **ESP-IDF v5.x**.
  * **Target Simulación (Host / SIL):** `gcc` (estándar C11) bajo Linux, WSL o Windows.
* **Política de Compilación:** Compilación estricta con **Zero Warnings** (`-Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wdouble-promotion`).

---

## 5. Flujo de Build – Flash – Debug

### 5.1. Flujo en PC Host (Simulación SIL y Auditoría de Memoria)
Permite validar la lógica de software, el estado de los periféricos y la distribución de memoria sin necesidad de conectar la placa física:

```bash
# Compilar con Zero Warnings estrictos
make warnings

# Ejecutar la simulación con salida observable (estados de pines y UART)
make run

# Compilar y ejecutar instrumentado con Sanitizers (ASan + UBSan)
make sanitize

# Auditar uso de memoria Flash (.text) y SRAM (.data, .bss), tabla de símbolos y Linker Map
make size
make nm
make map

# Limpiar binarios y artefactos generados
make clean
```

Alternativamente con CMake:
```bash
cmake -B build -S .
cmake --build build
./build/sim_tcs_firmware
```

### 5.2. Flujo en Microcontrolador ESP32 (Plataforma Física)
El proyecto incluye soporte nativo para el entorno oficial de Espressif:

```bash
# 1. Seleccionar el target del microcontrolador (única vez)
idf.py set-target esp32

# 2. Compilar firmware completo
idf.py build

# 3. Flashear la memoria del ESP32 por puerto USB serial
idf.py -p COM3 flash

# 4. Abrir la consola de telemetría UART (115200 bps)
#    (Usar 'Ctrl + ]' para salir)
idf.py -p COM3 monitor

# 5. Build, Flash y Monitor en un solo comando
idf.py -p COM3 flash monitor

# 6. Depuración por hardware mediante JTAG / OpenOCD
idf.py openocd
```

---

## 6. Estado Actual del Proyecto (Hito EV1 - Entrega Inicial)

En cumplimiento con los requerimientos de la primera evaluación sumativa (**EV1**):

* [x] **Selección de Plataforma y Toolchain:** Microcontrolador **ESP32-WROOM-32D** con arquitectura Xtensa Dual-Core, entorno **ESP-IDF / FreeRTOS** y simulación host en C11.
* [x] **Sistema de Build Dual:** Integración de **CMake** nativo y **Makefile** con soporte para análisis dinámico (`ASan`/`UBSan`) e inspección de memoria (`size`, `nm`, `map`).
* [x] **Capa HAL Tipada y Segura:** Contratos `hal_uart` y `hal_gpio` con retorno de estado (`hal_status_t`), seguimiento de pines mediante arreglos legibles en memoria (`s_pin_mode`, `s_pin_level`) y protección de pines reservados de SPI Flash en ESP32.
* [x] **Prueba Básica Funcional Propia (Hello World Observable):** Código en `src/main.c` con ciclo acotado determinista (JPL Regla 4), verificación de retornos (JPL Regla 6) y cero advertencias de compilación.
* [ ] **Próximos Pasos (EV2 - Semanas 4 a 6):** Implementación del contrato I2C, drivers de sensores (BMP280 en I2C y LDR en GPIO digital) y primera versión de la FSM térmica.

---

## 7. Documentación Complementaria

Para profundizar en los aspectos metodológicos, normativos y de diseño del proyecto, consulta:
* [DOCS.md](DOCS.md):
  * Estándares de codificación (Reglas de JPL "The Power of 10" y gestión de memoria).
  * Decisiones de diseño y tabla de mitigación de riesgos técnicos.
  * Estrategia de pruebas SIL/HIL bajo el modelo en V.
  * Cronograma y planificación de hitos (EV1 a EV5).

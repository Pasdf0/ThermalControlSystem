# Sistema de Control Térmico y Modos de Vuelo para Nanosatélites (TCS)

* **Asignatura:** Programación de Sistemas Embebidos (2026-2) - Departamento de Ingeniería Informática, USACH.
* **Profesor:** Carlos González Cortés.
* **Autor:** Amaru Monje Rojas (`amaru.monje@usach.cl`).

---

## 1. Descripción y Alcance del Proyecto

En el entorno de órbita baja terrestre (LEO), los nanosatélites experimentan oscilaciones térmicas extremas producto de la transición entre radiación solar directa y eclipse en la sombra terrestre. Estas variaciones pueden causar congelamiento, degradación irreversible o sobrecalentamiento en las baterías y la electrónica.

Este proyecto implementa un **Sistema de Control Térmico (TCS)** en lenguaje **C** sobre **FreeRTOS** que regula continuamente el entorno térmico, gestiona cambios entre modos mediante una máquina de estados finitos (FSM) y opera bajo restricciones de energía mediante tareas concurrentes sin depender de telemetría externa.

---

## 2. Especificaciones de Hardware y Periféricos

El sistema está compuesto por los siguientes módulos de hardware:

* **Microcontrolador (MCU):** Placa basada en **ESP32-WROOM-32D** de 32 bits, con soporte para FreeRTOS, entradas analógicas (ADC), bus I2C, puertos GPIO, temporizadores/PWM y comunicación UART.
* **Sensor 1 - Detección de Luz (Sol / Eclipse):** Fotoresistencia **LDR** conectada a una entrada analógica **ADC** (voltaje de 0 a 3.3V) para medir la radiación lumínica ambiental.
* **Sensor 2 - Monitoreo de Temperatura:** Sensor digital **BMP280** conectado mediante protocolo **I2C** para medir la temperatura interna en grados Celsius (°C).
* **Actuador 1 - Calefactor:** Módulo **Relé de 5V (1 canal, SRD-05VDC-SL-C)** con aislamiento y control lógico compatible con 3.3V, accionado mediante señal digital **GPIO** (ON/OFF).
* **Actuador 2 - Persiana de Disipación Térmica:** Micro servomotor **SG90** controlado por modulación de ancho de pulso (**PWM**) para abrir o cerrar la persiana térmica.
* **Interfaz de Monitoreo:** Consola serial hacia una computadora mediante cable **USB / UART** para visualización de datos y alertas.

---

## 3. Arquitectura de Software y Estructura del Repositorio

El software se divide en capas desacopladas: **Sistema Operativo en Tiempo Real (RTOS)**, **Software de Aplicación (FSM y Algoritmo de Control)**, **Drivers de Componentes Externos** y **Capa de Abstracción de Hardware (HAL)**.

### 3.1. Máquina de Estados Finitos (FSM)
El algoritmo de control organiza el comportamiento del satélite en 5 estados:
1. **Inicio:** Configura los puertos periféricos, comprueba la comunicación inicial con los sensores y asegura que los actuadores partan apagados.
2. **Modo Sol Normal:** Existe presencia de luz y la temperatura se encuentra dentro de los umbrales definidos; el calefactor se mantiene apagado y la persiana cerrada.
3. **Modo Eclipse / Frío:** El satélite está en sombra y la temperatura cae bajo el umbral mínimo; se activa el relé (calefactor) para evitar el congelamiento.
4. **Modo Sobrecalentamiento:** La temperatura supera el umbral máximo; se acciona el servomotor para abrir la persiana y disipar el calor.
5. **Modo Seguro:** Si algún sensor deja de responder o entrega datos erróneos, el sistema apaga el calefactor por precaución y envía una alerta inmediata por el puerto serial (UART).

### 3.2. Tareas Concurrentes (FreeRTOS)
Las operaciones se ejecutan en 3 tareas concurrentes que intercambian datos mediante colas y semáforos de FreeRTOS:
* **Tarea 1 - Adquisición de Sensores (`task_sensors`):** Lee periódicamente el nivel de luz (LDR por ADC) y la temperatura (BMP280 por I2C), generando las mediciones para el control.
* **Tarea 2 - Control Térmico y Estados (`task_control_fsm`):** Evalúa las mediciones y eventos, actualiza el estado de la FSM y controla el relé (GPIO) y el servomotor (PWM).
* **Tarea 3 - Telemetría (`task_telemetry_uart`):** Transmite periódicamente mediciones, cambios de modo y alertas por UART, evitando que la comunicación serial bloquee la ejecución del control térmico.

### 3.3. Estructura Tradicional del Proyecto (`include/` y `src/`)
Siguiendo la convención estándar en C para software de vuelo, las cabeceras (`.h`) se centralizan en `include/` y las implementaciones (`.c`) en `src/`, separando cada periférico y cada puerto de arquitectura (`esp32` vs `sim_linux`) para evitar acoplamiento y colisiones en el enlazador (`ld`):

```text
ThermalControlSystem/
├── Makefile                        # Reglas de compilación (make sim / make esp32), Zero Warnings y análisis
├── README.md                       # Guía de arquitectura, requerimientos y estándares de desarrollo
│
├── include/                        # Cabeceras públicas e interfaces (-Iinclude)
│   ├── config/
│   │   ├── system_constants.h      # Umbrales térmicos, timeouts, histéresis y frecuencias (const)
│   │   └── pinout.h                # Mapeo de pines físicos del ESP32-WROOM-32D
│   ├── app/
│   │   ├── tcs_fsm.h               # Definición de los 5 modos térmicos y transiciones
│   │   ├── thermal_control.h       # Lógica de evaluación térmica y modo seguro
│   │   └── telemetry.h             # Estructuras y formateo de tramas para consola UART
│   ├── drivers/
│   │   ├── bmp280.h                # Interfaz del sensor de temperatura digital (I2C)
│   │   ├── ldr.h                   # Interfaz de conversión de nivel de luz (ADC)
│   │   ├── heater_relay.h          # Interfaz de control ON/OFF del calefactor (Relé 5V)
│   │   └── servo_shutter.h         # Interfaz de apertura/cierre de persiana (SG90)
│   ├── hal/
│   │   ├── hal_gpio.h              # Contrato de abstracción para pines digitales (GPIO)
│   │   ├── hal_adc.h               # Contrato de abstracción para conversor analógico (ADC)
│   │   ├── hal_i2c.h               # Contrato de abstracción para bus serial síncrono (I2C)
│   │   ├── hal_pwm.h               # Contrato de abstracción para señal modulada (PWM)
│   │   └── hal_uart.h              # Contrato de abstracción para puerto serial (UART)
│   └── os/
│       └── tasks_config.h          # Prioridades, tamaños de stack, periodos y colas FreeRTOS
│
├── src/                            # Implementación modular del código fuente (.c)
│   ├── main.c                      # Punto de entrada e inicialización del sistema
│   ├── app/
│   │   ├── tcs_fsm.c               # Implementación determinista de la máquina de estados
│   │   ├── thermal_control.c       # Algoritmo de decisión térmica (independiente del HW)
│   │   └── telemetry.c             # Empaquetado y despacho de reportes de telemetría
│   ├── drivers/
│   │   ├── bmp280.c                # Driver del sensor BMP280 (reemplazable ante falta de stock)
│   │   ├── ldr.c                   # Acondicionamiento de lecturas del sensor LDR
│   │   ├── heater_relay.c          # Control lógico del relé con protección de estado
│   │   └── servo_shutter.c         # Control gradual de ángulo del servomotor SG90
│   ├── os/
│   │   ├── task_sensors.c          # Tarea 1: Lectura periódica de sensores
│   │   ├── task_control_fsm.c      # Tarea 2: Ejecución del bucle de control y FSM
│   │   └── task_telemetry_uart.c   # Tarea 3: Envío no bloqueante de telemetría por UART
│   └── hal/
│       ├── esp32/                  # Puerto para hardware real (ESP32-WROOM-32D)
│       │   ├── bsp_init.c          # Inicialización de clocks y periféricos nativos
│       │   ├── hal_gpio_esp32.c
│       │   ├── hal_adc_esp32.c
│       │   ├── hal_i2c_esp32.c
│       │   ├── hal_pwm_esp32.c
│       │   └── hal_uart_esp32.c
│       └── sim_linux/              # Puerto simulado en PC (MMIO en RAM para pruebas SIL)
│           ├── hal_gpio_sim.c
│           ├── hal_adc_sim.c
│           ├── hal_i2c_sim.c
│           ├── hal_pwm_sim.c
│           └── hal_uart_sim.c
│
└── tests/                          # Verificación y Validación Software-in-the-Loop (SIL)
    ├── test_fsm.c                  # Pruebas de transiciones de estado con vectores simulados
    └── mocks/                      # Inyección de fallos (desconexión I2C, sombras rápidas)
```

---

## 4. Estándares de Codificación (Guideline de Código en C)

Todo archivo en `src/` e `include/` debe adherirse a los lineamientos de código crítico de **JPL ("The Power of 10: Rules for Developing Safety-Critical Code")** y a las reglas de C embebido del curso:

### 4.1. Reglas de "The Power of 10" (JPL)
* **Sin Asignación Dinámica de Memoria:** Prohibido usar `malloc`, `calloc`, `realloc` o `free`. Todos los buffers y estructuras de estado deben reservarse estáticamente o durante la inicialización.
* **Tipos de Ancho Fijo (`<stdint.h>`):** Prohibido usar tipos primitivos de ancho ambiguo (`int`, `short`, `long`, `char` numérico); utilizar exclusivamente `uint8_t`, `int16_t`, `uint32_t`, `uint64_t`, etc., para garantizar portabilidad e integridad de memoria.
* **Flujo de Control Determinista:** Prohibido el uso de recursión, `goto` y `setjmp`/`longjmp` para mantener un consumo de *stack* predecible.
* **Bucles Acotados:** Todos los bucles (`for`, `while`) deben contar con una cota superior fija explícita para evitar bloqueos por ciclos infinitos.
* **Uso Seguro de Punteros:**
  * Limitar la indirección a un máximo de un nivel (no usar punteros dobles `**p`).
  * Validar siempre que un puntero no sea nulo (`if (p != NULL)`) antes de desreferenciarlo.
  * Respetar las reglas de *strict aliasing*; utilizar `memcpy` o `union` para copias o reinterpretación de bytes.
* **Funciones Cortas y Verificación de Retornos:** Mantener funciones breves (baja complejidad ciclomática) y comprobar obligatoriamente el valor de retorno de cada función llamada.

### 4.2. Calificadores de Lenguaje, Registros y Memoria
* **`volatile`:** Obligatorio en punteros o estructuras que mapean registros de hardware (Memory-Mapped I/O) y en variables modificadas por interrupciones (ISR), evitando que niveles de optimización como `-O2` o `-Os` eliminen accesos físicos.
* **`const`:** Aplicar en tablas de búsqueda, parámetros de configuración y punteros de solo lectura (`const uint8_t *`) para proteger contratos de interfaz en drivers y ubicar datos en memoria Flash (ROM) en vez de SRAM.
* **`static`:** Aplicar a todas las variables globales y funciones internas de un archivo `.c` para hacerlas privadas al módulo y prevenir conflictos de nombres durante el enlazado.
* **`static inline`:** Utilizar en funciones pequeñas y críticas (como manipulación rápida de pines o rutinas asociadas a ISR) para evitar el sobrecoste (*overhead*) de salto `CALL`/`RET`.
* **Operaciones Bitwise Seguras:** Modificar bits de registros mediante máscaras explícitas (`|=` para SET, `&= ~` para CLEAR, `^=` para TOGGLE), asegurando que los desplazamientos (`<<`, `>>`) sean no negativos y no sobrepasen el ancho del tipo de dato.
* **Alineación de Memoria:** Ordenar los campos de cada `struct` respetando la alineación natural de 4 bytes del procesador (ESP32) para evitar relleno (*padding*) innecesario o excepciones por acceso desalineado, reservando `__attribute__((packed))` solo para tramas de comunicación o ahorro estricto de memoria.

---

## 5. Mitigación de Riesgos en el Diseño

El código implementa las siguientes medidas directas para mitigar los riesgos técnicos identificados en el anteproyecto:

1. **Timeouts en Bus I2C (Desconexión del Sensor):** El driver `bmp280.c` y `hal_i2c` configuran un *timeout* estricto de lectura; si el sensor se desconecta o no responde a tiempo, la función retorna un código de error sin bloquear la tarea y fuerza la transición inmediata al **Modo Seguro**.
2. **Confirmación de Lecturas (Sombras Pasajeras en LDR):** El módulo `thermal_control.c` exige confirmar lecturas estables durante múltiples muestras consecutivas antes de conmutar el relé en `heater_relay.c`, evitando encendidos y apagados rápidos por sombras temporales.
3. **Encendido Gradual del Servomotor:** El driver `servo_shutter.c` incrementa el ciclo de trabajo PWM de forma progresiva y el hardware separa el riel de alimentación con tierra común (GND) para evitar caídas de voltaje en el microcontrolador.
4. **Aislamiento de Nivel Lógico en el Relé:** Se emplea un módulo de relé de 5V que incorpora aislamiento y acepta señales de control de 3.3V desde el GPIO del ESP32.
5. **Abstracción frente a Falta de Stock:** Gracias a que `app/` consume una interfaz genérica, si el sensor BMP280 (I2C) debe ser sustituido por un DHT11 o DHT22 (GPIO), solo se modifica la capa `drivers/` sin alterar la máquina de estados.

---

## 6. Entorno de Desarrollo y Herramientas de Compilación

### 6.1. Herramientas Requeridas
* **Entorno Host:** GNU/Linux (Ubuntu nativo, WSL o Máquina Virtual).
* **Compilación y Automatización:** `gcc` y `make` para el puerto de simulación en PC (`sim_linux`), y toolchain de compilación cruzada con soporte FreeRTOS para el puerto `esp32`.
* **IDE:** VS Code.
* **Verificación de Calidad:** `cppcheck` y `clang-tidy` (análisis estático), junto con **AddressSanitizer (ASan)** y **UndefinedBehaviorSanitizer (UBSan)** (análisis dinámico en PC).

### 6.2. Política de "Zero Warnings" y Flags Obligatorios
El proyecto aplica la política de tratar todas las advertencias como errores de compilación. El `Makefile` exige las siguientes banderas:
* **Advertencias y Estándar:** `-std=c11 -Wall -Wextra -Werror -Wpedantic`.
* **Seguridad en Sistemas Embebidos:** `-Wshadow` (detecta variables locales que ocultan globales), `-Wconversion` (alerta conversiones implícitas que truncan bits) y `-Wdouble-promotion` (evita la promoción implícita de `float` a `double` por software).
* **Niveles de Optimización:** `-Og` (o `-O0`) para desarrollo activo y depuración paso a paso; `-Os` para minimizar el uso de memoria Flash en despliegues sobre el microcontrolador.

### 6.3. Comandos de Compilación y Análisis (Puerto `sim_linux`)
```bash
# 1. Compilar simulación SIL seleccionando únicamente el puerto src/hal/sim_linux/
gcc -std=c11 -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wdouble-promotion \
    -Og -g3 -fsanitize=address -fsanitize=undefined \
    -Iinclude \
    src/main.c src/app/*.c src/drivers/*.c src/hal/sim_linux/*.c \
    -o build/sim_tcs_firmware

# 2. Ejecutar binario instrumentado con ASan y UBSan
./build/sim_tcs_firmware

# 3. Ejecutar análisis estático sobre el código fuente
cppcheck --enable=all --std=c11 -Iinclude src/
```

---

## 7. Estrategia de Pruebas (SIL y HIL)

El proceso de verificación y validación sigue el **V-Model** en dos fases complementarias:

1. **Pruebas de Software (SIL - Software-in-the-Loop):**
   * Compilación en PC enlazando `src/hal/sim_linux/` y `tests/` para inyectar lecturas numéricas simuladas de temperatura y luz mediante archivos y vectores de prueba.
   * Validación de todas las transiciones de la FSM y de la ausencia de fugas o desbordamientos de memoria mediante Sanitizers (`-fsanitize=address`, `-fsanitize=undefined`) antes de cargar el código en el microcontrolador.
2. **Pruebas de Hardware (HIL - Hardware-in-the-Loop):**
   * **Simulación física de luz:** Iluminación directa del sensor LDR con una linterna y bloqueo de luz para comprobar el cambio entre sol y eclipse.
   * **Simulación con potenciómetro:** Conexión de un potenciómetro a la entrada ADC para emular manualmente subidas y caídas de temperatura.
   * **Inyección física de fallos:** Desconexión del cable del sensor de temperatura BMP280 para verificar que el *timeout* actúe, el sistema pase a **Modo Seguro**, apague el calefactor y envíe la alerta por UART.
   * **Verificación de estados:** Confirmación de que cada condición de entrada física corresponda exactamente con las salidas del relé, el ángulo del servomotor SG90 y los reportes en la consola serial.
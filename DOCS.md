# Documentación de Diseño, Estándares y Planificación (TCS)

* **Proyecto:** Sistema de Control Térmico y Modos de Vuelo para Nanosatélites (TCS)
* **Documento Principal:** [README.md](README.md) (Especificaciones de hardware, arquitectura y flujo de build/flash).

---

## 1. Estándares de Codificación (Guideline de Código en C)

Todo el código fuente en `src/` e `include/` implementa de forma verificable los lineamientos de software crítico de **JPL ("The Power of 10: Rules for Developing Safety-Critical Code")** y las directrices de C embebido de la Unidad 1 del curso:

### 1.1. Reglas de "The Power of 10" (JPL) Aplicadas en el Código
1. **Sin Asignación Dinámica de Memoria:** Prohibido el uso de `malloc`, `calloc`, `realloc` o `free`. Todos los buffers y arreglos de estado de pines (`s_pin_mode` y `s_pin_level` en `hal_gpio_sim.c` y `hal_gpio_esp32.c`) se reservan estáticamente en la sección `.bss` en tiempo de compilación.
2. **Tipos de Ancho Fijo (`<stdint.h>`) y Sufijos Portables:** Prohibido el uso de tipos primitivos ambiguos (`int`, `short`, `long`). Se utiliza `<stdint.h>` (`uint8_t`, `uint32_t`) y sufijos explícitos de 32 bits (`115200UL` en `system_constants.h`) junto con tipos enumerados fuertes (`hal_status_t`, `hal_gpio_mode_t`, `hal_gpio_level_t` en `hal_types.h`).
3. **Flujo de Control Determinista:** Prohibido el uso de recursión, `goto` y `setjmp`/`longjmp`, garantizando cotas fijas en el consumo de *stack*.
4. **Bucles Acotados:** Todo ciclo (`for`, `while`) cuenta con una cota superior constante explícita (como `TCS_EV1_BLINK_CYCLES` en `src/main.c`) para prevenir bloqueos por lazos infinitos.
5. **Uso Seguro de Punteros y Verificación de Límites:**
   * Máximo un nivel de indirección (sin punteros dobles `**p`).
   * Validación defensiva de puntero nulo (`if (str == NULL) return HAL_ERR_PARAM;`) en `hal_uart_puts` antes de desreferenciar.
   * Validación estricta de límites de arreglo (`gpio_is_valid_pin`) antes de indexar cualquier posición de memoria, evitando accesos fuera de rango.
6. **Funciones Cortas y Verificación Obligatoria de Retornos:** Todas las funciones del HAL retornan un código tipado `hal_status_t` (`HAL_OK`, `HAL_ERR_PARAM`, `HAL_ERR_STATE`, `HAL_ERROR`), el cual es verificado obligatoriamente en `src/main.c` y en las llamadas al SDK de ESP-IDF (`hal_gpio_esp32.c`).

### 1.2. Calificadores de Lenguaje, Memoria y Abstracción Legible
* **`volatile`:** Aplicado a los arreglos de estado de los pines (`s_pin_mode`, `s_pin_level`) para asegurar que el optimizador del compilador (`-Og`, `-Os`) no elimine lecturas ni escrituras durante el control de periféricos.
* **`const`:** Aplicado en punteros de solo lectura (`const char *str` en `hal_uart.h`), protegiendo contratos de interfaz y ubicando literales en memoria Flash (`.rodata`) en lugar de SRAM.
* **`static` (Linkage Interno):** Aplicado a todas las variables de estado de módulo (`s_pin_initialized`, `s_pin_mode`, `s_pin_level`, `s_uart_initialized`) y funciones auxiliares privadas dentro de cada archivo `.c`, garantizando encapsulamiento y evitando colisiones en el enlazador (`ld`).
* **`static inline`:** Empleado en funciones auxiliares de comprobación de pines (`gpio_is_valid_pin` y `esp32_is_safe_pin`), eliminando el sobrecoste (*overhead*) de salto `CALL`/`RET`.
* **Abstracción por Arreglos Legibles vs. Bitwise:** En lugar de manipular registros empaquetados bit a bit mediante máscaras complejas (`|=`, `&= ~`, `^=`), se adopta un diseño claro y pedagógico basado en arreglos estáticos indexados por número de pin (`s_pin_mode[pin]`, `s_pin_level[pin]`). Esto maximiza la legibilidad, mantenibilidad y depuración sin sacrificar el determinismo en tiempo constante $\mathcal{O}(1)$.
* **Inspección de Secciones de Memoria (`.text`, `.rodata`, `.data`, `.bss`):** El `Makefile` incorpora las reglas `make size`, `make nm` y `make map` (`-Wl,-Map=...`) para auditar el uso de Flash y SRAM.

---

## 2. Decisiones de Diseño y Mitigación de Riesgos

La arquitectura de software incorpora soluciones preventivas directas para mitigar los riesgos técnicos identificados en el anteproyecto:

| Riesgo Técnico Identificado | Decisión de Diseño / Mitigación en Código | Módulo Responsable |
| :--- | :--- | :--- |
| **Incompatibilidad de voltaje LDR con conversor ADC** | Se adopta un módulo sensor LDR genérico alimentado a VCC conectado a una entrada digital GPIO (salida D0), simplificando la detección Sol/Eclipse a un umbral binario determinista sin sobrecargar el microcontrolador. | Hardware / `hal_gpio` |
| **Colisión con pines de SPI Flash en ESP32** | Validación mediante `esp32_is_safe_pin()` que bloquea el uso de los pines GPIO 6 a 11 (reservados a la Flash interna) y verifica capacidad de salida. | `hal/esp32/hal_gpio_esp32.c` |
| **Bloqueo por sensor desconectado (Bus I2C)** | Implementación de *timeout* acotado en lecturas I2C. Ante falta de respuesta, retorna `HAL_ERR_TIMEOUT` y gatilla transición a **Modo Seguro**. | `hal_i2c` / `drivers/bmp280.c` |
| **Falsos disparos por sombras pasajeras (LDR)** | Filtro de confirmación de lecturas digitales continuas durante $N$ muestras consecutivas antes de ordenar conmutación térmica. | `app/thermal_control.c` |
| **Caídas de tensión por consumo de motores** | Rampa de arranque gradual mediante incremento progresivo de ciclo de trabajo PWM y separación de rieles de alimentación con GND común. | `drivers/servo_shutter.c` |
| **Incompatibilidad de niveles lógicos en relé** | Módulo relé de 5V optoacoplado que conmuta con nivel lógico de 3.3V proveniente del GPIO del microcontrolador. | Hardware / `drivers/heater_relay.c` |
| **Quiebre de stock de componentes** | Capa HAL y Drivers desacoplados de la FSM; sustituir un componente solo exige modificar el driver específico sin alterar la lógica de modos. | `include/hal/` |

---

## 3. Estrategia de Pruebas y Verificación (SIL y HIL)

El proyecto adopta el **V-Model** para la verificación continua del software en dos entornos complementarios:

```text
       Requerimientos del Sistema              Validación Hardware-in-the-Loop (HIL)
             \                                      /
        Diseño Arquitectónico                Integración (FreeRTOS + Sensores)
              \                                    /
          Diseño Modular (HAL/Drivers)    Pruebas SIL (PC Host + Sanitizers)
                \                              /
                 ====== Implementación en C ======
```

### 3.1. Fase SIL (Software-in-the-Loop) en PC
* **Objetivo:** Validar la lógica de control, el estado de los pines mediante arreglos en RAM y la ausencia de errores de memoria antes de cargar en el microcontrolador.
* **Mecanismo:** Enlace con el puerto `src/hal/sim_linux/` que modela el estado de cada pin mediante arreglos estáticos en RAM.
* **Análisis Dinámico:** Ejecución con AddressSanitizer (**ASan**) y UndefinedBehaviorSanitizer (**UBSan**) mediante `make sanitize`.
* **Auditoría de Memoria:** Inspección de secciones y símbolos con `make size`, `make nm` y `make map`.

### 3.2. Fase HIL (Hardware-in-the-Loop) en ESP32
* **Objetivo:** Validar la interacción física real con el microcontrolador y periféricos bajo condiciones operativas.
* **Prueba de Sensor Lumínico (LDR):** Aplicación deliberada de luz focalizada (linterna) y bloqueo físico de luz (simulación de eclipse) verificando la alternancia del nivel lógico (0/1) en la entrada digital GPIO y la telemetría UART.
* **Prueba de Sensor Térmico (BMP280):** Variación de temperatura y simulación de umbrales para verificar los cambios de estado de la FSM.
* **Inyección Física de Fallos:** Desconexión deliberada del bus I2C durante la operación para verificar que el sistema entre a **Modo Seguro**, desactive el actuador de calor y reporte la anomalía por serial.

---

## 4. Planificación del Proyecto (Hitos EV1 – EV5)

| Hito | Semana | Alcance y Entregables | Responsable |
| :---: | :---: | :--- | :--- |
| **EV1** | Sem 3 | Selección de plataforma (ESP32), sistema de build dual (CMake/Make), HAL tipado con validación de errores y prueba básica observable (*Hello World* con ciclo acotado). | Amaru Monje |
| **EV2** | Sem 6 | Capa de abstracción I2C y PWM, drivers de sensores (BMP280 en I2C, LDR en entrada digital GPIO) y primera versión de la FSM. | Amaru Monje |
| **EV3** | Sem 10 | Integración con FreeRTOS (3 tareas concurrentes, colas, semáforos) y drivers de actuadores (Relé, Servomotor SG90). | Amaru Monje |
| **EV4** | Sem 13 | Pruebas integradas de ciclo térmico completo, inyección de fallos y telemetría serial en tiempo real. | Amaru Monje |
| **EV5** | Sem 16 | Validación final HIL del prototipo físico, optimizaciones de consumo y entrega del informe final. | Amaru Monje |

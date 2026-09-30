# Monitor interactivo RP2040 — mini REPL didáctico

Proyecto para Raspberry Pi Pico original con RP2040 y Pico SDK.

## Objetivo

Construir un pequeño monitor de comandos inspirado en la idea de una consola REPL.
El alumno escribe un comando, el programa lo interpreta y ejecuta una función.

## Hardware

- Raspberry Pi Pico original
- LED integrado en GPIO25
- Botón externo en GPIO13
- Botón entre GPIO13 y 3.3 V
- Pull-down interno habilitado

## Comandos

```text
help
on
off
toggle
read
sio
mask
```

## Ejemplos

```text
RP2040> on
LED GPIO25 = 1
sio_hw->gpio_set = 1u << 25;

RP2040> off
LED GPIO25 = 0
sio_hw->gpio_clr = 1u << 25;

RP2040> read
GPIO_IN = 0x00002000
GPIO13 = 1

RP2040> sio
SIO_BASE = 0xD0000000
GPIO_IN  = ...
GPIO_OUT = ...
GPIO_OE  = ...
```

## Qué debe estudiar el alumno

### Estructura de un comando

```c
typedef struct {
    const char *name;
    const char *help;
    command_handler_t handler;
} command_t;
```

Cada comando tiene nombre, texto de ayuda y una función asociada.

### Acceso directo a SIO

```c
sio_hw->gpio_set = 1u << 25;
sio_hw->gpio_clr = 1u << 25;
uint32_t entradas = sio_hw->gpio_in;
```

### Dirección

```c
sio_hw->gpio_oe_set = 1u << 25;  // salida
sio_hw->gpio_oe_clr = 1u << 13;  // entrada
```

## Compilación

Con Visual Studio Code y la extensión oficial Raspberry Pi Pico, abre esta carpeta y compila.

Desde terminal:

```bash
cmake -S . -B build -DPICO_BOARD=pico
cmake --build build
```

Se genera:

```text
build/Monitor_REPL_RP2040.uf2
```

## Consola

El proyecto usa USB CDC mediante Pico SDK. Abre el puerto COM de la Pico con PuTTY,
Tera Term o un monitor serial de VS Code.

## Ejercicios posteriores

1. Crear `status`.
2. Crear `blink`.
3. Agregar otro LED.
4. Crear comandos con argumentos.
5. Agregar comandos para TIMER.
6. Agregar comandos para ADC.
7. Agregar comandos para UART.

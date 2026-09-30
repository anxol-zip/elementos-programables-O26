# Ejercicio P3 — Monitor interactivo con TIMER

Proyecto basado en el monitor de la Práctica 3 (mini REPL didáctico sobre Pico SDK).
Se agregan comandos para practicar interrupciones con el TIMER del microcontrolador.

Angel Rugerio Jiménez #201720 — Elementos Programables I

## Objetivo

Agregar al monitor un comando que use el TIMER con un tiempo `n` ingresado por el usuario.
El TIMER genera una interrupción cada `n` segundos, sin bloquear la consola.

## Hardware

Configurado para **Raspberry Pi Pico 2 W** (RP2350). También compila para la Pico original (RP2040).

| Elemento | Pico original | Pico 2 W                       |
|----------|---------------|--------------------------------|
| LED      | GPIO25 (integrado) | GP15, LED externo con 330 Ω a GND |
| Botón    | GPIO13 a 3.3 V, pull-down interno | GPIO13 a 3.3 V, pull-down interno |

En la Pico 2 W el LED integrado está en el chip inalámbrico (CYW43), no en un GPIO,
así que no se puede controlar con el SIO. Por eso se usa un LED externo en GP15.

## Comandos

```text
help
on
off
toggle
read
sio
mask
status
timer_on <n>
timer_off
```

| Comando | Qué hace |
|---------|----------|
| `status` | Muestra el estado del LED (GPIO_OUT) y del botón (GPIO_IN) |
| `timer_on <n>` | Programa ALARM0 para invertir el LED cada `n` segundos (1 a 4294) |
| `timer_off` | Deshabilita la interrupción de ALARM0 y desarma la alarma |

El comando `boton` (interrupción por flanco en GPIO13) está escrito pero **comentado** por ahora.

## Ejemplos

```text
RP2040> timer_on 3
TIMER: el LED se invierte cada 3 s
timer_hw->alarm[0] = timerawl + 3000000;
RP2040>
[TIMER] Disparo #1, han transcurrido 3 s

[TIMER] Disparo #2, han transcurrido 3 s

RP2040> timer_off
TIMER detenido despues de 2 disparos
```

## Cómo funciona `timer_on`

```c
// Programar ALARM0 para dentro de n segundos
timer_hw->alarm[0] = timer_hw->timerawl + timer_interval_us;

// Habilitar la interrupcion de ALARM0
timer_hw->inte = ALARM0_MASK;

// Asociar la IRQ de ALARM0 con la ISR y habilitarla en el NVIC
irq_set_exclusive_handler(ALARM0_IRQ, timer0_isr);
irq_set_enabled(ALARM0_IRQ, true);
```

La ISR limpia la interrupción (`timer_hw->intr`), invierte el LED con `sio_hw->gpio_togl`
y vuelve a programar ALARM0 para dentro de `n` segundos.

`ALARM0_IRQ` depende del microcontrolador: en el RP2040 es `TIMER_IRQ_0`; en el RP2350
hay dos timers, `timer_hw` apunta a TIMER0 y su IRQ es `TIMER0_IRQ_0`.

## Compilación

Con Visual Studio Code y la extensión oficial Raspberry Pi Pico, abre esta carpeta y compila.
La placa está configurada como `pico2_w` en `CMakeLists.txt`.

Desde terminal:

```bash
cmake -S . -B build -DPICO_BOARD=pico2_w
cmake --build build
```

Se genera:

```text
build/Monitor_REPL_RP2040.uf2
```

## Consola

El proyecto usa USB CDC mediante Pico SDK. Abre el puerto de la Pico con Tera Term,
PuTTY o el monitor serial de VS Code.

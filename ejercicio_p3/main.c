// Ejercicio P3 - Elementos Programables
// Monitor interactivo del RP2040: comandos con TIMER e interrupcion de GPIO
// Angel Rugerio Jiménez #201720
// 23/09/26

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"
// #include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"

// Pico W / Pico 2 W: el LED integrado esta en el chip inalambrico (CYW43),
// no en un GPIO, asi que se usa un LED externo en GP15 (330 ohm a GND)
#ifdef CYW43_WL_GPIO_LED_PIN
#define LED_PIN 15
#else
#define LED_PIN 25
#endif
#define BUTTON_PIN 13
#define LED_MASK (1u << LED_PIN)
#define BUTTON_MASK (1u << BUTTON_PIN)

#define ALARM0_MASK (1u << 0)

// RP2040 tiene un solo TIMER; RP2350 (Pico 2) tiene dos y timer_hw apunta a TIMER0
#if PICO_RP2350
#define ALARM0_IRQ TIMER0_IRQ_0
#else
#define ALARM0_IRQ TIMER_IRQ_0
#endif
#define TIMER_MAX_S 4294u     // 4294 s * 1000000 us cabe en 32 bits
#define DEBOUNCE_MS 80u       // ventana antirrebote del boton

#define LINE_BUFFER_SIZE 64
#define MAX_ARGS 4

typedef int (*command_handler_t)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *help;
    command_handler_t handler;
} command_t;

static int cmd_help(int argc, char **argv);
static int cmd_on(int argc, char **argv);
static int cmd_off(int argc, char **argv);
static int cmd_toggle(int argc, char **argv);
static int cmd_read(int argc, char **argv);
static int cmd_sio(int argc, char **argv);
static int cmd_mask(int argc, char **argv);
static int cmd_status(int argc, char **argv);
static int cmd_timer_on(int argc, char **argv);
static int cmd_timer_off(int argc, char **argv);
// static int cmd_boton(int argc, char **argv);

static void timer0_isr(void);
// static void button_isr(uint gpio, uint32_t events);

// Variables que usan las interrupciones
static volatile uint32_t timer_interval_us = 0;
static volatile uint32_t timer_count = 0;
// static volatile uint32_t button_count = 0;
// static volatile uint32_t last_click_ms = 0;

static const command_t commands[] = {
    {"help",   "Muestra la lista de comandos ",          cmd_help},
    {"on",     "Enciende el LED (GPIO25 o GP15)",      cmd_on},
    {"off",    "Apaga el LED (GPIO25 o GP15)",         cmd_off},
    {"toggle", "Invierte el estado del LED",           cmd_toggle},
    {"read",   "Lee el boton conectado a GPIO13",      cmd_read},
    {"sio",    "Muestra GPIO_IN, GPIO_OUT y GPIO_OE",  cmd_sio},
    {"mask",   "Muestra la mascara del LED",            cmd_mask},
    {"status", "Muestra el estado del LED y del boton", cmd_status},
    {"timer_on",  "timer_on <n>: invierte el LED cada n s", cmd_timer_on},
    {"timer_off", "Detiene el TIMER",                       cmd_timer_off},
    // {"boton",  "boton on | boton off: interrupcion en GPIO13",    cmd_boton},
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

static void print_binary32(uint32_t value) {
    for (int bit = 31; bit >= 0; --bit) {
        putchar((value & (1u << bit)) ? '1' : '0');
        if ((bit % 4) == 0 && bit != 0) putchar(' ');
    }
}

static void read_line(char *buffer, size_t size) {
    size_t pos = 0;

    while (true) {
        int c = getchar();

        if (c == '\r' || c == '\n') {
            putchar('\n');
            buffer[pos] = '\0';
            return;
        }

        if (c == '\b' || c == 127) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
            }
            continue;
        }

        if (pos < size - 1) {
            buffer[pos++] = (char)c;
            putchar(c);
        }
    }
}

static int split_line(char *line, char **argv, int max_args) {
    int argc = 0;
    char *token = strtok(line, " \t");

    while (token != NULL && argc < max_args) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    return argc;
}

static void execute_command(int argc, char **argv) {
    if (argc == 0) return;

    for (size_t i = 0; i < COMMAND_COUNT; ++i) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            return;
        }
    }

    printf("Comando desconocido: %s\n", argv[0]);
    printf("Escribe 'help'.\n");
}

static int cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("\nComandos disponibles:\n");
    for (size_t i = 0; i < COMMAND_COUNT; ++i)
        printf("  %-9s - %s\n", commands[i].name, commands[i].help);
    return 0;
}

static int cmd_on(int argc, char **argv) {
    (void)argc; (void)argv;
    sio_hw->gpio_set = LED_MASK;
    printf("LED GPIO%d = 1\n", LED_PIN);
    printf("sio_hw->gpio_set = 1u << %d;\n", LED_PIN);
    return 0;
}

static int cmd_off(int argc, char **argv) {
    (void)argc; (void)argv;
    sio_hw->gpio_clr = LED_MASK;
    printf("LED GPIO%d = 0\n", LED_PIN);
    printf("sio_hw->gpio_clr = 1u << %d;\n", LED_PIN);
    return 0;
}

static int cmd_toggle(int argc, char **argv) {
    (void)argc; (void)argv;
    sio_hw->gpio_togl = LED_MASK;
    printf("GPIO%d invertido con GPIO_OUT_XOR\n", LED_PIN);
    return 0;
}

static int cmd_read(int argc, char **argv) {
    (void)argc; (void)argv;
    uint32_t entradas = sio_hw->gpio_in;
    bool boton = (entradas & BUTTON_MASK) != 0;

    printf("GPIO_IN = 0x%08lx\n", (unsigned long)entradas);
    printf("GPIO%d = %d\n", BUTTON_PIN, boton ? 1 : 0);
    return 0;
}

static int cmd_sio(int argc, char **argv) {
    (void)argc; (void)argv;

    uint32_t gpio_in  = sio_hw->gpio_in;
    uint32_t gpio_out = sio_hw->gpio_out;
    uint32_t gpio_oe  = sio_hw->gpio_oe;

    printf("\nSIO_BASE = 0xD0000000\n");

    printf("GPIO_IN  = 0x%08lx  ", (unsigned long)gpio_in);
    print_binary32(gpio_in);
    printf("\n");

    printf("GPIO_OUT = 0x%08lx  ", (unsigned long)gpio_out);
    print_binary32(gpio_out);
    printf("\n");

    printf("GPIO_OE  = 0x%08lx  ", (unsigned long)gpio_oe);
    print_binary32(gpio_oe);
    printf("\n");

    return 0;
}

static int cmd_mask(int argc, char **argv) {
    (void)argc; (void)argv;

    printf("1u << %d\n", LED_PIN);
    printf("Hex: 0x%08lx\n", (unsigned long)LED_MASK);
    printf("Bin: ");
    print_binary32(LED_MASK);
    printf("\n");

    return 0;
}

static int cmd_status(int argc, char **argv) {
    (void)argc; (void)argv;

    uint32_t salidas  = sio_hw->gpio_out;
    uint32_t entradas = sio_hw->gpio_in;

    bool led   = (salidas  & LED_MASK)    != 0;
    bool boton = (entradas & BUTTON_MASK) != 0;

    printf("GPIO%d - LED : %d\n", LED_PIN, led ? 1 : 0);
    printf("GPIO%d - BOTON : %d\n", BUTTON_PIN, boton ? 1 : 0);
    return 0;
}

static void timer0_isr(void) {
    // Limpiar la interrupcion de ALARM0
    timer_hw->intr = ALARM0_MASK;

    // Invertir el LED con GPIO_OUT_XOR
    sio_hw->gpio_togl = LED_MASK;
    timer_count++;

    printf("\n[TIMER] Disparo #%lu, han transcurrido %lu s\n",
           (unsigned long)timer_count, (unsigned long)(timer_interval_us / 1000000u));

    // Programar nuevamente ALARM0 para dentro de n segundos
    timer_hw->alarm[0] = timer_hw->timerawl + timer_interval_us;
}

static int cmd_timer_on(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: timer_on <n>\n");
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 1 || n > (int)TIMER_MAX_S) {
        printf("n debe estar entre 1 y %u segundos\n", TIMER_MAX_S);
        return 1;
    }

    timer_interval_us = (uint32_t)n * 1000000u;
    timer_count = 0;

    // Limpiar una interrupcion vieja antes de habilitarla
    timer_hw->intr = ALARM0_MASK;

    // Programar ALARM0 para dentro de n segundos
    timer_hw->alarm[0] = timer_hw->timerawl + timer_interval_us;

    // Habilitar la interrupcion de ALARM0
    timer_hw->inte = ALARM0_MASK;

    // Asociar TIMER IRQ 0 con nuestra ISR y habilitarla en el NVIC
    irq_set_exclusive_handler(ALARM0_IRQ, timer0_isr);
    irq_set_enabled(ALARM0_IRQ, true);

    printf("TIMER: el LED se invierte cada %d s\n", n);
    printf("timer_hw->alarm[0] = timerawl + %lu;\n", (unsigned long)timer_interval_us);
    return 0;
}

static int cmd_timer_off(int argc, char **argv) {
    (void)argc; (void)argv;

    // Deshabilitar la interrupcion de ALARM0 y la IRQ en el NVIC
    timer_hw->inte = 0;
    irq_set_enabled(ALARM0_IRQ, false);

    // Desarmar ALARM0 por si estaba pendiente
    timer_hw->armed = ALARM0_MASK;

    printf("TIMER detenido despues de %lu disparos\n", (unsigned long)timer_count);
    return 0;
}

// Comando boton: deshabilitado por ahora
/*
static void button_isr(uint gpio, uint32_t events) {
    (void)gpio; (void)events;

    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Debounce: pulsos mas rapidos que DEBOUNCE_MS son rebote mecanico
    if (now - last_click_ms < DEBOUNCE_MS) return;
    last_click_ms = now;

    button_count++;
    bool boton = (sio_hw->gpio_in & BUTTON_MASK) != 0;
    printf("\n[BOTON] Pulsacion #%lu  GPIO%d = %d\n",
           (unsigned long)button_count, BUTTON_PIN, boton ? 1 : 0);
}

static int cmd_boton(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: boton on  |  boton off\n");
        return 1;
    }

    if (strcmp(argv[1], "on") == 0) {
        button_count = 0;
        // Con pull-down, presionar produce un flanco de subida (0 -> 1)
        gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &button_isr);
        printf("Interrupcion de GPIO%d habilitada (flanco de subida)\n", BUTTON_PIN);
    } else if (strcmp(argv[1], "off") == 0) {
        gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, false);
        printf("Interrupcion de GPIO%d deshabilitada, %lu pulsaciones\n",
               BUTTON_PIN, (unsigned long)button_count);
    } else {
        printf("Uso: boton on  |  boton off\n");
        return 1;
    }
    return 0;
}
*/

static void hardware_init(void) {
    gpio_set_function(LED_PIN, GPIO_FUNC_SIO);
    sio_hw->gpio_clr = LED_MASK;
    sio_hw->gpio_oe_set = LED_MASK;

    gpio_set_function(BUTTON_PIN, GPIO_FUNC_SIO);
    sio_hw->gpio_oe_clr = BUTTON_MASK;
    gpio_pull_down(BUTTON_PIN);
}

int main(void) {
    stdio_init_all();
    hardware_init() ;

  /*   while (!stdio_usb_connected()) {
        sleep_ms(50);
    } */
  
    printf("\n=====================================\n");
    printf(" Monitor didactico RP2040 - SIO\n");
    printf("=====================================\n");
    printf("Escribe 'help' para comenzar.\n\n");

    char line[LINE_BUFFER_SIZE];
    char *argv[MAX_ARGS];

    while (true) {
        printf("RP2040> ");
        read_line(line, sizeof(line));
        int argc = split_line(line, argv, MAX_ARGS);
        execute_command(argc, argv);
    }
}

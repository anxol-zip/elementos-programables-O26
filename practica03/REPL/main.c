// Practica 03 - Elementos Programables
// Monitor interactivo del RP2040 y exploración del SIO
// Angel Rugerio Jiménez #201720
// 23/09/26

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico/stdlib.h"
//#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"

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

static const command_t commands[] = {
    {"help",   "Muestra la lista de comandos ",          cmd_help},
    {"on",     "Enciende el LED (GPIO25 o GP15)",      cmd_on},
    {"off",    "Apaga el LED (GPIO25 o GP15)",         cmd_off},
    {"toggle", "Invierte el estado del LED",           cmd_toggle},
    {"read",   "Lee el boton conectado a GPIO13",      cmd_read},
    {"sio",    "Muestra GPIO_IN, GPIO_OUT y GPIO_OE",  cmd_sio},
    {"mask",   "Muestra la mascara del LED",            cmd_mask},
    {"status", "Muestra el estado del LED y del boton", cmd_status},
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
        printf("  %-8s - %s\n", commands[i].name, commands[i].help);
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

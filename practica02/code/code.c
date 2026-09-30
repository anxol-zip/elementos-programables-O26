/*
========================================
PRÁCTICA 2. MÁQUINA DE ESTADOS PARA DOS SEMÁFOROS
CON BOTONES PEATONALES

Elementos Programables 1

Angel Rugerio Jiménez #201720
14/09/2026
========================================
*/

#include <stdio.h>
#include "pico/stdlib.h"

/* Asignación de pines */
#define A_ROJO      0
#define A_AMARILLO  1
#define A_VERDE     2

#define B_ROJO      28
#define B_AMARILLO  27
#define B_VERDE     26

#define BOTON_A     14   // Solicitud peatonal: detener la vialidad A
#define BOTON_B     18   // Solicitud peatonal: detener la vialidad B

/* Temporizaciones */
#define TIEMPO_AMARILLO_MS  2000
#define TIEMPO_PEATON_MS    5000
#define TIEMPO_LECTURA_MS   20     // Periodo de lectura de botones en E0 y E3

/* Estados de la máquina */
typedef enum {
    E0,   // A verde    / B rojo     : circula A
    E1,   // A amarillo / B rojo     : se detiene A
    E2,   // A rojo     / B rojo     : cruza peatón de A
    E3,   // A rojo     / B verde    : circula B
    E4,   // A rojo     / B amarillo : se detiene B
    E5    // A rojo     / B rojo     : cruza peatón de B
} Estado;

/* Color de un semáforo */
typedef enum {
    ROJO,
    AMARILLO,
    VERDE
} Color;

static const char *NOMBRE_ESTADO[] = { "E0", "E1", "E2", "E3", "E4", "E5" };
static const char *DESCRIPCION_ESTADO[] = {
    "Circula vialidad A",
    "Deteniendo vialidad A",
    "Cruce peatonal A",
    "Circula vialidad B",
    "Deteniendo vialidad B",
    "Cruce peatonal B"
};

// Enciende solo la luz indicada; las otras dos del mismo semáforo se apagan
void poner_semaforo(uint rojo, uint amarillo, uint verde, Color color) {
    gpio_put(rojo,     color == ROJO);
    gpio_put(amarillo, color == AMARILLO);
    gpio_put(verde,    color == VERDE);
}

// Los botones van a GND con pull-up interno: presionado = 0
bool boton_presionado(uint pin) {
    return !gpio_get(pin);
}

// Lee de vuelta las salidas para reportar lo que realmente muestra cada semáforo
const char *leer_semaforo(uint rojo, uint amarillo, uint verde) {
    bool r = gpio_get(rojo), y = gpio_get(amarillo), v = gpio_get(verde);
    if (r && !y && !v) return "ROJO";
    if (!r && y && !v) return "AMARILLO";
    if (!r && !y && v) return "VERDE";
    return "INVALIDO";
}

// Retroalimentación por Serial al entrar a un estado
void reportar_estado(Estado estado) {
    uint32_t t_ms = to_ms_since_boot(get_absolute_time());
    printf("[%6lu.%03lus] %s | A: %-8s B: %-8s | %s\n",
           (unsigned long)(t_ms / 1000), (unsigned long)(t_ms % 1000),
           NOMBRE_ESTADO[estado],
           leer_semaforo(A_ROJO, A_AMARILLO, A_VERDE),
           leer_semaforo(B_ROJO, B_AMARILLO, B_VERDE),
           DESCRIPCION_ESTADO[estado]);

    if (gpio_get(A_VERDE) && gpio_get(B_VERDE)) {
        printf("  !! ERROR: ambos semaforos en verde\n");
    }
}

// Programa principal
int main() {
    stdio_init_all();

    // Configurar los seis LED como salidas, todos apagados
    const uint leds[] = { A_ROJO, A_AMARILLO, A_VERDE, B_ROJO, B_AMARILLO, B_VERDE };
    for (int i = 0; i < 6; i++) {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 0);
    }

    // Configurar los dos botones como entradas con pull-up
    const uint botones[] = { BOTON_A, BOTON_B };
    for (int i = 0; i < 2; i++) {
        gpio_init(botones[i]);
        gpio_set_dir(botones[i], GPIO_IN);
        gpio_pull_up(botones[i]);
    }

    printf("\n=== Practica 2: FSM de dos semaforos con botones peatonales ===\n");
    printf("BOTON_A (GP%d, tecla 'a'): detener vialidad A\n", BOTON_A);
    printf("BOTON_B (GP%d, tecla 'b'): detener vialidad B\n\n", BOTON_B);

    Estado estado = E0;

    while (true) {
        switch (estado) {
            case E0:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, VERDE);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, ROJO);
                reportar_estado(estado);

                // Permanece en E0 hasta que se solicite el paso con BOTON_A
                while (!boton_presionado(BOTON_A)) {
                    sleep_ms(TIEMPO_LECTURA_MS);
                }
                printf("  -> BOTON_A presionado: solicitud peatonal en A\n");
                estado = E1;
                break;

            case E1:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, AMARILLO);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, ROJO);
                reportar_estado(estado);
                sleep_ms(TIEMPO_AMARILLO_MS);
                estado = E2;
                break;

            case E2:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, ROJO);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, ROJO);
                reportar_estado(estado);
                sleep_ms(TIEMPO_PEATON_MS);
                estado = E3;
                break;

            case E3:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, ROJO);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, VERDE);
                reportar_estado(estado);

                // Permanece en E3 hasta que se solicite el paso con BOTON_B
                while (!boton_presionado(BOTON_B)) {
                    sleep_ms(TIEMPO_LECTURA_MS);
                }
                printf("  -> BOTON_B presionado: solicitud peatonal en B\n");
                estado = E4;
                break;

            case E4:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, ROJO);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, AMARILLO);
                reportar_estado(estado);
                sleep_ms(TIEMPO_AMARILLO_MS);
                estado = E5;
                break;

            case E5:
                poner_semaforo(A_ROJO, A_AMARILLO, A_VERDE, ROJO);
                poner_semaforo(B_ROJO, B_AMARILLO, B_VERDE, ROJO);
                reportar_estado(estado);
                sleep_ms(TIEMPO_PEATON_MS);
                estado = E0;
                break;
        }
    }
}

// Fin del programa :)

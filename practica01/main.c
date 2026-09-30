/* =========================================================================
  Practica Semaforos Wokwi
  Elementos Programables 1
  Angel Rugerio Jiménez #201720
  02/09/2026
  ========================================================================= 
*/

#include "pico/stdlib.h"

/* Asignación de pines*/
#define A_ROJO      2
#define A_AMARILLO  3
#define A_VERDE     4

#define B_ROJO      6
#define B_AMARILLO  7
#define B_VERDE     8

// Programa principal
int main() {
  // Inicializar los seis GPIO y configurarlos como salidas
  const uint pines[] = { A_ROJO, A_AMARILLO, A_VERDE, B_ROJO, B_AMARILLO, B_VERDE };

  for (int i = 0; i < 6; i++) {
      gpio_init(pines[i]);
      gpio_set_dir(pines[i], GPIO_OUT);
      gpio_put(pines[i], 0);   // Todo apagado al iniciar
  }

  while (true) {
      // Estado 1: A verde / B rojo — 5s
      gpio_put(A_VERDE, 1);
      gpio_put(B_ROJO, 1);
      sleep_ms(5000);
      gpio_put(A_VERDE, 0);

      // Estado 2: A amarillo / B rojo — 2s
      gpio_put(A_AMARILLO, 1);
      sleep_ms(2000);
      gpio_put(A_AMARILLO, 0);

      // Estado 3: A rojo / B rojo — 1s (colchón de seguridad)
      gpio_put(A_ROJO, 1);
      sleep_ms(1000);

      // Estado 4: A rojo / B verde — 5s
      gpio_put(B_ROJO, 0);
      gpio_put(B_VERDE, 1);
      sleep_ms(5000);
      gpio_put(B_VERDE, 0);

      // Estado 5: A rojo / B amarillo — 2s
      gpio_put(B_AMARILLO, 1);
      sleep_ms(2000);
      gpio_put(B_AMARILLO, 0);

      // Estado 6: A rojo / B rojo — 1s (colchón de seguridad antes de reiniciar el ciclo)
      gpio_put(B_ROJO, 1);
      sleep_ms(1000);
      gpio_put(A_ROJO, 0);

      // Con el ciclo while, regresa automáticamente al estado 1.
  }
}

// Fin del programa :)
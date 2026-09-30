# Práctica: Control de dos semáforos sincronizados con Raspberry Pi Pico

## 1. Algoritmo en lenguaje natural

1. Iniciar el programa. 
2. Identificar los seis pines que controlan las luces: rojo, amarillo y verde del semáforo A, y rojo, amarillo y verde del semáforo B.
3. Configurar los seis pines como salidas digitales y apagar todas las luces.
4. Repetir indefinidamente el siguiente ciclo de seis estados:
   1. Encender la luz verde de A y la luz roja de B. Mantener este estado 5 segundos.
   2. Apagar la luz verde de A y encender la luz amarilla de A; B permanece en rojo. Mantener este estado 2 segundos.
   3. Apagar la luz amarilla de A y encender la luz roja de A; ahora ambos semáforos están en rojo. Mantener este estado 1 segundo.
   4. Apagar la luz roja de B y encender la luz verde de B; A permanece en rojo. Mantener este estado 5 segundos.
   5. Apagar la luz verde de B y encender la luz amarilla de B; A permanece en rojo. Mantener este estado 2 segundos.
   6. Apagar la luz amarilla de B y encender la luz roja de B; ambos semáforos quedan nuevamente en rojo. Mantener este estado 1 segundo.
5. Al terminar el estado 6, regresar automáticamente al estado 1 y repetir el ciclo de manera indefinida (el programa nunca termina).

## 2. Diagrama de flujo

Ver archivo `diagrama_flujo.svg` incluido en esta misma carpeta. Puede abrirse con cualquier navegador o insertarse directamente como imagen en el documento de Word (Insertar → Imagen → Este dispositivo).

El diagrama muestra: inicio, configuración de los seis GPIO como salida, los seis estados con su color de A, color de B y duración, y el regreso indefinido al estado 1.

## 3. Explicación breve de la secuencia de los seis estados

El sistema funciona como una intersección donde solo una calle puede tener el paso libre (luz verde) a la vez; esto se logra alternando cuál semáforo está en verde mientras el otro permanece en rojo.

- **Estado 1 (A verde / B rojo, 5 s):** Se permite el paso a la calle A mientras la calle B se mantiene detenida.
- **Estado 2 (A amarillo / B rojo, 2 s):** Se avisa a los conductores de A que el paso está por cerrarse, dándoles tiempo de reacción antes del cambio a rojo. B sigue detenida.
- **Estado 3 (A rojo / B rojo, 1 s):** Ambos semáforos están en rojo. Este intervalo de "todo rojo" es un colchón de seguridad que evita que un vehículo de A que apenas cruzó coincida con el arranque de B.
- **Estado 4 (A rojo / B verde, 5 s):** Ahora se permite el paso a la calle B mientras A permanece detenida.
- **Estado 5 (A rojo / B amarillo, 2 s):** Se avisa a los conductores de B que el paso está por cerrarse, antes de pasar a rojo.
- **Estado 6 (A rojo / B rojo, 1 s):** Ambos semáforos vuelven a quedar en rojo como colchón de seguridad, ahora antes de reiniciar el ciclo.

Después del estado 6 el programa regresa automáticamente al estado 1 (gracias al `while (true)`), por lo que la secuencia se repite de forma indefinida mientras el sistema esté encendido. En ningún momento ambos semáforos están en verde simultáneamente, y siempre existe un intervalo en amarillo antes de cada cambio a rojo, además de un intervalo con ambos semáforos en rojo antes de que el otro arranque.

## 4. Preguntas de reflexión (borrador — revisa y ajusta con tus propias palabras)

1. **¿Por qué los dos semáforos no deben estar en verde al mismo tiempo?**
   Porque ambas calles cruzan la misma intersección; si las dos tuvieran paso libre a la vez, los vehículos de ambas calles podrían colisionar en el cruce.

2. **¿Cuál es la función del estado amarillo?**
   Advertir a los conductores que están circulando que la luz está por cambiar a rojo, dándoles tiempo de frenar o de despejar la intersección antes de que se detenga por completo.

3. **¿Qué propósito tiene mantener ambos semáforos en rojo durante un intervalo?**
   Da un margen de seguridad para que cualquier vehículo que aún esté cruzando termine de hacerlo antes de que arranque el tránsito de la otra calle, evitando colisiones por transición.

4. **¿Qué instrucciones del programa controlan el estado de los LEDs?**
   `gpio_put(pin, 1)` y `gpio_put(pin, 0)`, que encienden o apagan cada LED según el pin correspondiente.

5. **¿Qué instrucción establece la duración de cada estado?**
   `sleep_ms(ms)`, que detiene la ejecución del programa durante el número de milisegundos indicado antes de pasar al siguiente estado.

6. **¿Qué parte del programa hace que la secuencia se repita indefinidamente?**
   El ciclo `while (true) { ... }`, que nunca termina y por lo tanto vuelve a ejecutar los seis estados una y otra vez.
   
## Enlace a Wokwi
**https://wokwi.com/projects/474102907321175041**

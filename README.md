# Cuatro en Línea

Juego Cuatro en Línea implementado en C++ utilizando sockets para la comunicación entre un servidor y varios clientes. Dos jugadores pueden conectarse al servidor y jugar una partida en tiempo real.

## Requisitos

- Sistema operativo compatible con sockets (Linux recomendado).
- Compilador de C++ (GCC recomendado).
- Conexión de red para la comunicación entre el servidor y los clientes.

## Estructura del Proyecto

- `server.cpp`: Código fuente del servidor.
- `client.cpp`: Código fuente del cliente.
- `Makefile` : Archivo de configuración para automatizar la compilación.

## Compilación

- Para compilar el servidor y el cliente, se utiliza el comando Make.
- Se debe escribir el comando "./servidor (Número de puerto)".
- Ejecutar comando "./cliente (dirección ip del servidor) (número de puerto)".

## Nota
- Para que comience el juego deben de haber al menos 2 jugadores y luego estos deben de ingresar un valor entre el 1 y 7, que indica el número de columna que quieran jugar.
- El juego finalizará cuando un jugador alinee cuatro fichas consecutivas o el tablero se llene, resultando en un empate.

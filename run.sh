#!/bin/bash

# Compilar todos los programas
gcc -o ecuafast ecuafast.c -lpthread
gcc -o sri sri.c
gcc -o senae senae.c
gcc -o supercia supercia.c
gcc -o puerto puerto.c -lpthread

# Iniciar entidades de control en segundo plano
./sri &
PID_SRI=$!
echo "SRI iniciado con PID $PID_SRI"

./senae &
PID_SENAE=$!
echo "SENAE iniciado con PID $PID_SENAE"

./supercia &
PID_SUPERCIA=$!
echo "SUPERCIA iniciado con PID $PID_SUPERCIA"

# Iniciar el administrador del puerto en segundo plano
./puerto &
PID_PUERTO=$!
echo "Puerto iniciado con PID $PID_PUERTO"

# Esperar 3 segundos para que todo esté listo
sleep 2

# Ejecutar ECUAFAST
./ecuafast
echo "ECUAFast finalizó."

# Esperar explícitamente a que el puerto y las entidades terminen
wait $PID_PUERTO
echo "Puerto finalizó."

wait $PID_SRI
echo "SRI finalizó."

wait $PID_SENAE
echo "SENAE finalizó."

wait $PID_SUPERCIA
echo "SUPERCIA finalizó."

echo "Todos los procesos han finalizado correctamente."

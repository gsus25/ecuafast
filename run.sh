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

# Esperar 3 segundos antes de ejecutar ECUAFAST
sleep 3

# Ejecutar ECUAFAST
./ecuafast

# Finalizar procesos secundarios
kill $PID_SRI $PID_SENAE $PID_SUPERCIA $PID_PUERTO
echo "Todos los procesos finalizados."

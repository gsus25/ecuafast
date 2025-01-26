# ECUAFAST
ECUAFast es un sistema distribuido diseñado para agilizar el proceso de aforo de contenedores en puertos ecuatorianos. Simula la interacción entre barcos y entidades de control (SRI, SENAE, SUPERCIA), así como el administrador del puerto, utilizando un modelo concurrente y distribuido.

## Descripción General
- **Barcos (ECUAFast):** Simulan la llegada de barcos al mar territorial y su comunicación con las entidades de control para decidir si requieren aforo.
- **Entidades de Control:** Cada entidad (SRI, SENAE, SUPERCIA) aplica reglas específicas para decidir si un barco recibe `PASS` o `CHECK`.
- **Administrador del Puerto:** Gestiona la cola de atraco de barcos y prioriza según las reglas del proyecto.

 ## Compilación
Para compilar el programa:
```
$ make
```
Para limpieza de archivos generados:

```
$ make clean
```

### Ejecucion

Se puede ejecutar todo de manera separada abriendo cada terminal por programa

Para iniciar las entidades de control:
```bash
$ ./sri
[SRI] Servidor iniciado en el puerto 1234 y esperando conexiones...
```
```bash
$ ./senae
[SENAE] Servidor iniciado en el puerto 1235 y esperando conexiones...
```
```bash
$ ./supercia
[SUPERCIA] Servidor iniciado en el puerto 1236 y esperando conexiones...
```

Para iniciar el administrador del puerto:
```bash
$ ./puerto
[PUERTO] Servidor listo en el puerto 1237.
```

Y por último para ejecutar ECUAFAST y simular la llegada de 50 barcos:
```bash
$ ./ecuafast
La cantidad de barcos a simular es: 50
[BARCO 1] Mensaje generado: Carga=Porta Convencional, Peso=37402.00, Destino=Ecuador
[BARCO 1] Resultado: No necesita aforo.
[BARCO 2] Mensaje generado: Carga=PANAMAX, Peso=53863.00, Destino=USA
[BARCO 2] Resultado: No necesita aforo.
[BARCO 3] Mensaje generado: Carga=Porta Convencional, Peso=5535.00, Destino=Ecuador
[BARCO 3] Resultado: No necesita aforo.
....
```

También se puede ejecutar todo directamente desde un terminal utilizando un script que ejecutará todos los programas y simulará la llegada de 50 barcos:
```bash
$ ./run.sh
SRI iniciado con PID 12842
SENAE iniciado con PID 12843
SUPERCIA iniciado con PID 12844
Puerto iniciado con PID 12845
[SUPERCIA] Servidor iniciado en el puerto 1236 y esperando conexiones...
[SRI] Servidor iniciado en el puerto 1234 y esperando conexiones...
[PUERTO] Servidor listo en el puerto 1237.
[SENAE] Servidor iniciado en el puerto 1235 y esperando conexiones...
La cantidad de barcos a simular es: 50
[BARCO 1] Mensaje generado: Carga=PANAMAX, Peso=32424.00, Destino=Ecuador
[SRI] Cliente conectado.
[SRI] Mensaje recibido: Carga=PANAMAX, Peso=32424.00, Destino=Ecuador
[SRI] Promedio actual de peso: 1621.20
[SRI] Respuesta: PASS
[SRI] Cliente desconectado.
[SENAE] Cliente conectado.
[SENAE] Mensaje recibido: Carga=PANAMAX, Peso=32424.00, Destino=Ecuador
[SENAE] Tercer cuartil de pesos: 0.00
[SENAE] Respuesta: PASS
[SENAE] Cliente desconectado.
[SUPERCIA] Cliente conectado.
[SUPERCIA] Mensaje recibido: Carga=PANAMAX, Peso=32424.00, Destino=Ecuador
[SUPERCIA] Respuesta: PASS
[BARCO 1] Resultado: No necesita aforo.
[SUPERCIA] Cliente desconectado.
[PUERTO] Recibido: ID=1, Carga=PANAMAX, Peso=32424.00, Destino=Ecuador, Aforo=0
[PUERTO] Barco 1 agregado a la cola. Destino: Ecuador, Necesita aforo: 0
[PUERTO] Barco 1 atracando. Destino: Ecuador
[PUERTO] Barco 1 tiempo de desembarque: 2 segundos.
[BARCO 2] Mensaje generado: Carga=PANAMAX, Peso=44314.00, Destino=Europa
.....
```




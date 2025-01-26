#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 1235
#define BUFFER_SIZE 1024
#define NUM_PESOS 100

// Historial de los pesos recibidos para calcular el tercer cuartil
static double pesos_historial[NUM_PESOS] = {0};
static int indice_pesos = 0;

// Función para insertar un nuevo peso en el historial
void agregar_peso_historial(double peso) {
    pesos_historial[indice_pesos] = peso;
    indice_pesos = (indice_pesos + 1) % NUM_PESOS;
}

// Función para calcular el tercer cuartil de los pesos históricos
double calcular_tercer_cuartil() {
    double pesos_ordenados[NUM_PESOS];
    memcpy(pesos_ordenados, pesos_historial, sizeof(pesos_historial));

    // Ordenar los pesos
    for (int i = 0; i < NUM_PESOS - 1; i++) {
        for (int j = 0; j < NUM_PESOS - i - 1; j++) {
            if (pesos_ordenados[j] > pesos_ordenados[j + 1]) {
                double temp = pesos_ordenados[j];
                pesos_ordenados[j] = pesos_ordenados[j + 1];
                pesos_ordenados[j + 1] = temp;
            }
        }
    }

    // Calcular el tercer cuartil (75% de los datos)
    int posicion = (3 * NUM_PESOS) / 4;
    return pesos_ordenados[posicion];
}

// Función para manejar la conexión con un cliente
void *handle_client(void *arg) {
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("[SENAE] Cliente desconectado.\n");
            close(client_fd);
            break;
        }

        printf("[SENAE] Mensaje recibido: %s\n", buffer);

        // Extraer información del mensaje recibido
        char carga[256], destino[50];
        double peso;
        sscanf(buffer, "Carga=%[^,], Peso=%lf, Destino=%s", carga, &peso, destino);

        // Agregar el peso al historial y calcular el tercer cuartil
        agregar_peso_historial(peso);
        double tercer_cuartil = calcular_tercer_cuartil();
        printf("[SENAE] Tercer cuartil de pesos: %.2f\n", tercer_cuartil);

        // Evaluar si el barco necesita aforo
        char respuesta[BUFFER_SIZE];
        if (strcmp(carga, "PANAMAX") == 0 && peso >= tercer_cuartil &&
            (strcmp(destino, "Europa") == 0 || strcmp(destino, "USA") == 0)) {
            snprintf(respuesta, BUFFER_SIZE, "CHECK");
            printf("[SENAE] Respuesta: CHECK\n");
        } else {
            snprintf(respuesta, BUFFER_SIZE, "PASS");
            printf("[SENAE] Respuesta: PASS\n");
        }

        // Enviar respuesta al cliente
        send(client_fd, respuesta, strlen(respuesta), 0);
    }

    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Crear el socket del servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[SENAE] Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Asociar el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SENAE] Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 5) < 0) {
        perror("[SENAE] Error al escuchar");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SENAE] Servidor iniciado en el puerto %d y esperando conexiones...\n", PORT);

    // Aceptar conexiones entrantes
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("[SENAE] Error al aceptar conexión");
            continue;
        }

        printf("[SENAE] Cliente conectado.\n");

        // Crear un hilo para manejar al cliente
        pthread_t thread;
        int *pclient = malloc(sizeof(int));
        *pclient = client_fd;
        pthread_create(&thread, NULL, handle_client, pclient);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}

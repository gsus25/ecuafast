#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 1234
#define BUFFER_SIZE 1024
#define NUM_PROMEDIOS 20

// Historial de los últimos 20 pesos para calcular el promedio
static double promedio_pesos[NUM_PROMEDIOS] = {0};
static int indice_historial = 0;

// Función para actualizar el promedio dinámico
double actualizar_promedio(double nuevo_peso) {
    promedio_pesos[indice_historial] = nuevo_peso;
    indice_historial = (indice_historial + 1) % NUM_PROMEDIOS;

    double suma = 0;
    for (int i = 0; i < NUM_PROMEDIOS; i++) {
        suma += promedio_pesos[i];
    }
    return suma / NUM_PROMEDIOS;
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
            printf("[SRI] Cliente desconectado.\n");
            close(client_fd);
            break;
        }

        printf("[SRI] Mensaje recibido: %s\n", buffer);

        // Extraer información del mensaje recibido
        char carga[256], destino[50];
        double peso;
        sscanf(buffer, "Carga=%[^,], Peso=%lf, Destino=%s", carga, &peso, destino);

        // Actualizar promedio dinámico
        double promedio_actual = actualizar_promedio(peso);
        printf("[SRI] Promedio actual de peso: %.2f\n", promedio_actual);

        // Evaluar si el barco necesita aforo
        char respuesta[BUFFER_SIZE];
        if (strcmp(carga, "Porta Convencional") == 0 && peso > promedio_actual &&
            strcmp(destino, "Ecuador") == 0) {
            snprintf(respuesta, BUFFER_SIZE, "CHECK");
            printf("[SRI] Respuesta: CHECK\n");
        } else {
            snprintf(respuesta, BUFFER_SIZE, "PASS");
            printf("[SRI] Respuesta: PASS\n");
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
        perror("[SRI] Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Asociar el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SRI] Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 5) < 0) {
        perror("[SRI] Error al escuchar");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SRI] Servidor iniciado en el puerto %d y esperando conexiones...\n", PORT);

    // Aceptar conexiones entrantes
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("[SRI] Error al aceptar conexión");
            continue;
        }

        printf("[SRI] Cliente conectado.\n");

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

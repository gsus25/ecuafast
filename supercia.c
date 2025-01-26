#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 1236
#define BUFFER_SIZE 1024

// Función para determinar aleatoriamente si se debe aplicar CHECK
int determinar_check(const char *tipo_carga) {
    int probabilidad = 0;
    if (strcmp(tipo_carga, "Porta Convencional") == 0) {
        probabilidad = 30; // 30% para Porta Convencional
    } else if (strcmp(tipo_carga, "PANAMAX") == 0) {
        probabilidad = 50; // 50% para PANAMAX
    }
    return (rand() % 100) < probabilidad;
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
            printf("[SUPERCIA] Cliente desconectado.\n");
            close(client_fd);
            break;
        }

        printf("[SUPERCIA] Mensaje recibido: %s\n", buffer);

        // Extraer información del mensaje recibido
        char carga[256];
        sscanf(buffer, "Carga=%[^,]", carga);

        // Determinar aleatoriamente si se aplica CHECK
        char respuesta[BUFFER_SIZE];
        if (determinar_check(carga)) {
            snprintf(respuesta, BUFFER_SIZE, "CHECK");
            printf("[SUPERCIA] Respuesta: CHECK (probabilidad aplicada para %s)\n", carga);
        } else {
            snprintf(respuesta, BUFFER_SIZE, "PASS");
            printf("[SUPERCIA] Respuesta: PASS\n");
        }

        // Enviar respuesta al cliente
        send(client_fd, respuesta, strlen(respuesta), 0);
    }

    return NULL;
}

int main() {
    srand(time(NULL)); // Inicializar generador de números aleatorios

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Crear el socket del servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[SUPERCIA] Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Asociar el socket a la dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[SUPERCIA] Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 5) < 0) {
        perror("[SUPERCIA] Error al escuchar");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SUPERCIA] Servidor iniciado en el puerto %d y esperando conexiones...\n", PORT);

    // Aceptar conexiones entrantes
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("[SUPERCIA] Error al aceptar conexión");
            continue;
        }

        printf("[SUPERCIA] Cliente conectado.\n");

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

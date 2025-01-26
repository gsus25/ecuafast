#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BARCOS 100
#define CAPACIDAD_PUERTO 10
#define PROBABILIDAD_DANO 10 // 10%
#define BUFFER_SIZE 1024
#define PUERTO_PORT 1237

typedef struct Barco {
    int id;
    int necesita_aforo;
    char destino[50];
    struct Barco *siguiente;
} Barco;

Barco *inicio_cola = NULL;
Barco *final_cola = NULL;
int barcos_en_puerto = 0;
pthread_mutex_t mutex_cola = PTHREAD_MUTEX_INITIALIZER;

// Función para agregar un barco a la lista enlazada
void agregar_barco(int id, int necesita_aforo, const char *destino) {
    pthread_mutex_lock(&mutex_cola);

    Barco *nuevo = malloc(sizeof(Barco));
    if (!nuevo) {
        perror("[PUERTO] Error al asignar memoria para el barco");
        pthread_mutex_unlock(&mutex_cola);
        return;
    }

    nuevo->id = id;
    nuevo->necesita_aforo = necesita_aforo;
    strcpy(nuevo->destino, destino);
    nuevo->siguiente = NULL;

    // Insertar con prioridad
    if (!inicio_cola || (necesita_aforo && strcmp(destino, "Ecuador") != 0)) {
        nuevo->siguiente = inicio_cola;
        inicio_cola = nuevo;
        if (!final_cola) final_cola = nuevo;
    } else {
        final_cola->siguiente = nuevo;
        final_cola = nuevo;
    }

    printf("[PUERTO] Barco %d agregado a la cola. Destino: %s, Necesita aforo: %d\n", id, destino, necesita_aforo);
    pthread_mutex_unlock(&mutex_cola);
}

// Función para remover un barco dañado
void verificar_danos() {
    pthread_mutex_lock(&mutex_cola);

    if (inicio_cola && (rand() % 100 < PROBABILIDAD_DANO)) {
        Barco *danado = inicio_cola;
        inicio_cola = inicio_cola->siguiente;
        if (!inicio_cola) final_cola = NULL;

        printf("[PUERTO] Barco %d removido por daño en trayecto.\n", danado->id);
        free(danado);
    }

    pthread_mutex_unlock(&mutex_cola);
}

// Función para procesar el atraco de barcos
void procesar_atraco() {
    while (1) {
        pthread_mutex_lock(&mutex_cola);

        if (barcos_en_puerto < CAPACIDAD_PUERTO && inicio_cola) {
            Barco *barco = inicio_cola;
            inicio_cola = inicio_cola->siguiente;
            if (!inicio_cola) final_cola = NULL;

            barcos_en_puerto++;
            pthread_mutex_unlock(&mutex_cola);

            printf("[PUERTO] Barco %d atracando. Destino: %s\n", barco->id, barco->destino);

            // Calcular el tiempo de desembarque
            int tiempo = (strcmp(barco->destino, "Ecuador") == 0) ? 2 : 1;
            if (barco->necesita_aforo) tiempo *= 2;

            printf("[PUERTO] Barco %d tiempo de desembarque: %d segundos.\n", barco->id, tiempo);

            sleep(tiempo);
            printf("[PUERTO] Barco %d terminó el desembarque.\n", barco->id);

            free(barco);
            pthread_mutex_lock(&mutex_cola);
            barcos_en_puerto--;
        }
        pthread_mutex_unlock(&mutex_cola);

        verificar_danos();
        sleep(1);
    }
}

// Servidor para recibir barcos desde ECUAFast
void *server_puerto(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[PUERTO] Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PUERTO_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[PUERTO] Error al hacer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("[PUERTO] Error al escuchar conexiones");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[PUERTO] Servidor listo en el puerto %d.\n", PUERTO_PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("[PUERTO] Error al aceptar conexión");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(client_fd, buffer, BUFFER_SIZE, 0) <= 0) {
            perror("[PUERTO] Error al recibir datos");
            close(client_fd);
            continue;
        }

        printf("[PUERTO] Recibido: %s\n", buffer);

        int id, necesita_aforo;
        char destino[50];
        if (sscanf(buffer, "ID=%d, Carga=%*[^,], Peso=%*[^,], Destino=%[^,], Aforo=%d", &id, destino, &necesita_aforo) != 3) {
            fprintf(stderr, "[PUERTO] Error al parsear el mensaje: %s\n", buffer);
            close(client_fd);
            continue;
        }

        agregar_barco(id, necesita_aforo, destino);
        close(client_fd);
    }
}

int main() {
    pthread_t hilo_atraco, hilo_server;

    // Crear hilo para procesar atracos
    pthread_create(&hilo_atraco, NULL, (void *)procesar_atraco, NULL);

    // Crear hilo para servidor de comunicación con ECUAFast
    pthread_create(&hilo_server, NULL, (void *)server_puerto, NULL);

    pthread_join(hilo_atraco, NULL);
    pthread_join(hilo_server, NULL);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define SRI_PORT 1234
#define SENAE_PORT 1235
#define SUPERCIA_PORT 1236
#define PUERTO_PORT 1237
#define SERVER_IP "127.0.0.1"

// Función para conectarse a un servidor
int connect_to_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[ECUAFast] Error al crear socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[ECUAFast] Error al conectar al servidor");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Función para comunicar con un servidor
void communicate_with_server(int sockfd, const char *mensaje, char *respuesta) {
    char buffer[BUFFER_SIZE];
    send(sockfd, mensaje, strlen(mensaje), 0);

    memset(buffer, 0, BUFFER_SIZE);
    recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (respuesta != NULL) {
        strncpy(respuesta, buffer, BUFFER_SIZE - 1); // Copiar respuesta
    }
    close(sockfd);
}

// Función para generar un barco
void generar_barco(char *mensaje, char *tipo_carga, char *destino, double *peso) {
    const char *cargas[] = {"Porta Convencional", "PANAMAX"};
    const char *destinos[] = {"Ecuador", "Europa", "USA"};

    strcpy(tipo_carga, cargas[rand() % 2]);
    strcpy(destino, destinos[rand() % 3]);
    *peso = (rand() % 75001) + 5000; // Peso entre 5,000 y 80,000

    snprintf(mensaje, BUFFER_SIZE, "Carga=%s, Peso=%.2f, Destino=%s", tipo_carga, *peso, destino);
}

int main() {
    srand(time(NULL));

    int n_barcos = 50;
    printf("La cantidad de barcos a simular es: %d\n", n_barcos);
    
    for (int i = 0; i < n_barcos; i++) {
        char mensaje[BUFFER_SIZE], tipo_carga[50], destino[50];
        double peso;
        generar_barco(mensaje, tipo_carga, destino, &peso);

        printf("[BARCO %d] Mensaje generado: %s\n", i + 1, mensaje);

        // Consultar a las entidades
        char respuesta_SRI[BUFFER_SIZE] = "PASS", respuesta_SENAE[BUFFER_SIZE] = "PASS", respuesta_SUPERCIA[BUFFER_SIZE] = "PASS";

        int sri_sock = connect_to_server(SRI_PORT);
        if (sri_sock != -1) {
            communicate_with_server(sri_sock, mensaje, respuesta_SRI);
        }

        int senae_sock = connect_to_server(SENAE_PORT);
        if (senae_sock != -1) {
            communicate_with_server(senae_sock, mensaje, respuesta_SENAE);
        }

        int supercia_sock = connect_to_server(SUPERCIA_PORT);
        if (supercia_sock != -1) {
            communicate_with_server(supercia_sock, mensaje, respuesta_SUPERCIA);
        }

        // Decidir si necesita aforo
        int checks = 0;
        if (strcmp(respuesta_SRI, "CHECK") == 0) checks++;
        if (strcmp(respuesta_SENAE, "CHECK") == 0) checks++;
        if (strcmp(respuesta_SUPERCIA, "CHECK") == 0) checks++;

        int necesita_aforo = (checks >= 2);
        printf("[BARCO %d] Resultado: %s necesita aforo.\n", i + 1, necesita_aforo ? "Sí" : "No");

        // Enviar datos al puerto
        int puerto_sock = connect_to_server(PUERTO_PORT);
        if (puerto_sock != -1) {
            snprintf(mensaje, BUFFER_SIZE, "ID=%d, Carga=%s, Peso=%.2f, Destino=%s, Aforo=%d", i + 1, tipo_carga, peso, destino, necesita_aforo);
            communicate_with_server(puerto_sock, mensaje, NULL);
        } else {
            printf("[ECUAFast] Error: No se pudo conectar al puerto.\n");
        }

        usleep(500000); // Pausa de 500ms entre barcos
    }

    printf("[ECUAFast] Todos los barcos procesados.\n");
    return 0;
}

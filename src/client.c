#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORTA_SERVER 5000
#define IP_SERVER "127.0.0.1"
#define CLIENT_BUFFER_SIZE 1024

struct sockaddr_in server_address;

void inizializza_client(int*);
void comunica_con_server(int);

int main(void)
{
    int client_socket_fd;

    inizializza_client(&client_socket_fd);
    comunica_con_server(client_socket_fd);

    return 0;
}

void inizializza_client(int *client_socket_fd)
{
    /* creiamo la socket del client */
    if ((*client_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore client 'socket'");
    }

    /* configuriamo la struct contenente le informazioni sull'indirizzo del server al quale vogliamo connetterci */
    server_address.sin_family       = AF_INET;
    server_address.sin_port         = htons(PORTA_SERVER);
    server_address.sin_addr.s_addr  = inet_addr(IP_SERVER);

    /* ci connettiamo al server utilizzando le informazioni sul suo indirizzo appena configurato */
    if (connect(*client_socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("impossibile connettersi al server");
        exit(1);
    } else {
        printf("+++ Client connesso al server\n");
    }
}

void comunica_con_server(int client_socket_fd) {
    char*   client_buffer = malloc(CLIENT_BUFFER_SIZE * sizeof(char));
    ssize_t byte_letti;

    /* legge il contenuto inviato dal server */
    if ((byte_letti = read(client_socket_fd, client_buffer, CLIENT_BUFFER_SIZE)) < 0) {
        perror("errore client 'read'");
        exit(1);
    }

    client_buffer[strcspn(client_buffer, "\n")] = 0;
    printf("<<< %s [%zu - bytes]\n", client_buffer, byte_letti);

    /* chiudiamo la socket del client e liberiamo la memoria allocata dinamicamente */
    close(client_socket_fd);
    free(client_buffer);
}

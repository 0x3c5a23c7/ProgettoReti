#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORTA_SERVER 3000
#define IP_SERVER "127.0.0.1"
#define CLIENT_BUFFER_SIZE 1024
#define TEMP_BUFFER_SIZE 10

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
    char*   temp_buffer = malloc(CLIENT_BUFFER_SIZE * sizeof(char));
    char*   risposta_dopo_acquisto = malloc(CLIENT_BUFFER_SIZE * sizeof(char));
    ssize_t byte_letti;
    int     numero_biglietti = 0;

    /* legge il contenuto inviato dal server */
    /* ci informa sulla quantità di biglietti disponibili attualmente */
    if ((byte_letti = read(client_socket_fd, client_buffer, CLIENT_BUFFER_SIZE)) < 0) {
        perror("errore client 'read'");
        exit(1);
    }

    client_buffer[strcspn(client_buffer, "\n")] = 0;
    printf("<<< %s\n", client_buffer);
     
    /* ricaviamo il numero di biglietti disponibili */
    sscanf(client_buffer, "I biglietti disponibili sono: %d", &numero_biglietti);

    /** 
     * In base al numero di biglietti disponibili:
     * < 1 -> il client ha ricevuto il messaggio "Biglietti non disponibili" quindi deve terminare
     * > 0 -> c'è almeno 1 biglietto disponibile quindi il client procede con l'acquisto
     */
    
    /* verifico se il client ha ricevuto il messaggio di terminazione */
    if (strcmp(client_buffer, "Biglietti non disponibili") == 0) {
        printf("+++ Biglietti esauriti, connessione terminata");
        close(client_socket_fd);
        free(client_buffer);
        free(temp_buffer);
        free(risposta_dopo_acquisto);
        exit(1);
    }

    /* se non ha ricevuto il messaggio di terminazione allora effettua l'acquisto */
    printf("+++ Inserire la quantità di biglietti da acquistare: ");

    /* leggiamo il numero di biglietti da acquistare dallo stdin */
    fgets(temp_buffer, TEMP_BUFFER_SIZE, stdin);

    /* mandiamo al server la quantità di biglietti che vogliamo acquistare */
    if (send(client_socket_fd, temp_buffer, sizeof(temp_buffer), 0) == -1) {
        perror("errore client 'send'");
        exit(1);
    }

    /* leggiamo la risposta del server dopo che il client ha inviato la quantità di biglietti che vuole comprare */
    if (read(client_socket_fd, risposta_dopo_acquisto, CLIENT_BUFFER_SIZE) == -1) {
        perror("errore client 'read'");
        exit(1);
    }
    printf("%s\n", risposta_dopo_acquisto);

    /* chiudiamo la socket del client e liberiamo la memoria allocata dinamicamente */
    close(client_socket_fd);
    free(client_buffer);
    free(temp_buffer);
    free(risposta_dopo_acquisto);
}

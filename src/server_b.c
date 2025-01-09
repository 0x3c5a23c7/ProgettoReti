#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define PORTA 5000
#define NUMERO_BIGLIETTI 10
#define CLIENT_IP_ADDRESS_SIZE 20
#define CLIENT_BUFFER_MESS 100
#define SERVER_MESSAGE_SIZE 100 

struct sockaddr_in server_socket_address;

void inizializza_server(int*); 
void accetta_connessioni(int);
void* gestisci_client(void*);

int main(void)
{
    int server_socket_fd;

    inizializza_server(&server_socket_fd);
    accetta_connessioni(server_socket_fd);

    close(server_socket_fd);
    return 0;
}

void inizializza_server(int *server_socket_fd)
{
    if ((*server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("errore nell'inizializzazione della socket del server");
        exit(1);
    }

    server_socket_address.sin_family       = AF_INET;
    server_socket_address.sin_port         = htons(PORTA);
    server_socket_address.sin_addr.s_addr  = htonl(INADDR_ANY);

    if ((bind(*server_socket_fd, (struct sockaddr*)&server_socket_address, sizeof(server_socket_address))) == -1) {
        perror("errore server 'bind'");
        exit(1);
    }

    /* definiamo la quantità totale di connessioni che il server potrà accettare */
    if ((listen(*server_socket_fd, 100)) == -1) {
        perror("errore server 'listen'");
        exit(1);
    }
    printf("+++ Server in ascolto sulla porta: %d\n", PORTA);
}

void accetta_connessioni(int server_socket_fd)
{
    /* dichiarazione delle variabili necessarie per la gestione delle connessioni */
    int*                client_socket_fd = malloc(sizeof(int*));
    struct sockaddr_in  client_socket_address;
    socklen_t           client_socket_len = sizeof(client_socket_address);

    while(1) {
        if ((*client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_socket_address, &client_socket_len)) == -1) {
            perror("errore server 'accept'");
            /* nel caso in cui la connessione fallisca, il server libera la memoria e resta in attesa di altre connessioni */
            free(client_socket_fd);
            continue;
        }

        /* per ogni client creiamo un thread separato che lo gestisca */
        pthread_t thread;
        if (pthread_create(&thread, ((void*)0), gestisci_client, client_socket_fd) != 0) {
            perror("errore nella creazione del thread");
            /* se la creazione del thread fallisce allora il server libera la memoria e resta in attesa di altre connessioni */
            free(client_socket_fd);
            continue;
        }

        pthread_detach(thread);
    }

    close(*client_socket_fd);
}

void* gestisci_client (void* client_socket_fd_ptr) 
{
    /* otteniamo il valore puntato da client_socket_fd_ptr */
    int client_socket_fd = *(int*)client_socket_fd_ptr;
    free(client_socket_fd_ptr);

    /* variabili utili per contenere informazioni sull'indirizzo del client */
    char*               client_ip_address = malloc(CLIENT_IP_ADDRESS_SIZE * sizeof(char));
    struct sockaddr_in  client_socket_address;
    socklen_t           client_socket_addr_len = sizeof(client_socket_address);

    /* DEBUG messaggio da parte del server */
    char* messaggio_server = malloc(SERVER_MESSAGE_SIZE * sizeof(char));

    /* ricaviamo l'indirizzo del client in maniera leggibile */
    getpeername(client_socket_fd, (struct sockaddr*)&client_socket_address, &client_socket_addr_len);
    inet_ntop(AF_INET, (struct in_addr*)&client_socket_address.sin_addr, client_ip_address, client_socket_addr_len);
    printf("+++ Il server ha ricevuto una connessione da: %s\n", client_ip_address);

    if (NUMERO_BIGLIETTI > 0) {
        snprintf(messaggio_server, SERVER_MESSAGE_SIZE, "I biglietti sono disponibili: %d", NUMERO_BIGLIETTI); 
        if (send(client_socket_fd, messaggio_server, SERVER_MESSAGE_SIZE, 0) == -1) {
            perror("errore server 'send'");
            exit(1);
        }
    } else {
        messaggio_server = "Biglietti non disponibili";
        if (send(client_socket_fd, messaggio_server, strlen(messaggio_server), 0) == -1) {
            perror("errore server 'send'");
            exit(1);
        }
    }
    printf(">>> Il server ha inviato: %s [%zu - bytes]\n", messaggio_server, strlen(messaggio_server));
    
    return ((void*)0);
}

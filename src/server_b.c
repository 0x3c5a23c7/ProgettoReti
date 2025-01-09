#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define PORTA 3000
#define CLIENT_IP_ADDRESS_SIZE 20
#define CLIENT_BUFFER_MESS 100
#define CLIENT_TEMP_BUFFER 10
#define SERVER_MESSAGE_SIZE 100 

int biglietti_disponibili = 10;
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
    int*                client_socket_fd = malloc(sizeof(int));
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

    /* variabili utili per i biglietti comprati dal client */
    char*   temp_client_biglietti = malloc(CLIENT_TEMP_BUFFER * sizeof(char));
    int     biglietti_comprati = 0;

    /* messaggio da parte del server */
    char*   messaggio_server = malloc(SERVER_MESSAGE_SIZE * sizeof(char));

    /* controlliamo se la memoria è stata allocata correttamente */
    if (!messaggio_server || !client_ip_address || !temp_client_biglietti) {
        perror("server errore 'malloc'");
        close(client_socket_fd);
        exit(1);
    }

    /* ricaviamo l'indirizzo del client in maniera leggibile */
    getpeername(client_socket_fd, (struct sockaddr*)&client_socket_address, &client_socket_addr_len);
    inet_ntop(AF_INET, (struct in_addr*)&client_socket_address.sin_addr, client_ip_address, client_socket_addr_len);
    printf("+++ Il server ha ricevuto una connessione da: %s\n", client_ip_address);

    /**
     * In base alla quantità di biglietti disponibili: 
     * <= 0 -> biglietti terminati, termina il client chiudendo la sua socket
     * >    -> biglietti disponibili, gestisci l'acquisto del client e poi chiudi la sua socket 
     */
    if (biglietti_disponibili > 0) {
        snprintf(messaggio_server, SERVER_MESSAGE_SIZE, "I biglietti disponibili sono: %d", biglietti_disponibili); 
        if (send(client_socket_fd, messaggio_server, SERVER_MESSAGE_SIZE, 0) == -1) {
            perror("errore server 'send'");
            exit(1);
        }

        /// ACQUISTO DAL CLIENT, AGGIORNA
        
        /* leggiamo la risposta del client, esso manda al server la quantità di biglietti da acquistare */
        if (read(client_socket_fd, temp_client_biglietti, CLIENT_TEMP_BUFFER) == -1) {
            perror("errore server 'read'");
            exit(1);
        }
        
        /* controllo se il client ha mandato un input valido (solo numeri) */ 
        char* endptr;
        biglietti_comprati = strtol(temp_client_biglietti, &endptr, 10);

        /* se l'input non è valido termino il client */
        if ((*endptr != '\n' && *endptr != '\0') || biglietti_comprati < 1) {
            printf("!!! Il client ha inserito un valore non valido\n");
            snprintf(messaggio_server, SERVER_MESSAGE_SIZE, "\nInput non valido.");
            send(client_socket_fd, messaggio_server, strlen(messaggio_server), 0);
            close(client_socket_fd);
        } else {
            /* trasformiamo il numero di biglietti da stringa a intero */
            biglietti_comprati = atoi(temp_client_biglietti);

            if (biglietti_comprati <= biglietti_disponibili) {
                printf("<<< Il client ha deciso di acquistare: %d biglietto/i\n", biglietti_comprati);
                messaggio_server = "\nHai correttamente acquistato i/il biglietti/o";
                if (send(client_socket_fd, messaggio_server, strlen(messaggio_server), 0) == -1) {
                    perror("errore server 'send'");
                    exit(1);
                }
                close(client_socket_fd);

                /* calcolo biglietti disponibili dopo l'acquisto del client */
                biglietti_disponibili -= biglietti_comprati;
                printf("\nBiglietti disponibili attualmente: %d\n", biglietti_disponibili);
            } else {
                printf("!!! Il client ha chiesto più biglietti di quelli disponibili\n");
                messaggio_server = "\nSiamo spiacenti, impossibile acquistare la quantità di biglietti specificata";
                if (send(client_socket_fd, messaggio_server, strlen(messaggio_server), 0) == -1) {
                    perror("errore server 'send'");
                    exit(1);
                }
                close(client_socket_fd);
            }
        }
    } else {
        /* biglietti terminati, mandiamo al client il messaggio di terminazione */
        printf("!!! I biglietti sono terminati\n");
        messaggio_server = "Biglietti non disponibili";
        if (send(client_socket_fd, messaggio_server, strlen(messaggio_server), 0) == -1) {
            perror("errore server 'send'");
            close(client_socket_fd);
            exit(1);
        }
        close(client_socket_fd);
        pthread_exit(((void*)0));
    }

    return ((void*)0);
}

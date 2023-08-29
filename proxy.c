/*
 * This is the main program for the proxy, which receives connections for sending and receiving clients
 * both in binary and XML format. Many clients can be connected at the same time. The proxy implements
 * an event loop.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "xmlfile.h"
#include "connection.h"
#include "record.h"
#include "recordToFormat.h"
#include "recordFromFormat.h"

#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

/* This struct should contain the information that you want
 * keep for one connected client.
 */
struct Client
{
    int sock_fd;
    char ID;
    int is_xml;
};

typedef struct Client Client;

void handleClient(Client* client, Client* clients[], int *amount_of_clients, fd_set *sockets);
Client *find_client_from_sock(Client *clients[], int sock_fd);
void forwardMessage( Record* msg, int sock_fd, int is_xml);
int recordSplitter(char *buffer, Record *records[], int is_xml);
Client* get_client_from_id(char id, Client* clients[], int amount_of_clients);

void usage( char* cmd )
{
    fprintf( stderr, "Usage: %s <port>\n"
                     "       This is the proxy server. It takes as imput the port where it accepts connections\n"
                     "       from \"xmlSender\", \"binSender\" and \"anyReceiver\" applications.\n"
                     "       <port> - a 16-bit integer in host byte order identifying the proxy server's port\n"
                     "\n",
             cmd );
    exit( -1 );
}
/*
 * This function is called when a new connection is noticed on the server
 * socket.
 * The proxy accepts a new connection and creates the relevant data structures.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
int handleNewClient( int server_sock, Client *clients[], int *amount_of_clients, fd_set* sockets)
{
    char binary_or_xml[1];
    char id[1];
    Client *client = clients[*amount_of_clients];
    client->sock_fd = tcp_accept(server_sock);
    FD_SET(client->sock_fd, sockets);
    tcp_read(client->sock_fd, binary_or_xml, 1);
    tcp_read(client->sock_fd, id, 1);
    client->ID = id[0];
    if (binary_or_xml[0] == 'B') {
        client->is_xml = 0;
    } else {
        client->is_xml = 1;
    }
    clients[*amount_of_clients] = client;
    (*amount_of_clients)++;
    return client->sock_fd;
}

int recordSplitter(char *buffer, Record *records[], int is_xml) {
    int record_bytes = 0;
    int recnumb = 0;
    Record *converted_record;
    char *start_record;
    char *end_record;
    int read_binary = 0;

    while(1) {
        if (is_xml) {
            start_record = strstr(buffer += record_bytes, "<record>");
            end_record = strstr(buffer, "</record>");
            if (start_record == NULL || end_record == NULL) {
                break;
            }
            record_bytes = end_record - start_record + 10;
            char *record = malloc(record_bytes);
            strncpy(record, start_record, record_bytes);
            converted_record = XMLtoRecord(record, record_bytes, 0);
            free(record);
        } else {
            converted_record = BinaryToRecord(buffer, strlen(buffer), &record_bytes);
            read_binary++;
        }
        if (converted_record != NULL) {
            records[recnumb] = converted_record;
            recnumb++;
            printRecordAsXML(converted_record);
        }
        if (read_binary > 0) {
            break;
        }

    }
    return recnumb;
}

/*
 * This function is called when a connection is broken by one of the connecting
 * clients. Data structures are clean up and resources that are no longer needed
 * are released.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void removeClient( Client* client, fd_set *sockets)
{
    FD_CLR(client->sock_fd, sockets);
    close(client->sock_fd);
}

/*
 * This function is called when the proxy received enough data from a sending
 * client to create a Record. The 'dest' field of the Record determines the
 * client to which the proxy should send this Record.
 *
 * If no such client is connected to the proxy, the Record is discarded without
 * error. Resources are released as appropriate.
 *
 * If such a client is connected, this functions find the correct socket for
 * sending to that client, and determines if the Record must be converted to
 * XML format or to binary format for sendig to that client.
 *
 * It does then send the converted messages.
 * Finally, this function deletes the Record before returning.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
void forwardMessage( Record* msg, int sock_fd, int is_xml)
{
    int bufsize = 0;
    char *record;
    fprintf(stderr, "%i", sock_fd);
    if (is_xml) {
        record = recordToXML(msg, &bufsize);
    } else {
        record = recordToBinary(msg, &bufsize);
    }
    tcp_write_loop(sock_fd, record, bufsize);
    free(record);
}

/*
 * This function is called whenever activity is noticed on a connected socket,
 * and that socket is associated with a client. This can be sending client
 * or a receiving client.
 *30
 * The calling function finds the Client structure for the socket where acticity
 * has occurred and calls this function.
 *
 * If this function receives data that completes a record, it creates an internal
 * Record data structure on the heap and calls forwardMessage() with this Record.
 *
 * If this function notices that a client has disconnected, it calls removeClient()
 * to release the resources associated with it.
 *
 * *** The parameters and return values of this functions can be changed. ***
 */
Client* get_client_from_id(char id, Client* clients[], int amount_of_clients) {
    for (int i = 0; i < amount_of_clients; i++) {
        if (clients[i]->ID == id) {
            return clients[i];
        }
    }
    return NULL;
}



void handleClient(Client* client, Client* clients[], int *amount_of_clients, fd_set *sockets)
{
    Record *records[100];
    char *buffer = malloc(10000);
    memset(buffer, 0, 10000);
    int bytes_read = tcp_read(client->sock_fd, buffer, 5000);
    if (bytes_read == 0) {
        fprintf(stderr, "no bytes read!\n");
        removeClient(client, sockets);
        (*amount_of_clients)--;
    }
    int amount_of_records = recordSplitter(buffer, records, client->is_xml);

    for (int i = 0; i < amount_of_records; i++) {
        Client *receiver = get_client_from_id(records[i]->dest, clients, *amount_of_clients);
        if (receiver != NULL) {
            fprintf(stderr, "test");
            forwardMessage(records[i], receiver->sock_fd,receiver->is_xml);
        }
        deleteRecord(records[i]);
    }
    free(buffer);
}

int main( int argc, char* argv[] )
{
    int port;
    int server_sock;

    if( argc != 2 )
    {
        usage( argv[0] );
    }

    port = atoi( argv[1] );
    server_sock = tcp_create_and_listen( port );
    if( server_sock < 0 ) exit( -1 );

    fd_set sockets_set;
    FD_ZERO(&sockets_set);
    FD_SET(server_sock, &sockets_set);
    Client *clients[26];

    for (int i = 0; i < 26; i++) {
        clients[i] = calloc(1, sizeof(Client));
        if (clients[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for client %d\n", i);
            return -1;
        }
    }


    int amount_of_clients = 0;
    /*
     * The following part is the event loop of the proxy. It waits for new connections,
     * new data arriving on existing connection, and events that indicate that a client
     * has disconnected.
     *
     * This function uses handleNewClient() when activity is seen on the server socket
     * and handleClient() when activity is seen on the socket of an existing connection.
     *
     * The loops ends when no clients are connected any more.
     */
    int biggest_fd = server_sock;
    do
    {
        int active_socket = tcp_wait(&sockets_set, biggest_fd);
        if (active_socket == server_sock) {
            biggest_fd = handleNewClient(server_sock, clients, &amount_of_clients, &sockets_set);
            fprintf(stderr, "new client: %i\n", biggest_fd);
            FD_SET(biggest_fd, &sockets_set);
        } else {
            Client *client = find_client_from_sock(clients, active_socket);
            handleClient(client, clients, &amount_of_clients, &sockets_set);
        }
        biggest_fd = server_sock + amount_of_clients;

    }
    while(amount_of_clients > 0);

    for (int i = 0; i < 26; i++) {
        free(clients[i]);
    }
    tcp_close( server_sock );

    return 0;
}

Client *find_client_from_sock(Client *clients[], int sock_fd) {
    for (int i = 0; i < 26; i++) {
        if(clients[i] != NULL && clients[i]->sock_fd == sock_fd) {
            return clients[i];
        }
    }
    return NULL;
}
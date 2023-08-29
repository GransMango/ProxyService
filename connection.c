/*
 * This is a file that implements the operation on TCP sockets that are used by
 * all of the programs used in this assignment.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "connection.h"

int tcp_connect( char* hostname, int port )
{
    char string_port[5];
    snprintf(string_port, 5, "%d", port);
    struct addrinfo hints;
    struct addrinfo *results;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(hostname, string_port, &hints, &results) != 0) {
        fprintf(stderr, "Getaddrinfo failed.");
        exit(-2);
    }

    int sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    connect(sock, results->ai_addr, results->ai_addrlen);
    return sock;
}

int tcp_read( int sock, char* buffer, int n )
{
    int result_recv = recv(sock, buffer, n, 0);
    if (result_recv == -1) {
        perror("socket");
        return -1;
    } else if (result_recv == 0) {
        return 0;
    }
    return result_recv;
}

int tcp_write( int sock, char* buffer, int bytes )
{
    int sent_bytes = send(sock, buffer, bytes, 0);
    if (sent_bytes == -1) {
        perror("socket");
        return -1;
    } else if (sent_bytes == 0) {
        return 0;
    }
    return sent_bytes;
}

int tcp_write_loop( int sock, char* buffer, int bytes )
{
    int sent_bytes_total = 0;
    int sent_bytes = 0;
    while(sent_bytes_total < bytes) {
        sent_bytes = tcp_write(sock, buffer+sent_bytes_total, bytes);
        if (sent_bytes == -1) {
            return -1;
        }
        sent_bytes_total += sent_bytes;
    }
    return bytes;
}

void tcp_close( int sock )
{
    if (close(sock) == -1) {
        perror("close");
    }
}

int tcp_create_and_listen(int port) {
    char string_port[5];
    snprintf(string_port, 5, "%d", port);
    struct addrinfo hints;
    struct addrinfo *results = NULL; // Initialize to NULL
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_result = getaddrinfo(NULL, string_port, &hints, &results);
    if (getaddrinfo_result != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(getaddrinfo_result));
        return -1;
    }

    int sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if (sock == -1) {
        perror("Error creating socket");
        freeaddrinfo(results);
        return -1;
    }

    int bind_value = bind(sock, results->ai_addr, results->ai_addrlen);
    if (bind_value != 0) {
        perror("Error binding with socket");
        freeaddrinfo(results);
        close(sock);
        return -1;
    }

    listen(sock, 27);
    freeaddrinfo(results);
    return sock;
}


int tcp_accept( int server_sock )
{
    struct sockaddr_storage client_adress;

    socklen_t size_address = sizeof(client_adress);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_adress, &size_address);

    if (client_sock < 0) {
        fprintf(stderr, "Error accepting socket");
    }
    return client_sock;
}

int tcp_wait(fd_set* waiting_set, int wait_end) {
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        read_fds = *waiting_set;

        int result = select(wait_end + 1, &read_fds, NULL, NULL, NULL);
        if (result == -1) {
            perror("select");
            return -1;
        }
        for (int i = 0; i <= wait_end; i++) {
            if (FD_ISSET(i, &read_fds)) {
                return i;
            }
        }
    }
}



int tcp_wait_timeout( fd_set* waiting_set, int wait_end, int seconds ) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    read_fds = *waiting_set;

    int result = select(wait_end + 1, &read_fds, NULL, NULL, &tv);
    if (result == -1) {
        perror("select");
        return -1;
    }
    for (int i = 0; i <= wait_end; i++) {
        if (FD_ISSET(i, &read_fds)) {
            return i;
        }
    }
    return 0;
}


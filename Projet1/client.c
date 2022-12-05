#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include "utils.h"
#include <unistd.h>
#include <time.h>
#include "functions.c"
#include <sys/time.h>

// Initialization of global variables
char *server_addr = "127.0.0.1";
int key_size = 8;
int mean_rate = 1000;
int duration = 1;
int port = 8123;

/*
Function used to send a request to the server. It takes the socket, the index of the requested file
and the size of the key as arguments. It then fills a buffer of size 4 + 4 + key_sizeÂ² which will
be the content of the request. Finally it sends that buffer to the server.
*/
int send_request(int sockfd, int requested_file, int key_size)
{
    uint32_t buffer[key_size * key_size + 8];

    // Filling the buffer
    // Index and size of key :
    int requested = htonl(requested_file);
    int size = htonl(key_size);
    memcpy(buffer, &requested, 4);
    memcpy(buffer + 4, &size, 4);

    // Key :
    for (int i = 0; i < key_size * key_size; i++)
    {
        uint32_t temp = (uint32_t)rand() % 256;
        memcpy(buffer + 8 + i, &temp, 1);
    }

    // Sends the request
    write(sockfd, buffer, ((key_size * sizeof(uint32_t) * key_size) + 2 * sizeof(int)));

    return 0;
}

/*
This is here that we will try to establish the connection with the server, send the request,
wait for the encrypted file and then close the connection. It will also register the response time
and write to a text file.
*/
int main(int argc, char *argv[])
{
    // Initialization of some variables
    int opt;
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    srand(time(NULL));

    // Parse all arguments using a parse function (located in the functions.c file)
    parse_client_args(argc, argv, &key_size, &mean_rate, &duration, &server_addr, &port);

    // Try to connect the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed\n");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr);
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Connection with the server failed...\n");
        exit(0);
    }

    // Generates a random index between 1 and 1000
    int index = 1 + rand() % 999;

    // Starts a timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Sends the request
    send_request(sockfd, index, key_size);

    // Wait for the encrypted file sent by the server
    int *temp;
    recv(sockfd, temp, 0, 0);

    // Ends the timer
    gettimeofday(&end, NULL);

    // Computes elapsed time and writes it to a text file
    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    double elapsed_time = (double)micros / 1000000.0;
    FILE *f = fopen("results.txt", "a");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, " %f \n", elapsed_time);

    // Close connection and sockets, returns
    fclose(f);
    close(sockfd);
    return 0;
}

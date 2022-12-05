#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include "utils.h"
#include <unistd.h>
#include "functions.c"
#include <pthread.h>
#include <semaphore.h>
#define MAX 80
#define PORT 8080
#define SIZE 1024
#define BUFFERSIZE 512

// Initialization of global variables
int size_of_file = 128;
uint8_t ***files;
int stop = 0;
int port = 8080;
int nb_threads = 4;
int readercount = 0;
sem_t x, y;
pthread_t tid;

// Structure to handle the request sent by the client
struct arg_struct
{
    uint8_t buffer[BUFFERSIZE * BUFFERSIZE + 8];
    int socket;
} arguments;

/*
Handles the request sent by the client. It takes its content as argument (in the form of a structure).
Then it retrieves all the information using ntohl and memcopy and execute the encryption function
on the requested file using the key. Finally, it sends the encrypted file and closes the connection.
*/
void *handle_req(struct arg_struct *arguments)
{
    int index, key_size;
    uint8_t error_code = 0;
    uint8_t *key = malloc(BUFFERSIZE * BUFFERSIZE * sizeof(uint8_t));

    // Retrieving of index and key size
    memcpy(&index, arguments->buffer, 4);
    memcpy(&key_size, arguments->buffer + 4, 4);
    index = ntohl(index);
    key_size = ntohl(key_size);

    // Retrieving of key
    memcpy(key, arguments->buffer + 8, key_size * key_size);

    // Close the connection if stop == 1
    if (key_size == 0 && index == 0)
    {
        printf("Stop signal\n");
        stop = 1;
        return (void *)1;
    }

    // Checks if the packet is valid
    if (index > 999 || index <= 0 || (key_size == 0) || ((key_size & (key_size - 1)) != 0))
    {
        return (void *)-1;
    }

    // Encryption of the file
    uint8_t **encrypted = encryption(key, files[index], size_of_file, key_size);

    // Intialization of the response buffer (of size 1 + 4 + size of encrypted file)
    // Also writes the error code and file size
    uint8_t resp_buffer[5 + size_of_file * size_of_file];
    memcpy(resp_buffer, &error_code, 1);
    uint32_t real_size = size_of_file * size_of_file;
    memcpy(resp_buffer + 1, &real_size, 4);

    // Put the encrypted file on the buffer
    int start = 0;
    for (size_t i = 0; i < size_of_file; i++)
    {
        for (size_t j = 0; j < size_of_file; j++)
        {
            memcpy(resp_buffer + 5 + start++, &encrypted[i][j], 1);
        }
    }

    // Sends buffer back to client
    write(arguments->socket, resp_buffer, 5 + size_of_file * size_of_file);

    // Free everything and close connection
    for (int i = 0; i < size_of_file; i++)
    {
        free(encrypted[i]);
    }
    free(encrypted);
    close(arguments->socket);
    free(arguments);
    free(key);
    pthread_exit(NULL);
}

/*
Main function. Parse arguments, generates all the files, starts connection then enters in a loop
to chat with clients. Uses a pool of thread to handle each client (and their request, 1 per client).
Once all connections have been closed, closes the server socket and frees everything
*/
int main(int argc, char **argv)
{
    // Initialization of some variables
    int serverSocket, newConnection;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    pthread_t readerthreads[nb_threads];
    int i = 0;
    struct timeval timeout;
    timeout.tv_sec = 120;
    timeout.tv_usec = 0;

    // Parse all arguments using a parse function (located in the functions.c file)
    parse_server_args(argc, argv, &nb_threads, &size_of_file, &port);

    // Generates 1000 files
    files = generateFiles(size_of_file);

    sem_init(&x, 0, nb_threads);

    // Creation of the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        error("setsockopt failed\n");
    if (setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0)
        error("setsockopt failed\n");

    // Try to bind to the socket
    if ((bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    if (listen(serverSocket, 100) == 0)
    {
        printf("Listening new connection\n");
    }
    else
    {
        printf("Error while Listening\n");
    }

    // Function for chatting between client and server
    while (!stop)
    {
        addr_size = sizeof(serverStorage);
        // Take first connection in the queue
        newConnection = accept(serverSocket, (struct sockaddr *)&serverStorage, &addr_size);

        struct arg_struct *arguments = malloc(sizeof(struct arg_struct));
        arguments->socket = newConnection;

        // Receive a request
        recv(newConnection, arguments->buffer, BUFFERSIZE * BUFFERSIZE + 8, 0);

        // Creation of thread to handle client's request
        int res = pthread_create(&readerthreads[i++], NULL, (void *)handle_req, (void *)arguments);
        if (res != 0)
        {
            printf("Failed to create a new thread\n");
            return 0;
        }

        // Thread joining
        if (i >= nb_threads)
        {
            i = 0;
            while (i < nb_threads)
            {
                pthread_join(readerthreads[i++], NULL);
            }
            i = 0;
        }
    }

    // Close and free everything
    close(serverSocket);
    for (size_t i = 0; i < 1000; i++)
    {
        for (size_t j = 0; j < size_of_file; j++)
        {

            free(files[i][j]);
        }
        free(files[i]);
    }
    free(files);
    return 0;
}
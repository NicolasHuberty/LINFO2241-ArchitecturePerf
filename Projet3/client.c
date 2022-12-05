#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include "functions.c"
#include <sys/resource.h>
#include <pthread.h>
#include <sys/time.h>
#define ARRAY_TYPE float

//Define global variables
char *server_addr = "127.0.0.1";
int keysz = 128;
int rate = 100;
int duration = 10;
int port = 8000;
int npages = 1000;
struct sockaddr_in servaddr;
ARRAY_TYPE *key;
long sent_times[10000];
long receive_times[10000];

//retur the actual time in msec
long getts(){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return (current_time.tv_sec*1000000) + current_time.tv_usec;
}


void *rcv(void* r)
{
    //Transform the number of thread in int
    unsigned t = (unsigned)(intptr_t)r;

    //Connect the client to the server
    int sockfd,err;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information and connection
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    // Start watch
    sent_times[t] = getts();

    //printf("Send a new packet\n");

    // Send file id
    unsigned fileindex = htonl(rand() % npages);
    err = send(sockfd, &fileindex, 4, 0);
    if (err < 0)
    {
        perror("Sending of file index failed");
        exit(EXIT_FAILURE);
    }

    // Send key size
    int revkey = htonl(keysz);
    err = send(sockfd, &revkey, 4, 0);
    if (err < 0)
    {
        perror("Sending of key size failed");
        exit(EXIT_FAILURE);
    }

    // Update key values
    for (unsigned int i = 0; i < keysz * keysz; i++)
    {
        key[i] = (ARRAY_TYPE) rand();
    }

    // Send key
    err = send(sockfd, key, sizeof(ARRAY_TYPE) * keysz * keysz, 0);
    if (err < 0)
    {
        perror("Sending of key failed");
        exit(EXIT_FAILURE);
    }
    printf("Send a request\n");
    // Receive the packet from the server

    // Receive error bit
    unsigned char error;
    recv(sockfd, &error, 1, 0);
    printf("Receive an answer\n");

    // Receive crypted file
    unsigned filesz;
    recv(sockfd, &filesz, 4, 0);

    if (filesz > 0)
    {
        long int left = ntohl(filesz);
        //printf("Left val:%ld\n",left);
        ARRAY_TYPE buffer[65536];
        while (left > 0)
        {
            unsigned b = left;
            if (b > 65536)
                b = 65536;
            left -= recv(sockfd, &buffer, b, 0);
        }
    

        /*printf("Receive this crypted file:\n");
        for (int i = 0; i < 16; i++)
        {
            printf("%d ",buffer[i]);
        }*/
    }
        
    receive_times[t] = getts();

    close(sockfd);
    return (void *)0;
}

int main(int argc, char *argv[])
{
    pthread_t thread[duration*rate];
    srand(time(NULL));
    parse_client_args(argc, argv, &keysz, &rate, &duration, &server_addr, &port);

    // Key generation
    key = malloc(sizeof(ARRAY_TYPE) * keysz * keysz);


    for (int i = 0; i < duration*rate; i++)
    {
        
        pthread_create(&thread[i], NULL, rcv, (void *)(intptr_t)i);
    }


    for (int i = 0; i < duration*rate; i++)
    {
        pthread_join(thread[i],NULL);
    }
    
    long elapsed;
    for(int i = 0; i < duration*rate; i++)
    {
        elapsed = receive_times[i] - sent_times[i];
        char str[30];
        #if OPTIM
        sprintf(str,"resultssimd%d.txt",keysz);
        #else
        sprintf(str,"results%d.txt",keysz);
        #endif
        FILE *f = fopen(str, "a");
        if (f == NULL)
        {
            printf("Error opening file!\n");
            exit(1);
        }
        fprintf(f, " %ld \n", elapsed);

        // Close connection and sockets, returns
        fclose(f);    
    }
 
    return 0;
}
    
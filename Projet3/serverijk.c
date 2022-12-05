#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "functions.c"
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#define MAX 80
#define ARRAY_TYPE uint32_t


int nbytes = 1024;
uint32_t **pages;
int port = 8080;
int npages = 1000;
int client_sock;
int connection_handler(void *socket_desc)
{
    //printf("Handle new connection");
    // Get the socket descriptor
    int sockfd = (int)(intptr_t)socket_desc;
    int fileid,keysz;

    int tread = recv(sockfd, &fileid, 4, 0);
    printf("Receive file id: %d\n",ntohl(fileid));
    if (tread < 0)
    {
        perror("Reception of file index failed");
        exit(EXIT_FAILURE);
    }

    tread = recv(sockfd, &keysz, 4, 0);
    if (tread < 0)
    {
        perror("Reception of key size failed");
        exit(EXIT_FAILURE);
    }

    // Network byte order
    keysz = ntohl(keysz);
    fileid = ntohl(fileid);

    ARRAY_TYPE key[keysz * keysz];
    unsigned tot = keysz * keysz * sizeof(ARRAY_TYPE);

    unsigned done = 0;
    while (done < tot)
    {
        tread = recv(sockfd, key, tot - done, 0);
        if (tread < 0)
        {
            perror("Reception of key failed");
            exit(EXIT_FAILURE);
        }
        done += tread;
    }

    int nr = nbytes / keysz;
    ARRAY_TYPE *file = pages[fileid % npages];
    ARRAY_TYPE *crypted = calloc(nbytes * nbytes,sizeof(ARRAY_TYPE));

    // Check if the packet is valid
    if (fileid > 999 || fileid <= 0 ||(keysz == 0) || ((keysz & (keysz - 1)) != 0)){
        return -1;
    }

    // Compute sub-matrices
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    // Compute sub-matrices
    for (int i = 0; i < nr; i++)
    {
        int vstart = i * keysz;
        for (int j = 0; j < nr; j++)
        {
            int hstart = j * keysz;
            // Do the sub-matrix multiplication
            for (int ln = 0; ln < keysz; ln++)
            {
                int aline = (vstart + ln) * nbytes + hstart;
                for (int col = 0; col < keysz; col++)
                {
                    int tot = 0;
                    for (int k = 0; k < keysz; k++)
                    {
                        int vline = (vstart + k) * nbytes + hstart;
                        tot += key[ln * keysz + k] * file[vline + col];
                    }
                    crypted[aline + col] = tot;
                }
            }
        }
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU time used: %f\n", cpu_time_used);
    //print_data(key,file,crypted,nbytes,keysz);

    uint8_t err = 0;
    send(sockfd, &err, 1, MSG_NOSIGNAL);

    unsigned sz = htonl(nbytes * nbytes * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    
    send(sockfd, crypted, nbytes * nbytes * sizeof(ARRAY_TYPE), MSG_NOSIGNAL);


    //Free everything and close connection
    free(crypted);
    close(sockfd);
    return 0;
}

// Driver function
int main(int argc, char **argv){

    // Initialization
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t addr_size;
    parse_server_args(argc, argv, &nbytes, &port);

    // Files generation
    pages = malloc(sizeof(void*) * npages);
    for (int i = 0; i < npages; i++){
        pages[i] = malloc(sizeof(ARRAY_TYPE) * nbytes * nbytes);    
        for (unsigned j = 0; j < nbytes * nbytes; j++)
        {
            pages[i][j] = j;
        }
        
    }
   
    for (unsigned i = 0; i < nbytes * nbytes; i++)
        pages[0][i] = i;


    // Socket creation & binding
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    struct timeval timeout;      
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;

    // Feedback
    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
        printf("setsockopt failed\n");
    if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout,sizeof timeout) < 0)
        printf("setsockopt failed\n");
    if((bind(sockfd,(const struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
        printf("socket bind failed...\n");
        exit(0);
    }
    if(listen(sockfd,2000) == 0)
        printf("Listening new connection\n");
    else
        printf("Error while Listening\n");


    // Communication between client and server
    //addr_size = sizeof(servaddr);
    while ((client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*) &addr_size)))
    {
        if(client_sock < 0){
            return -1;
        }
        //printf("Connection accepted\n");
        connection_handler((void *)(intptr_t)client_sock);
    }

    //Close and free everything
    close(sockfd);
    close(client_sock);
    for (int i = 0; i < npages; i++)
    {
        free(pages[i]);    
    }
    free(pages);
    return 0;
}
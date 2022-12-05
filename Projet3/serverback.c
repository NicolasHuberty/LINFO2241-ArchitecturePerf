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
#define ARRAY_TYPE uint32_t

int nbytes = 1024;
int nthreads = 1;
ARRAY_TYPE **pages;
int port = 8000;
int npages = 10;
int client_sock;
ARRAY_TYPE *crypted;

int connection_handler(int sockfd)
{
    //printf("Receive a new packet\n");
    int fileid,keysz,err;

    // Receive the file id
    err = recv(sockfd, &fileid, 4, 0);
    if (err < 0)
    {
        perror("Reception of file index failed");
        exit(EXIT_FAILURE);
    }
    fileid = ntohl(fileid);
    //printf("Receive file id: %d\n", fileid);
    // Receive key size
    err = recv(sockfd, &keysz, 4, 0);
    if (err < 0)
    {
        perror("Reception of key size failed");
        exit(EXIT_FAILURE);
    }
    keysz = ntohl(keysz);
    //printf("key size: %d\n", keysz);
    // Check if the packet is valid
    if (fileid > 999  ||(keysz == 0) || ((keysz & (keysz - 1)) != 0)){
        
        printf("Receive a bad packet");
        exit(EXIT_FAILURE);
    }
    
    // Receive the key
    ARRAY_TYPE key[keysz * keysz];
    unsigned tot = keysz * keysz * sizeof(ARRAY_TYPE);
    unsigned done = 0;
    while (done < tot)
    {
        err = recv(sockfd, key, tot - done, 0);
        if (err < 0)
        {
            perror("Reception of key failed");
            exit(EXIT_FAILURE);
        }
        done += err;
    }

    // Update crypted to 0 and pick the file asked
    ARRAY_TYPE *file = pages[fileid % npages];
    for (int i = 0; i < nbytes*nbytes; i++)
        crypted[i] = 0;
    

    // Compute sub-matrices
    int nr = nbytes / keysz;
    for (int i1 = 0; i1 < nr; i1++)
    {
        int vstart = i1 * keysz;
        for (int j2 = 0; j2 < nr; j2++)
        {
            int hstart = j2 * keysz;
            // Do the sub-matrix multiplication
            int first_index = vstart*nbytes+hstart;
            for (int i = 0; i < keysz; i++)
            {
                for (int k = 0; k < keysz; k++)
                {
                    int r = key[i*keysz+k];
                    for (int j = 0; j < keysz; j++)
                    {
                        crypted[first_index + i*nbytes+j] += r * file[first_index + k*nbytes+j];
                    }                        
                }
                    
                
                
            }
            
        }
    }

    //printf("CPU time used: %f\n", cpu_time_used);

    //print_data(key,file,crypted,nbytes,keysz);
    //printf("Answer the request\n");
    // Send error bit
    uint8_t error = 0;
    send(sockfd, &error, 1, MSG_NOSIGNAL);

    //Send size of file
    unsigned sz = htonl(nbytes * nbytes * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    
    //Send the crypted file
    send(sockfd, crypted, nbytes * nbytes * sizeof(ARRAY_TYPE), MSG_NOSIGNAL);


    //Free everything and close connection
    close(sockfd);
    return 0;
}


int main(int argc, char **argv){

    // Initialization
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t addr_size;
    crypted = calloc(nbytes * nbytes,sizeof(ARRAY_TYPE));

    //Parse the arguments
    parse_server_args(argc, argv,&nthreads, &nbytes, &port);

    // Socket creation & binding
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if((bind(sockfd,(const struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
        printf("socket bind failed...\n");
        exit(0);
    }
    if(listen(sockfd,5000) == 0)
        printf("Listening new connection\n");
    else
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }


    // Accept the first client and malloc the pages
    //printf("Wait a new connection\n");
    client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*) &addr_size);
    //printf("Receive a new connection\n");
    pages = malloc(sizeof(void*) * npages);
    for (int i = 0; i < npages; i++){
        pages[i] = malloc(sizeof(ARRAY_TYPE) * nbytes * nbytes);    
        /*for (unsigned j = 0; j < nbytes * nbytes; j++)
        {
            pages[i][j] = j;
        }*/
    }
    for (unsigned i = 0; i < nbytes * nbytes; i++)
        pages[0][i] = i;

    //Handle the request
    connection_handler(client_sock);

    //Main loop
    while ((client_sock = accept(sockfd, (struct sockaddr *)&servaddr,(socklen_t*) &addr_size)))
    {
        if(client_sock < 0){
            return -1;
        }
        connection_handler(client_sock);
    }

    //Close and free everything
    close(sockfd);
    close(client_sock);
    for (int i = 0; i < npages; i++)
    {
        free(pages[i]);    
    }
    free(pages);
    free(crypted);
    return 0;
}
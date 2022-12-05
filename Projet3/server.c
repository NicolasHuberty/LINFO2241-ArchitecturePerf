#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "functions.c"
#include <x86intrin.h>

#define ARRAY_TYPE float

int nbytes = 1024;
int nthreads = 1;
ARRAY_TYPE **pages;
int port = 3006;
int npages = 10;
int client_sock;
//ARRAY_TYPE *crypted;
clock_t start,end;
double cpu_time_used;
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
    //for (int i = 0; i < nbytes*nbytes; i++)
        //crypted[i] = 0;
    

    // Compute sub-matrices
    ARRAY_TYPE crypt[nbytes*nbytes] __attribute__ ((aligned(32)));
    #if OPTIM
    
    ARRAY_TYPE file[nbytes*nbytes]  __attribute__ ((aligned(32)));//= pages[fileid % npages];
    memcpy(file, pages[0], nbytes*nbytes*sizeof(ARRAY_TYPE));// TO MODIFY pages[0] to pages[fileid % npages]
    // Compute sub-matrices
    int shift = -keysz;
    int key_shift = 0;
    int z = 0;
    for (int row = 0; row < nbytes; row++)
    {
        if(row % keysz == 0){
            shift += keysz;
        }
        key_shift = 0;
        for(int k = (row*keysz); k < (row*keysz)+keysz; k++){
            __m256 r = _mm256_set1_ps(key[k%(keysz*keysz)]);
            z = 0;
            for(int fnum = (shift * nbytes)+key_shift*nbytes; fnum < (shift * nbytes)+ nbytes + key_shift*nbytes; fnum+=8){
                __m256 a = _mm256_load_ps(file + fnum);
                __m256 b = _mm256_load_ps(crypt + ((row*nbytes)+z));
                b = _mm256_add_ps(b,_mm256_mul_ps(r,a));
                _mm256_store_ps(crypt+((row*nbytes)+ z),b);
                z += 8;
            }
            key_shift++;
        }
    }
    #else
    int shift = -keysz;
    int key_shift = 0;
    int z = 0, r = 0;
    for (int row = 0; row < nbytes; row++)
    {
        if(row % keysz == 0){
            shift += keysz;
        }
        key_shift = 0;
        for(int k = (row*keysz); k < (row*keysz)+keysz; k++){
            r = key[k%(keysz*keysz)];
            z = 0;
            for(int fnum = (shift * nbytes)+key_shift*nbytes; fnum < (shift * nbytes)+ nbytes + key_shift*nbytes; fnum++){
                crypt[(row*nbytes)+ z++] += r * pages[fileid%npages][fnum];
            }
            key_shift++;
        }
    }
    #endif

    //print_data(key,file,crypted,nbytes,keysz);
    // Send error bit
    uint8_t error = 0;
    send(sockfd, &error, 1, MSG_NOSIGNAL);

    //Send size of file
    unsigned sz = htonl(nbytes * nbytes * sizeof(ARRAY_TYPE));
    send(sockfd, &sz, 4, MSG_NOSIGNAL);
    
    //Send the crypted file
    send(sockfd, crypt, nbytes * nbytes * sizeof(ARRAY_TYPE), MSG_NOSIGNAL);


    //Free everything and close connection
    close(sockfd);
    return 0;
}


int main(int argc, char **argv){
    #if OPTIM
    printf("SIMD Version\n");
    #else
    printf("Non SIMD Version\n");
    #endif
    // Initialization
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t addr_size;
    //crypted = calloc(nbytes * nbytes,sizeof(ARRAY_TYPE));

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
    //for (unsigned i = 0; i < nbytes * nbytes; i++)
    //    pages[0][i] = i;

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
    //free(crypted);
    return 0;
}
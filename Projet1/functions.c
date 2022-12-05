#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <time.h>
#include <math.h>

/*
Generates all the files. Takes size as argument and creates 1000 files with a size of sizeÂ² bytes.
The files contains a matrix, each element is a uint8_t.
It returns a 3d array of size 1000 * size * size.
*/
uint8_t ***generateFiles(int size)
{
    // Initializaiton
    srand(time(NULL));
    uint8_t ***files;
    int i, j, k;

    // Allocation of memory using malloc
    files = malloc(1000 * sizeof(uint8_t **));
    for (i = 0; i < 1000; i++)
    {
        files[i] = malloc(sizeof(uint8_t *) * size);
        for (j = 0; j < size; j++)
        {
            files[i][j] = malloc(sizeof(uint8_t) * size);
        }
    }

    // Creation of the files
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < size; j++)
        {
            for (k = 0; k < size; k++)
            {
                // We generate a random number between 0 and 32 for each element of the matrix.
                uint8_t num = (rand() % 32);
                files[i][j][k] = (uint8_t)num;
            }
        }
    }
    return files;
}

/*
Computes the encryption of a file given a key (as a 1d array), the file itself and
both the key size and the file size. It is a naive matrix multiplication using for loops.
Returns the encrypted file as a 2d array of uint8_t elements.
*/
uint8_t **encryption(uint8_t *key_param, uint8_t **file, int file_size, int key_size)
{
    // Initializaiton of some important variables
    int n_block = file_size / key_size;
    int l, m, i, j, k, sum;

    // We parse the key given as an argument into a 2d matrix
    int key[(int)((key_size))][(int)(key_size)];
    for (int i = 0; i < (key_size); i++)
    {
        for (int j = 0; j < (key_size); j++)
        {
            key[i][j] = key_param[(int)(i * (key_size) + j)];
        }
    }

    // If the key size doesn't divide the file size, we return an error.
    if (file_size % key_size != 0)
    {
        fprintf(stderr, "Wrong key size: %d and file size: %d", key_size, file_size);
        exit(EXIT_FAILURE);
    }

    // Memory allocation for the encrypted file
    uint8_t **encrypted_file = malloc(file_size * sizeof(uint8_t *));
    for (int i = 0; i < file_size; i++)
    {
        encrypted_file[i] = malloc(sizeof(uint8_t) * file_size);
    }

    // Matrix multiplication using lots of for loops (2 for the "blocks", 2 more for each element
    // inside the blocks and one more to do the sum of multiplications)
    for (l = 0; l < n_block; l++)
    {
        for (m = 0; m < n_block; m++)
        {
            for (i = 0; i < key_size; i++)
            {
                for (j = 0; j < key_size; j++)
                {
                    sum = 0;
                    for (k = 0; k < key_size; k++)
                    {
                        sum += (key[i][k] * file[k + l * key_size][j + m * key_size]);
                    }
                    sum = sum % 256;
                    encrypted_file[i + l * key_size][j + m * key_size] = sum;
                }
            }
        }
    }
    return encrypted_file;
}


/*
Used to print the key, the requested file and the resulting encrypted file. It is optional and
used mostly for debugging purposes.
*/
void print_data(uint8_t *key, uint8_t **matrix, uint8_t **encrypt, int file_size, int key_size)
{
    printf("Key size:%d \t File size:%d\n", key_size, file_size);

    // Key printing
    printf("----------KEY----------\n");
    for (int i = 0; i < key_size; i++)
    {
        for (int j = 0; j < key_size; j++)
        {
            printf("%d ", key[i * key_size + j]);
        }
        printf("\n");
    }

    // File printing
    printf("\n--------FILE--------\n");
    for (int i = 0; i < file_size; i++)
    {
        for (int j = 0; j < file_size; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    // Encrypted file printing
    printf("-------ENCRYPTED FILE-------\n");
    for (int i = 0; i < file_size; i++)
    {
        for (int j = 0; j < file_size; j++)
        {
            printf("%d ", encrypt[i][j]);
        }
        printf("\n");
    }
}

/*
Parsing of the server arguments. Verifies that they are correct and assigns their values.
*/
void parse_server_args(int argc, char *argv[], int *j, int *s, int *p)
{
    int opt;
    while ((opt = getopt(argc, argv, "j:s:p:")) != -1)
    {
        switch (opt)
        {

        //Number of threads
        case 'j':
            *j = atoi(optarg);
            break;

        //File size
        case 's':
            if (atoi(optarg) && (!(atoi(optarg) & (atoi(optarg) - 1))))
            {
                *s = atoi(optarg);
            }
            else
            {
                printf("The file size must be a power of 2\n");
                exit(0);
            }
            break;

        //TCP port
        case 'p':
            *p = atoi(optarg);
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }
}

/*
Parsing of the client arguments. Verifies that they are correct and assigns their values.
*/
void parse_client_args(int argc, char *argv[], int *k, int *r, int *t, char **serv_addr, int *port)
{
    int opt;
    while ((opt = getopt(argc, argv, "k:r:t:")) != -1)
    {
        switch (opt)
        {

        //Key size
        case 'k':
            if (atoi(optarg) && (!(atoi(optarg) & (atoi(optarg) - 1))))
            {
                *k = atoi(optarg);
            }
            else
            {
                printf("The key size must be a power of 2\n");
                exit(0);
            }
            break;

        //Rate
        case 'r':
            *r = atoi(optarg);
            break;

        //Duration
        case 't':
            *t = atoi(optarg);
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    //Server address and TCP port
    for (; optind < argc; optind++)
    {
        *serv_addr = strtok(argv[optind], ":");
        *port = atoi(strtok(NULL, ":"));
    }
}
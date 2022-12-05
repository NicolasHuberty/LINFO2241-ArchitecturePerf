#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "functions.c"
#include <time.h>
#include <x86intrin.h>
#define ARRAY_TYPE float

int main(int argc, char** argv){
    ARRAY_TYPE crypted[16];
    ARRAY_TYPE file[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    ARRAY_TYPE key[4] = {1,2,3,4};
    ARRAY_TYPE answer[16] = {11,14,17,20,23,26,29,32,35,38,41,44,47,50,53,56};
    int MAX_DIM = 100;
    float a[MAX_DIM][MAX_DIM] __attribute__ ((aligned(16)));
    float b[MAX_DIM][MAX_DIM] __attribute__ ((aligned(16)));
    float c[MAX_DIM][MAX_DIM] __attribute__ ((aligned(16)));
    float d[MAX_DIM][MAX_DIM] __attribute__ ((aligned(16)));
    float e[MAX_DIM][MAX_DIM] __attribute__ ((aligned(16)));
    // Compute sub-matrices
    int keysz = 2;
    int nbytes = 4;
    int shift = -keysz;
    int key_shift = 0;
    int z = 0;
    for (int i = 0; i < 16; i++)
    {
        crypted[i] = 0;
    }
    
    for (int row = 0; row < nbytes; row++)
    {
        if(row % keysz == 0){
            shift += keysz;
        }
        key_shift = 0;
        for(int k = (row*keysz); k < (row*keysz)+keysz; k++){
            __m128 r = _mm_load1_ps(key + (k%(keysz*keysz)));
            z = 0;
            for(int fnum = (shift * nbytes)+key_shift*nbytes; fnum < (shift * nbytes)+ nbytes + key_shift*nbytes; fnum+=4){
                __m128 a = _mm_loadu_ps(file + fnum);
                __m128 b = _mm_loadu_ps(crypted + ((row*nbytes)+z));
                b = _mm_add_ps(b,_mm_mul_ps(r,a));
                _mm_storeu_ps(crypted+((row*nbytes)+ z++),b);
            }
            key_shift++;
        }
    }

/*
for(int i=0;i<MAX_DIM;i+=1){

      for(int j=0;j<MAX_DIM;j+=4){

        for(int k=0;k<MAX_DIM;k+=4){

          __m128 result = _mm_load_ps(&d[i][j]);

          __m128 a_line  = _mm_load_ps(&a[i][k]);

          __m128 b_line0 = _mm_load_ps(&b[k][j+0]);

          __m128 b_line1 = _mm_loadu_ps(&b[k][j+1]);

          __m128 b_line2 = _mm_loadu_ps(&b[k][j+2]);

          __m128 b_line3 = _mm_loadu_ps(&b[k][j+3]);

         result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a_line, a_line, 0x00), b_line0));
         result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a_line, a_line, 0x55), b_line1));
         result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a_line, a_line, 0xaa), b_line2));
         result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a_line, a_line, 0xff), b_line3));
         _mm_store_ps(&d[i][j],result);
        }
      }
    }*/
    printf("Crypted easy way file: \n");
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%f ",crypted[i*nbytes+j]);
        }printf("\n");
        
    }

   return 0;
}
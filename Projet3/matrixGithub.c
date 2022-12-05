#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h> /* where intrinsics are defined */
#include <omp.h>

void sgemm( int m, int n, float *A, float *C)
{   
    int e = m/4*4;
    
    /*{   
        for (int j=0; j<m; j++) {
        	
            for (int i =0; i<e; i+=4) {
    	        __m128 c = _mm_loadu_ps( C+i+m*j );
            	
            	for (int k=0;k<n;k++) {
            	    __m128 b = _mm_load1_ps( A + j + m*k );
            	    __m128 a  = _mm_loadu_ps( A+ i + m*k );
            	    
            	    c = _mm_add_ps( c, _mm_mul_ps( a, b ) );
            	}
            	
            	_mm_storeu_ps( C+i+m*j, c );
            }
        }
    }*/
     
        for (int j=0; j<m; j++) {
            
            for (int k=0;k<n;k++) {
                __m128 b = _mm_load1_ps( A + j + m*k );
                
                for (int i =0; i<e; i+=4) {
                    __m128 c = _mm_loadu_ps( C+i+m*j );
                    //__m128 a  = _mm_loadu_ps( A+i+m*k );

                    c = _mm_add_ps( c, _mm_mul_ps( _mm_loadu_ps( A+i+m*k ), b ) );
                    _mm_storeu_ps( C+i+m*j, c );
                }
            }
        }
    

    if (e != m) {
        for( int j = 0; j < n; j++ ) {
            int a1 = j*m;
            for( int k = 0; k < m; k++ ) {
                int a2 = k*m;
                for( int i = e; i < m; i++ ) {
                    *(C+i+a2) += *(A+i+a1) * (*(A+k+a1));
	        }
	    }
        }
    }
}

 void sgemm1( int m, int n)
{
    float A[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//(float*)malloc( m*n*sizeof(float) );
  float C[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//(float*)malloc( m*n*sizeof(float) );
  float crypted[16];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            crypted[i*m+j] = 0;
            for (int k = 0; k < n; k++) {
                crypted[i*m+j] += A[i*m+k] * C[k*m+j];
            }
        }
    }
    for (int i = 0; i < m*n; i++)
    {
        printf("%f ",crypted[i]);
    }
    printf("---------------------------------------\n");
    int e = m;
    float crypted2[16] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
    for (int j=0; j<m; j++) {
    
        for (int k=0;k<n;k++) {
            __m128 b = _mm_load1_ps( A + j + m*k );
            
            for (int i =0; i<e; i+=4) {
                __m128 c = _mm_loadu_ps( C+i+m*j );
                //__m128 a  = _mm_loadu_ps( A+i+m*k );
                if(k == 0){
                    c = _mm_mul_ps( _mm_loadu_ps( C+i+m*j ), b );
                }else{
                    c = _mm_add_ps( c, _mm_mul_ps( _mm_loadu_ps( C+i+m*j ), b ) );
                }
                _mm_storeu_ps( crypted2+j+m*k, c );
            }
        }
    }    
    for (int i = 0; i < m*n; i++)
    {
        printf("%f ",crypted2[i]);
    }
    printf("---------------------------------------\n");
    


}   
void matricial_product(float *matrixA, float *matrixB){}
int main( int argc, char **argv ) {
  int m =4;
  int n = 4,i;
  float A[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//(float*)malloc( m*n*sizeof(float) );
  float C[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//(float*)malloc( m*n*sizeof(float) );
  float *C1 = (float*)malloc( m*n*sizeof(float) );
  float *R1 = (float*)malloc( m*n*sizeof(float) );
  float *R2 = (float*)malloc( m*n*sizeof(float) );
  for( i = 0; i < m*m; i++ ) C1[i] = (float) 0.00;
  for( i = 0; i < m*m; i++ ) R1[i] = (float) 0.00;
  for( i = 0; i < m*m; i++ ) R2[i] = (float) 0.00;
  //sgemm(m, n, A, C);
  sgemm1(m, n);
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
        printf("%f ",R1[i*m+j]);
    }printf("\n");
    
  }
  

  /*for( i = 0; i < m*m; i++ ) {
      printf("%d\n", i);
      printf("C1: %f\n ",C1[i]);
      printf("C:%f\n ",C[i]);
      if( C1[i] != C[i]) {
        printf("Error!!!! Transpose does not result in correct answer!!\n");
          exit( -1 );
     }
  }*/
  free( C1);
  return 0;
  }
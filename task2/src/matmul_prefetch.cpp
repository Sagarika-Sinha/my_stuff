// matmul_prefetch.cpp
#include "matmul.h"
#include <xmmintrin.h>   //this is where _mm_prefetch n constants are declared
#include <cstring> 
#include <cstdlib> 

#ifndef PREFETCH_DISTANCE     //how many elements ahead we issue a prefetch for
#define PREFETCH_DISTANCE 8      //this parameter is sweep in the g++ command, default val 8
#endif

#ifndef PREFETCH_HINT   //for cache fill level 
#define PREFETCH_HINT _MM_HINT_T0   //default is into all cache levels(l1,l2,l3), can be overridden at compile time
#endif
static int get_prefetch_distance() {
    const char* env = std::getenv("PREFETCH_DISTANCE");
    return env ? std::atoi(env) : PREFETCH_DISTANCE;
}

static int get_prefetch_hint() {
    const char* env = std::getenv("PREFETCH_HINT");
    if (!env) return _MM_HINT_T0;
    if (std::strcmp(env, "T1") == 0)  return _MM_HINT_T1;
    if (std::strcmp(env, "T2") == 0)  return _MM_HINT_T2;
    if (std::strcmp(env, "NTA") == 0) return _MM_HINT_NTA;
    return _MM_HINT_T0;  // default / "T0"
}

//a,b : inputs, c: output buffer being written to, m,n,k: logical matrix dims, lda,ldb,ldc: memory strides
void matmul_prefetch(const float* A,const float* B,float* C,int M,int N,int K,int lda,int ldb,int ldc){
    for (int i=0;i<M;++i){ //looping over each row i of c
        const float* a=A+static_cast<long>(i)*lda;  //pointer to the start of row i of a

        for (int j=0;j<N;++j){  //loop over each col j of op(each row of b)
            const float* b=B+static_cast<long>(j)*ldb;   //pointer to the start of row j of b

            
            if (j+1<N){ //before starting dop product for the current j, issuing prefectch for the next row of b(j+1)
                _mm_prefetch((const char*)(B+static_cast<long>(j+1)*ldb),PREFETCH_HINT);   //_mm_prefetcg takes a const char* hence typecasting
            }

            float acc=0.0f;  //accumulator for . product bw a and b
            int p=0;
            for (;p<K;++p){    //walking along both a and b for K elems 
                if (p+PREFETCH_DISTANCE<K){   //prefetch requests
                    _mm_prefetch((const char*)&a[p+PREFETCH_DISTANCE],PREFETCH_HINT);
                    _mm_prefetch((const char*)&b[p+PREFETCH_DISTANCE],PREFETCH_HINT);
                }
                acc+=a[p]*b[p];
            }
            C[static_cast<long>(i)*ldc+j]=acc;  //writing op
        }
    }
}

//in the naive code, b is accessed row wise like a so it is
//computing c=abT

//lda,ldb,dc are the leading dims of a,b,c. if matrix is stored
//with no padding, the leading dim just equals the row length
//lda=k, ldb=k, ldc=n if no padding
//why ld vars?? cause sometimes we want a to be a submatrix
//from a bigger matrix,
//a=A+i*lda jumps ti the start of the correct row,skipping padding of prev rows
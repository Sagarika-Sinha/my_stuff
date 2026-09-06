// matmul_simd.cpp
#include "matmul.h"
#include <immintrin.h>   
#include <cstdlib>      
#include <cstring>     
//__m256 reg holds 8 floats, but we need a single scalar number, so this collapes those 8 vals down to 1 via shiffle and add steps.
static inline float hsum256_ps(__m256 v) {
    __m128 lo=_mm256_castps256_ps128(v);      // lower 128 bits extracted
    __m128 hi=_mm256_extractf128_ps(v,1);    // upper 128 bits extracted
    __m128 sum4=_mm_add_ps(lo,hi);           // adds the low 4 floats to the high 4 floats ele wise producing 4 partial sums like v0+v4, v1+v5 etc
    __m128 shuf=_mm_movehdup_ps(sum4);        // duplicates the high ele so [a,b,c,d] becomes [b,b,d,d]
    __m128 sums2=_mm_add_ps(sum4,shuf);  //[a,b,c,d]+[b,b,d,d]=[a+b,2b,c+d,2d], we want a+b and c+d
    shuf=_mm_movehl_ps(shuf,sums2);        //moves high half of one reg to the low half of the result reg, shuffling so that val at pos2 (c+d) is  in the low lane
    __m128 result=_mm_add_ss(sums2,shuf);   //adds the lowest single scalar float of each reg so that a+b+c+d is in th elowest lane of the result, other 3 lanes untouched.
    return _mm_cvtss_f32(result);    //extractes value in lowest lanes of the simd reg as a float
}

//same thing for 4 floats
static inline float hsum128_ps(__m128 v) {
    __m128 shuf=_mm_movehdup_ps(v);
    __m128 sums=_mm_add_ps(v,shuf);
    shuf=_mm_movehl_ps(shuf,sums);
    __m128 result=_mm_add_ss(sums,shuf);
    return _mm_cvtss_f32(result);
}


static float dot_avx256(const float* a,const float* b,int K) {
    __m256 acc_vec=_mm256_setzero_ps();  // holds 8 running partial sums in parallel so lane 0 hols a[0]*b[0]+a[8]*b[8]+ etc
    int p=0;
    for (;p+8<=K;p+=8) {   //loop over K in strides of 8
        __m256 va=_mm256_loadu_ps(a+p);   //loads 8 consec floats from a and b, unaligned load
        __m256 vb=_mm256_loadu_ps(b+p);
        acc_vec=_mm256_add_ps(acc_vec,_mm256_mul_ps(va,vb));   //multiplies the two 8 wide vectors ele wise for each of the 8 lines, then adds res to the runnning acc also ele wise
    }
    float acc=hsum256_ps(acc_vec);    //collapses the 8 parallel partial sums in acc_vec down into a single scalar total
    for (;p<K;++p) acc+=a[p]*b[p];  // remainder (K not divisible by 8)
    return acc; 
}

//same thing for 128 bit 4 bit wide
static float dot_sse128(const float* a,const float* b,int K) {
    __m128 acc_vec=_mm_setzero_ps();
    int p=0;
    for (;p+4<= K;p+=4) {
        __m128 va=_mm_loadu_ps(a+p);
        __m128 vb=_mm_loadu_ps(b+p);
        acc_vec=_mm_add_ps(acc_vec,_mm_mul_ps(va,vb));
    }
    float acc=hsum128_ps(acc_vec);
    for (;p<K;++p) acc+=a[p]*b[p];  // remainder
    return acc;
}

#ifdef __AVX512F__
// Dot product using 512-bit (AVX-512) SIMD,16 floats per step.
static float dot_avx512(const float* a,const float* b,int K) {
    __m512 acc_vec=_mm512_setzero_ps();
    int p=0;
    for (;p+16<= K;p+=16) {
        __m512 va=_mm512_loadu_ps(a+p);
        __m512 vb=_mm512_loadu_ps(b+p);
        acc_vec=_mm512_fmadd_ps(va,vb,acc_vec); // fused multiply-add
    }
    float acc=_mm512_reduce_add_ps(acc_vec);
    for (;p<K;++p) acc+=a[p]*b[p];  // remainder
    return acc;
}
#endif

// Reads SIMD_WIDTH env var ("128"/"256"/"512") at runtime;defaults to 256.
// This lets you sweep widths without touching matmul.h's fixed signature.
static int get_simd_width() {
    const char* env=std::getenv("SIMD_WIDTH");
    if (!env) return 256;
    return std::atoi(env);
}

void matmul_simd(const float* A,const float* B,float* C,
                  int M,int N,int K,int lda,int ldb,int ldc) {
    int width=get_simd_width();

    for (int i=0;i<M;++i) {  // loop over op rows
        const float* a=A+static_cast<long>(i)*lda;   //pointer to the start of the row of a

        for (int j=0;j<N;++j) {   //loop over op cols
            const float* b=B+static_cast<long>(j)*ldb;

            float acc;
#ifdef __AVX512F__
            if (width == 512) {
                acc=dot_avx512(a,b,K);
            } else
#endif
            if (width == 256) {
                acc=dot_avx256(a,b,K);
            } else {
                acc=dot_sse128(a,b,K);  // covers width == 128 and any fallback
            }

            C[static_cast<long>(i)*ldc+j]=acc;
        }
    }
}
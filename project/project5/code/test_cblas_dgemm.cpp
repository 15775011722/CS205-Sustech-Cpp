#pragma GCC optimize(2)

// #include <cblas.h>
#include "test_cblas_dgemm.hpp"
#include <stdint.h>
#include <assert.h>
#include <immintrin.h>
#include <omp.h>
#include <iostream>
#include <chrono>
#include <string.h>
using namespace std;

//  注：这个值得根据read_file的buf长度以及浮点数整数小数保存范围来决定
#define LINE_MAX_NUM 70 // 每个浮点数有6个整数位，6个小数位，加小数点和空格=14，1024/14≈73,所以取71,循环从0开始再-1
#define LINE_MAX 1024
#define MAT_LENGTH 2048
#define SEPARATOR " "
#define FILE_A "A_128.txt"
#define FILE_B "B_128.txt"
#define FILE_C "C_128.txt"
#define ALPHA 1
#define BETA 1
#define RANGE 1000000.0



void set_item(OPENBLAS_CONST char TX, 
    double *X, OPENBLAS_CONST blasint ld, OPENBLAS_CONST double value, 
    OPENBLAS_CONST long long x, OPENBLAS_CONST long long y)
{
    if(TX == 'N')
        X[x*ld+y] += value;
    else
        X[y*ld+x] += value;
}

OPENBLAS_CONST double *get_item(OPENBLAS_CONST char TX, 
    OPENBLAS_CONST double *X, OPENBLAS_CONST blasint ld, 
    OPENBLAS_CONST long long x, OPENBLAS_CONST long long y)
{
    OPENBLAS_CONST double *p = X;
    if(TX == 'N')
        p += x*ld+y;
    else
        p += y*ld+x;
    return p;
}

// Row: A:no trans, B:trans; Col A:trans, B:no trans
void mat_mul_quick(OPENBLAS_CONST char TC, OPENBLAS_CONST char TA,  
    OPENBLAS_CONST char TB, OPENBLAS_CONST double alpha, 
    OPENBLAS_CONST double *A, OPENBLAS_CONST double *B, double *C, 
    OPENBLAS_CONST blasint M, OPENBLAS_CONST blasint N, OPENBLAS_CONST blasint K, 
    OPENBLAS_CONST blasint lda, OPENBLAS_CONST blasint ldb, OPENBLAS_CONST blasint ldc)
{
    long long cycle1 = M;
    long long cycle2 = N;
    long long cycle3 = K;
    long long remainder = (K >> 2) << 2; // 除4再乘4，相当于减去余数
    // #ifdef __AVX512F___
    __m256d a;
    __m256d b;
    __m256d c;
    __m256d _alpha = _mm256_set1_pd(alpha);
    double result[4] = {0};
    for(long long i = 0; i < cycle1; i++)
    {
        for(long long j = 0; j < cycle2; j++)
        {
            if(remainder != 0)
            {
                // #pragma omp parallel for schedule(4)
                for (long long k = 0, length=cycle3-4; k <=length; k+=4)
                {
                    a = _mm256_load_pd(A + k);
                    b = _mm256_load_pd(B + k);
                    a = _mm256_mul_pd(a, _alpha);
                    c = _mm256_mul_pd(a, b);
                    _mm256_storeu_pd(result, c); 
                    set_item(TC, C, ldc, result[0]+result[1]+result[2]+result[3], i, j);
                }
            }
            for (long long k = remainder; k < cycle3; k++)
            {
                // set_item(Order, C, ldc, alpha*A[i*lda+k]*B[j*ldb+k], i, j);
                set_item(TC, C, ldc, 
                alpha*(*get_item(TA, A, lda, i, k))*(*get_item(TB, B, ldb, k, j)), i, j);
            }
        }
    }
}

void mat_mul(OPENBLAS_CONST char TC, OPENBLAS_CONST char TA, 
    OPENBLAS_CONST char TB, OPENBLAS_CONST double alpha, OPENBLAS_CONST double beta, 
    OPENBLAS_CONST blasint M, OPENBLAS_CONST blasint N, OPENBLAS_CONST blasint K, 
    OPENBLAS_CONST double *A, OPENBLAS_CONST blasint lda, 
	OPENBLAS_CONST double *B, OPENBLAS_CONST blasint ldb, 
    double *C, OPENBLAS_CONST blasint ldc)
{
    // 先把β*C给计算了
    int size = M*N;
    int remainder = (size >> 2) << 2; // 除4再乘4，相当于减去余数
    __m256d c;
    __m256d _beta = _mm256_set1_pd(beta);
    // #pragma omp parallel for schedule(4)
    for (int k = 0, length=size-4; k <= length; k+=4)
    {
        c = _mm256_load_pd(C + k);
        c = _mm256_mul_pd(c, _beta);
        _mm256_storeu_pd(C + k, c);
    }
    for (int k = remainder; k < size; k++)
    {
        C[k] *= beta;
    }
    if(TA=='N' && TB=='T') {
        mat_mul_quick(TC, TA, TB, alpha, A, B, C, M, N, K, lda, ldb, ldc);
        return;
    }

    int cycle1 = M;
    int cycle2 = N;
    int cycle3 = K;
    remainder = (K >> 3) << 3; // 除4再乘4，相当于减去余数
    double result = 0;
    for(int i = 0; i < cycle1; i++)
    {
        for(int j = 0; j < cycle2; j++)
        {
            // #pragma omp parallel for schedule(4)
            for (long long k = 0, length = cycle3-8; k<=length; k+=8)
            {
                result = 0;
                result += alpha*(*get_item(TA, A, lda, i, k))*(*get_item(TB, B, ldb, k, j));
                result += alpha*(*get_item(TA, A, lda, i, k+1))*(*get_item(TB, B, ldb, k+1, j));
                result += alpha*(*get_item(TA, A, lda, i, k+2))*(*get_item(TB, B, ldb, k+2, j));
                result += alpha*(*get_item(TA, A, lda, i, k+3))*(*get_item(TB, B, ldb, k+3, j));
                result += alpha*(*get_item(TA, A, lda, i, k+4))*(*get_item(TB, B, ldb, k+4, j));
                result += alpha*(*get_item(TA, A, lda, i, k+5))*(*get_item(TB, B, ldb, k+5, j));
                result += alpha*(*get_item(TA, A, lda, i, k+6))*(*get_item(TB, B, ldb, k+6, j));
                result += alpha*(*get_item(TA, A, lda, i, k+7))*(*get_item(TB, B, ldb, k+7, j));
                set_item(TC, C, ldc, result, i, j);
            }
            for (long long k = remainder; k < cycle3; k++)
            {
                // set_item(Order, C, ldc, alpha*A[i*lda+k]*B[j*ldb+k], i, j);
                set_item(TC, C, ldc, 
                alpha*(*get_item(TA, A, lda, i, k))*(*get_item(TB, B, ldb, k, j)), i, j);
            }
        }
    }
}

void cblas_dgemm(OPENBLAS_CONST enum CBLAS_ORDER Order, OPENBLAS_CONST enum CBLAS_TRANSPOSE TransA, OPENBLAS_CONST enum CBLAS_TRANSPOSE TransB, OPENBLAS_CONST blasint M, OPENBLAS_CONST blasint N, OPENBLAS_CONST blasint K,
		 OPENBLAS_CONST double alpha, OPENBLAS_CONST double *A, OPENBLAS_CONST blasint lda, OPENBLAS_CONST double *B, OPENBLAS_CONST blasint ldb, OPENBLAS_CONST double beta, double *C, OPENBLAS_CONST blasint ldc)
{
    assert(Order >= CblasRowMajor);
    assert(Order <= CblasColMajor);
    assert(TransA >= CblasNoTrans);
    assert(TransA <= CblasConjNoTrans);
    assert(TransB >= CblasNoTrans);
    assert(TransB <= CblasConjNoTrans);
    char TA, TB, TC;  // T:按列展开  N:按行展开
    TC = (Order == CblasRowMajor) ? 'N' : 'T';
    if(Order == CblasRowMajor)
    {
        TA = (TransA == CblasTrans || TransA == CblasConjTrans) ? 'T' : 'N';
        TB = (TransB == CblasTrans || TransB == CblasConjTrans) ? 'T' : 'N';
    }
    else
    {
        TA = (TransA==CblasNoTrans || TransA==CblasConjNoTrans) ? 'T' : 'N';
        TB = (TransB==CblasNoTrans || TransB==CblasConjNoTrans) ? 'T' : 'N';
    }
    assert(lda >= ((TA=='T') ? M : K));
    assert(ldb >= ((TB=='T') ? K : N));
    assert(ldc >= ((TC=='T') ? M : N));
    mat_mul(TC, TA, TB, alpha, beta, M, N, K, A, lda, B, ldb, C, ldc);
    // mat_mul_quick(TC, TA, TB, alpha, A, B, C, M, N, K, lda, ldb, ldc);
}

int write_file(const char* path, OPENBLAS_CONST double *array, long size) {
    FILE *fp = fopen(path, "w");
    if(fp == NULL) {
		printf("文件读取失败。\n");
		return -1;
	}
    for (long i = 0; i < size; i++) {
        fprintf(fp, "%lf ", *array);
        if ((i % LINE_MAX_NUM) == 0) {
            fprintf(fp, "\n");
        }
        array++;
    }
	fclose(fp);
    return 1;
}

int read_file(const char *path, double *array) {
    FILE *fp = fopen(path, "r");
    char* temp;
    if(fp == NULL) {
		printf("文件读取失败。\n");
		return 1;
	}
    char buf[LINE_MAX] = {0};
    float tmp = 0.0;
    size_t line_num = 0, line_len = 0;
    // 读行
    while (fgets(buf, LINE_MAX, fp)) {
        line_len = strlen(buf);
        // 去除末尾换行
        if (buf[line_len-1] == '\n')
            buf[--line_len] = '\0';
        if(buf[line_len-1] == '\r')
            buf[--line_len] = '\0';
        // 长度为0，空行
        if (line_len == 0) 
            continue;
        // 分割字符串
        temp = strtok(buf, SEPARATOR);
        while (temp) {
            if(sizeof(temp) < sizeof(char)) 
                continue;
            tmp = atof(temp);  //  char*转float
            *array = (double)tmp;
            array++;
            temp = strtok(NULL, SEPARATOR);
        }
        memset(buf, 0, sizeof(buf));  // 清空缓存
    }
	fclose(fp);
    return 0;
}

void generatingRandomNumber(int size, double *array) {
    for (int i = 0; i < size; i++) {
        array[i] = ((float)rand()/(float)(RAND_MAX)) * RANGE;
    }
}

int main()
{
    // double A[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};         
    // double B[4] = {1.0,2.0,1.0,-3.0};  
    // double C[6] = {.5,.5,.5,.5,.5,.5}; 
    // cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans,3,2,2,1,A,2,B,2,1,C,2);
    // for(int i=0; i<6; i++)
    //     printf("%lf ", C[i]);
    // printf("\n");

    // double A[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};         
    // double B[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};  
    // double C[9] = {.5,.5,.5,.5,.5,.5,.5,.5,.5}; 
    // std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now(); 
    // cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans,3,3,2,1,A, 3, B, 3,2,C,3);
    // std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now(); 
    // std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start); 
    // std::cout << "It took me " << time_span.count() << " seconds." << std::endl;
    // for(int i=0; i<9; i++)
    //     printf("%lf ", C[i]);
    // printf("\n");

    // double A[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    // double B[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    // double C[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    // double D[9] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    // double E[16] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
    // double F[4] = {0, 0, 0, 0};
    // double G[9] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    // cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, 3, 3, 2, 1, A, 2, B, 2, 2, D, 3);

    // for (int i = 0; i < 3; i++)
    // {
    //     for (int j = 0; j < 3; j++)
    //         printf("%lf ", D[i * 3 + j]);
    //     printf("\n");
    // }
    // printf("\n");
    double *A = new double[MAT_LENGTH*MAT_LENGTH]; 
    double *B = new double[MAT_LENGTH*MAT_LENGTH]; 
    double *C = new double[MAT_LENGTH*MAT_LENGTH]; 
    generatingRandomNumber(MAT_LENGTH*MAT_LENGTH, A);
    generatingRandomNumber(MAT_LENGTH*MAT_LENGTH, B);
    generatingRandomNumber(MAT_LENGTH*MAT_LENGTH, C);
    // read_file(FILE_A, &A[0]);
    // read_file(FILE_B, &B[0]);
    // read_file(FILE_C, &C[0]);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now(); 
    cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans,
    MAT_LENGTH,MAT_LENGTH,MAT_LENGTH,ALPHA,A, MAT_LENGTH, B, MAT_LENGTH,BETA,C,MAT_LENGTH);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start); 
    std::cout << "It took me " << time_span.count() << " seconds." << std::endl;
    // write_file(FILE_C, &C[0], MAT_LENGTH*MAT_LENGTH);
    delete [] A;
    delete [] B;
    delete [] C;
    return 0;
}

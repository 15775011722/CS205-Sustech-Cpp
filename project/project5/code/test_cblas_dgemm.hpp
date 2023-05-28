// #include <stddef.h>

#ifndef OPENBLAS_CONST
# define OPENBLAS_CONST const
#endif

typedef enum CBLAS_ORDER {CblasRowMajor=101, CblasColMajor=102} CBLAS_ORDER;
typedef enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113, CblasConjNoTrans=114} CBLAS_TRANSPOSE;
typedef int blasint;

void cblas_dgemm(
	OPENBLAS_CONST enum CBLAS_ORDER Order, 
	OPENBLAS_CONST enum CBLAS_TRANSPOSE TransA, 
	OPENBLAS_CONST enum CBLAS_TRANSPOSE TransB, 
	OPENBLAS_CONST blasint M, 
	OPENBLAS_CONST blasint N, 
	OPENBLAS_CONST blasint K,
	OPENBLAS_CONST double alpha, 
	OPENBLAS_CONST double *A, 
	OPENBLAS_CONST blasint lda, 
	OPENBLAS_CONST double *B, 
	OPENBLAS_CONST blasint ldb, 
	OPENBLAS_CONST double beta, 
	double *C, 
	OPENBLAS_CONST blasint ldc);
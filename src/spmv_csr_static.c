#define _GNU_SOURCE        /* per affinity/pinning opzionale */
#include "spmv_kernels.h"
#include <omp.h>
#include <stdint.h>

#ifndef PREFDIST
#define PREFDIST 2         /* distanza di prefetch sui vettori   */
#endif

/*---------- kernel CSR + OpenMP static ------------------------------------*/
void spmv_csr_static(const CSRMatrix *restrict A,
    const double *restrict x,
    double       *restrict y)
{
#pragma omp parallel for schedule(static,128)
for (int32_t i = 0; i < A->rows; ++i) {
double sum = 0.0;
int32_t start = A->row_ptr[i], end = A->row_ptr[i+1];

#pragma omp simd reduction(+:sum)
for (int32_t p = start; p < end; ++p) {
#if PREFDIST>0
if (p + PREFDIST < end)
__builtin_prefetch(&x[A->col_idx[p+PREFDIST]], 0, 1);
#endif
sum += A->values[p] * x[A->col_idx[p]];
}
y[i] = sum;
}
}


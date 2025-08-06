#if !defined(MEASURE_PERFORMANCE_H)
#define MEASURE_PERFORMANCE_H

#include "CSRMatrix.h"

double measure_spmv_csr_parallel(const CSRMatrix *csr, double *x, double *y,
    int repetitions, int num_threads);

double measure_spmv_csr_serial(const CSRMatrix *A, double *x, double *y,
    int repetitions);    
    
#endif 



#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "include/CSRMatrix.h"

double measure_spmv_csr_parallel(const CSRMatrix *csr, double *x, double *y,
                                 int repetitions, int num_threads){
    double start_time, end_time;
    int rep;
    int i;
    int k;
    double sum = 0.0;

    omp_set_num_threads(num_threads);

    start_time = omp_get_wtime();

    for (rep = 0; rep < repetitions; rep++) {
        
        #pragma omp parallel for schedule(static) private(i, k)
        for (i = 0; i < csr->rows; i++){
            sum = 0.0;
            for (k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++)
            {
                sum += csr->values[k] * x[csr->col_idx[k]];
            }
            y[i] = sum;
        }
    }

    end_time = omp_get_wtime();
    double avg_time = (end_time - start_time) / repetitions;

    return avg_time;
}

double measure_spmv_csr_serial(const CSRMatrix *csr, const double *x, double *y,
                               int repetitions)
{
    double start_time, end_time;

    int rep;
    int i;
    int k;
    double sum = 0.0;

    start_time = omp_get_wtime();

    for (rep = 0; rep < repetitions; rep++)
    {
        for (i = 0; i < csr->rows; i++)
        {
            sum = 0.0;
            for (k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++)
            {
                sum += csr->values[k] * x[csr->col_idx[k]];
            }
            y[i] = sum;
        }
    }

    end_time = omp_get_wtime();
    double avg_time_sec = (end_time - start_time) / repetitions;

    return avg_time_sec;
}

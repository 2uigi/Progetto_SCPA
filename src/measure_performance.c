#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "include/CSRMatrix.h"

double measure_spmv_csr_parallel(const CSRMatrix *csr, double *x, double *y,
                        int repetitions, int num_threads){

    double start_time, end_time;
    omp_set_num_threads(num_threads);

    start_time = omp_get_wtime();

    for (int rep = 0; rep < repetitions; rep++) {
        #pragma omp parallel for
        for (int i = 0; i < csr->rows; i++) {
            double sum = 0.0;
            for (int k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++) {
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
                               int repetitions) {
    double start_time, end_time;

    start_time = omp_get_wtime();

    for (int rep = 0; rep < repetitions; rep++) {
        for (int i = 0; i < csr->rows; i++) {
            double sum = 0.0;
            for (int k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++) {
                sum += csr->values[k] * x[csr->col_idx[k]];
            }
            y[i] = sum;
        }
    }

    end_time = omp_get_wtime();
    double avg_time_sec = (end_time - start_time) / repetitions;
    double flops = (2.0 * csr->nnz) / avg_time_sec;

    if (flops < 1e9)
        printf("Performance: %.2f MFLOPS\n", flops / 1e6);
    else
        printf("Performance: %.2f GFLOPS\n", flops / 1e9);

    return (end_time - start_time) / repetitions;
}


void report_performance(double avg_time_sec, int NZ) {
    double flops = (2.0 * NZ) / avg_time_sec;

    if (flops < 1e9)
        printf("Performance: %.2f MFLOPS\n", flops / 1e6);
    else
        printf("Performance: %.2f GFLOPS\n", flops / 1e9);
}

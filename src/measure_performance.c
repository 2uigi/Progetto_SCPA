#define _POSIX_C_SOURCE 199309L  // se usi GCC, prima di qualunque #include
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "include/CSRMatrix.h"

double measure_spmv_csr_parallel(const CSRMatrix *csr, double *x, double *y,
                                 int repetitions, int num_threads)
{
    struct timespec start, end;

    omp_set_num_threads(num_threads);

    // Start timer
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        perror("clock_gettime start");
        exit(EXIT_FAILURE);
    }

    for (int rep = 0; rep < repetitions; rep++)
    {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < csr->rows; i++)
        {
            double sum = 0.0; // dichiarata qui per essere privata ad ogni thread
            for (int k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++)
            {
                sum += csr->values[k] * x[csr->col_idx[k]];
            }
            y[i] = sum;
        }
    }

    // End timer
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
    {
        perror("clock_gettime end");
        exit(EXIT_FAILURE);
    }

    double elapsed_sec = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    if (elapsed_sec < 1e-9)  // evita tempo nullo
        elapsed_sec = 1e-9;

    return elapsed_sec / repetitions;
}

double measure_spmv_csr_serial(const CSRMatrix *csr, const double *x, double *y,
                               int repetitions)
{
    struct timespec start, end;
    int rep, i, k;
    double sum = 0.0;

    // Avvia il timer ad alta risoluzione
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        perror("clock_gettime start");
        exit(EXIT_FAILURE);
    }

    for (rep = 0; rep < repetitions; rep++)
    {
        for (i = 0; i < csr->rows; i++)
        {
            sum = 0.0;
            #pragma omp simd reduction(+:sum)
            for (k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; k++)
            {
                sum += csr->values[k] * x[csr->col_idx[k]];
            }
            y[i] = sum;
        }
    }

    // Ferma il timer
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
    {
        perror("clock_gettime end");
        exit(EXIT_FAILURE);
    }

    double elapsed_sec = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    if (elapsed_sec < 1e-9)  // evita tempo nullo
        elapsed_sec = 1e-9;

    return elapsed_sec / repetitions;
}

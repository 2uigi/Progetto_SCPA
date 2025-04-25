#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "include/CSRMatrix.h"
#include "include/spmv_kernels.h"

/*-------------------------------------------------------------
 *  Small helper: difference between two timespec in seconds
 *-----------------------------------------------------------*/
static inline double diff_sec(struct timespec start, struct timespec end)
{
    return (end.tv_sec  - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

/*----------------------------------------------------------------
 *  measure_spmv()
 *  ----------------
 *  Generic high‑resolution timer for any SpMV kernel that follows
 *  the spmv_kernel_t signature declared in spmv_kernels.h.
 *
 *  A        : pointer to CSR matrix
 *  kernel   : y = A * x implementation
 *  x, y     : input / output vectors (y must be allocated)
 *  repeats  : number of timed repetitions (>=1)
 *  nthreads : >0  -> overrides OMP thread count for the region
 *             <=0 -> honour OMP environment default
 *
 *  Returns the average wall‑time per call in seconds.
 *----------------------------------------------------------------*/
double measure_spmv(const CSRMatrix *A,
                    spmv_kernel_t    kernel,
                    const double    *x,
                    double          *y,
                    int              repeats,
                    int              nthreads)
{
    if (repeats < 1) repeats = 1;

    /* Warm‑up (NOT timed) to fault pages and fill caches */
    kernel(A, x, y);

    if (nthreads > 0)
        omp_set_num_threads(nthreads);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t0);

    /* Team created once: loop over repeats inside                 */
    #pragma omp parallel
    {
        for (int r = 0; r < repeats; ++r)
            kernel(A, x, y);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    return diff_sec(t0, t1) / repeats;
}

/*-------------------------------------------------------------
 *  Convenience wrappers so existing call‑sites keep compiling
 *-----------------------------------------------------------*/

double measure_spmv_csr_serial(const CSRMatrix *A,
                               const double    *x,
                               double          *y,
                               int              repeats)
{
    return measure_spmv(A, spmv_csr_serial, x, y, repeats, 1);
}

double measure_spmv_csr_parallel(const CSRMatrix *A,
                                 const double    *x,
                                 double          *y,
                                 int              repeats,
                                 int              nthreads)
{
    /* nthreads <= 0  -> use OMP default */
    return measure_spmv(A, spmv_csr_static, x, y,
                        repeats, nthreads > 0 ? nthreads : -1);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <omp.h>

#include "file_utils.h"
#include "read_mtx.h"
#include "CSRMatrix.h"
#include "COOMatrix.h"
#include "random_vec.h"
#include "measure_performance.h"
#include "store_performance.h"
#include "matrix_analysis.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define PATH_MAX_LENGTH 4096

// Macro per gestire errori di sistema con messaggi e exit
#define DIE(msg)                                           \
    do                                                     \
    {                                                      \
        fprintf(stderr, "%s: %s\n", msg, strerror(errno)); \
        exit(EXIT_FAILURE);                                \
    } while (0)

// Alloca memoria allineata o termina il programma
static void *alloc_aligned(size_t alignment, size_t size)
{
    void *ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0)
    {
        DIE("Aligned allocation failed");
    }
    return ptr;
}

// Crea e inizializza performance_parameters
static performance_parameters *create_perf_params(const CSRMatrix *csr, const char *path,
                                                  double avg_time, int threads, int reps)
{
    performance_parameters *p = malloc(sizeof(*p));
    if (!p)
    {
        DIE("Failed to allocate performance_parameters");
    }
    p->avg_time_sec = avg_time;
    p->matrix_filename = strdup(get_filename_from_path(path));
    p->NZ = csr->nnz;
    p->num_threads = threads;
    p->repetitions = reps;
    p->iterations = (long long)csr->rows * reps;
    return p;
}

// Prepara il problema: legge il file, trasforma coo->csr, alloca x e y
static void prepare_problem(const char *mtx_file, CSRMatrix **out_csr,
                            double **out_x, double **out_y)
{
    FILE *f = fopen(mtx_file, "r");
    if (!f)
        DIE("Failed to open matrix file");

    COOMatrix *coo = read_coo_from_mtx(f);
    fclose(f);
    if (!coo)
        DIE("Failed to read COO matrix");

    CSRMatrix *csr = convert_coo_to_csr(coo);
    free(coo);

    double *x = alloc_aligned(64, csr->cols * sizeof *x);
    for (int i = 0; i < csr->cols; ++i)
    {
        x[i] = drand48();
    }

    double *y = malloc(csr->rows * sizeof *y);
    if (!y)
        DIE("Failed to allocate output vector y");

    *out_csr = csr;
    *out_x = x;
    *out_y = y;
}

// Esegue warm-up e misura SpMV CSR in seriale
static double benchmark_spmv_serial(const CSRMatrix *csr, const double *x, double *y,
                             int warmup_iters, int measure_iters){

    measure_spmv_csr_serial((CSRMatrix *)csr, (double *)x, y, warmup_iters);
    return measure_spmv_csr_serial((CSRMatrix *)csr, (double *)x, y, measure_iters);

}

// Esegue warm-up e misura SpMV CSR in seriale
static double benchmark_spmv_parallel(const CSRMatrix *csr, const double *x, double *y,
    int warmup_iters, int measure_iters, int nthreads){

    measure_spmv_csr_parallel((CSRMatrix *)csr, (double *)x, y, warmup_iters, nthreads);
    return measure_spmv_csr_parallel((CSRMatrix *)csr, (double *)x, y, measure_iters, nthreads);

}

// Analizza matrice: baseline seriale, poi con RCM
static void analyze_matrix(const char *path, int warmup, int measure, int nthreads){
    CSRMatrix *csr;
    double *x, *y;

    printf("\nProcessing: %s\n", path);
    prepare_problem(path, &csr, &x, &y);

    // Misura seriale
    double t0 = benchmark_spmv_serial(csr, x, y, warmup, measure);
    performance_parameters *p0 = create_perf_params(csr, path, t0, 1, measure);
    report_performance_to_csv(p0);
    free(p0->matrix_filename);
    free(p0);

    // Misura parallela
    /*t0 = benchmark_spmv_parallel(csr, x, y, warmup, measure, nthreads);
    p0 = create_perf_params(csr, path, t0, nthreads, measure);
    report_performance_to_csv(p0);
    free(p0->matrix_filename);
    free(p0);*/

    // Cleanup
    free(x);
    free(y);
    free(csr->row_ptr);
    free(csr->col_idx);
    free(csr->values);
    free(csr);
}

int main(int argc, char *argv[])
{     
    char **file_list;
        if (argc < 4) {
        fprintf(stderr, "Usage: %s <warmup> <measure> <nthreads> <matrix_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;

    // Conversione di argv[1] in int warmup
    long temp = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid warmup value: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    int warmup = (int)temp;

    // Conversione di argv[2] in int measure
    temp = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid measure value: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    int measure = (int)temp;

    // Conversione di argv[3] in int nthreads
    temp = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid nthreads value: %s\n", argv[3]);
        return EXIT_FAILURE;
    }
    int nthreads = (int)temp;

    char *matrix_path = argv[4];

    printf("warmup=%d, measure=%d, nthreads=%d matrix_path=%s\n", warmup, measure, nthreads, matrix_path);

    analyze_matrix(matrix_path, warmup, measure, nthreads);

    /*    char* matrice_path = "matrici_cluster_Ultra_Sparse_Regular";
    int file_count;

    if (list_mtx_files(matrice_path, &file_list, &file_count) != 0){

        fprintf(stderr, "Errore: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Trovati %d file .mtx:\n", file_count);
    
    
    
    
    for (int i = 0; i < file_count; i++){

        char full_path[PATH_MAX_LENGTH];

        snprintf(full_path, sizeof(full_path), "%s/%s", matrice_path, file_list[i]);

        printf("\nProcessando il file: %s\n", full_path);

        analyze_matrix("/matrici_cluster_Ultra_Sparse_Regular/thermal2.mtx", warmup, measure, nthreads);
    }

    //free delle liste
    /*for (int i = 0; i < file_count; i++)
    {
        free(file_list[i]);
    }
    free(file_list);
        return EXIT_SUCCESS;
    */    
    
}

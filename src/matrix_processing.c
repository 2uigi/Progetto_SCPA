#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "include/file_utils.h"
#include "include/COOMatrix.h"
#include "include/CSRMatrix.h"
#include "include/ELLPACKMatrix.h"
#include "include/random_vec.h"
#include "include/measure_performance.h"
#include "include/store_performance.h"


void process_matrix_file(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    double* x;
    double* y;
    double avg_time_sec;

    if (f == NULL) {
        fprintf(stderr, "Errore nell'aprire il file %s: %s\n", file_path, strerror(errno));
        return;
    }

    COOMatrix *coo = read_coo_from_mtx(f);
    fclose(f);  

    if (coo == NULL) {
        fprintf(stderr, "Errore nella lettura della matrice da %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    CSRMatrix *csr = convert_coo_to_csr(coo);

    x = generate_random_vector_for_csr(csr->cols);
    y = (double*)malloc(sizeof(double) * csr->rows);
    if (y==NULL) {
        fprintf(stderr, "Errore nell'allocazione del vettore y\n");
        exit(EXIT_FAILURE);
    }
    
    performance_parameters *p_p = (performance_parameters*) malloc(sizeof(performance_parameters*));
    if(p_p == NULL){
        fprintf(stderr, "Errore allocazione performance_parameters\n");
        exit(EXIT_FAILURE);
    }

    p_p->avg_time_sec = measure_spmv_csr_serial(csr, x, y, 1);
    p_p->matrix_filename = get_filename_from_path(file_path);
    p_p->NZ = csr->nnz;
    p_p->num_threads = 1;
    p_p->repetitions = 1;

    report_performance_to_csv(p_p);

    measure_spmv_csr_parallel(csr,x, y, 1,20);

    free(p_p);
    free(x);
    free(y);

    // Libera le risorse
    free(coo);
    free(csr);
    //free(ellp);
}
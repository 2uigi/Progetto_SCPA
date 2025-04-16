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


void process_matrix_file(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    double* x;
    double* y;

    if (f == NULL) {
        fprintf(stderr, "Errore nell'aprire il file %s: %s\n", file_path, strerror(errno));
        return;
    }

    COOMatrix *coo = read_coo_from_mtx(f);
    fclose(f);  

    if (coo == NULL) {
        fprintf(stderr, "Errore nella lettura della matrice da %s\n", file_path);
        return;
    }

    // Stampa la matrice COO prima e dopo ordinamento
    //print_coo_matrix(coo);
    //printf("------ sort ------\n");
    //sort_coo_matrix(coo);
    //print_coo_matrix(coo);

    // Conversione da COO a CSR
    CSRMatrix *csr = convert_coo_to_csr(coo);
    //print_csr_matrix(csr);
    printf("\n-------------------------------------\n");
    //print_full_matrix_from_csr(csr);

    x = generate_random_vector_for_csr(csr->cols);
    y = (double *)malloc(sizeof(double) * csr->rows);
    if (!y) {
        fprintf(stderr, "Errore nell'allocazione del vettore y\n");
        exit(EXIT_FAILURE);
    }
    measure_spmv_csr_serial(csr, x, y, 100);

    
    // Conversione da COO a ELLPACK
    //ELLPACKMatrix *ellp = convert_coo_to_ellp(coo);
    //print_ellpack_matrix(ellp);

    // Libera le risorse
    free(coo);
    free(csr);
    //free(ellp);
}
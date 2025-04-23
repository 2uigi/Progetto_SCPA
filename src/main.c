#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "include/file_utils.h"
#include "include/read_mtx.h"
#include "include/CSRMatrix.h"
#include "include/COOMatrix.h"
#include "include/random_vec.h"
#include "include/measure_performance.h"
#include "include/store_performance.h"
#include "include/RCM_Reorder.h"

#ifndef PATH_MAX_LENGTH
#define PATH_MAX_LENGTH 4096
#endif

const char matrice_path[] = "matrici";    // directory con i .mtx 

// Factory per creare e inizializzare performance_parameters
performance_parameters *create_performance_parameters(const CSRMatrix *csr, const char *file_path, double avg_time, int threads, int repetitions) {
    performance_parameters *p = malloc(sizeof(performance_parameters));
    if (!p) {
        fprintf(stderr, "Errore allocazione performance_parameters\n");
        exit(EXIT_FAILURE);
    }

    p->avg_time_sec = avg_time;
    p->matrix_filename = get_filename_from_path(file_path); // Assicurati che faccia strdup()
    p->NZ = csr->nnz;
    p->num_threads = threads;
    p->repetitions = repetitions;

    return p;
}

void analyze_single_matrix(const char *file_path, int warmup_iters, int measure_iters) {
    printf("\nAnalisi del file singolo: %s\n", file_path);

    FILE *f = fopen(file_path, "r");
    if (!f) {
        fprintf(stderr, "Errore nell'aprire il file %s: %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    COOMatrix *coo = read_coo_from_mtx(f);
    fclose(f);

    if (!coo) {
        fprintf(stderr, "Errore nella lettura della matrice da %s\n", file_path);
        exit(EXIT_FAILURE);
    }

    CSRMatrix *csr = convert_coo_to_csr(coo);
    free(coo);

    double *x;
    if (posix_memalign((void**)&x, 64, csr->cols * sizeof(double)) != 0) {
        fprintf(stderr, "Allocazione allineata fallita\n");
        exit(EXIT_FAILURE);
    }

    // Ora x è allocato e allineato: lo inizializziamo una volta sola
    for (int i = 0; i < csr->cols; i++) {
        x[i] = drand48();
    }

    double *y = malloc(sizeof(double) * csr->rows);
    if (!y) {
        fprintf(stderr, "Errore nell'allocazione del vettore y\n");
        free(csr);
        free(x);
        exit(EXIT_FAILURE);
    }

    // Warm-up
    measure_spmv_csr_serial(csr, x, y, warmup_iters);

    // Profilazione vera e propria
    double serial_time = measure_spmv_csr_serial(csr, x, y, measure_iters);
    printf("Tempo medio (seriale, %d ripetizioni): %f sec\n", measure_iters, serial_time);

    // Cleanup
    free(x);
    free(y);
    free(csr->row_ptr);
    free(csr->col_idx);
    free(csr->values);
    free(csr);
}


int main(int argc, char* argv[]) {

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <matrix.mtx> <warmup_iters> <measure_iters>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *file = argv[1];
    int warmup_iters  = atoi(argv[2]);
    int measure_iters = atoi(argv[3]);

    analyze_single_matrix("matrici/cavity10.mtx", warmup_iters, measure_iters);

    /*char **file_list;
    int file_count;

    if (list_mtx_files(matrice_path, &file_list, &file_count) != 0) {
        fprintf(stderr, "Errore: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Trovati %d file .mtx:\n", file_count);

    for (int i = 0; i < file_count; i++) {
        char full_path[PATH_MAX_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", matrice_path, file_list[i]);
        printf("\nProcessando il file: %s\n", full_path);

        // --- Caricamento matrice in formato COO ---
        FILE *f = fopen(full_path, "r");
        if (!f) {
            fprintf(stderr, "Errore nell'aprire il file %s: %s\n", full_path, strerror(errno));
            continue;
        }

        COOMatrix *coo = read_coo_from_mtx(f);
        fclose(f);

        if (!coo) {
            fprintf(stderr, "Errore nella lettura della matrice da %s\n", full_path);
            continue;
        }

        CSRMatrix *csr = convert_coo_to_csr(coo);
        free(coo);

        //analyze_matrix_structure(csr, file_list[i], "matrix_structure_report.txt");
        
        // --- Allocazione vettori ---
        double *x = generate_random_vector_for_csr(csr->cols);
        posix_memalign((void**)&x, 64, csr->cols * sizeof(double));
        double *y = malloc(sizeof(double) * csr->rows);
        if (!y) {
            fprintf(stderr, "Errore nell'allocazione del vettore y\n");
            free(csr);
            continue;
        }

        // --- Warm-up ---
        measure_spmv_csr_serial(csr, x, y, 10);

        // --- Misurazione performance seriale ---
        double serial_time = measure_spmv_csr_serial(csr, x, y, 100);
        performance_parameters *p_p = create_performance_parameters(csr, full_path, serial_time, 1, 100);
        report_performance_to_csv(p_p);
        free(p_p->matrix_filename);
        free(p_p);

        // --- Reordering RCM ---
        CSRMatrix csr_reordered;
        double *x_reordered = NULL;

        apply_rcm_to_csr(csr, x, &csr_reordered, &x_reordered);
        posix_memalign((void**)&x, 64, csr->cols * sizeof(double));

        // --- Misurazione performance seriale con reordering ---
        double reordered_time = measure_spmv_csr_serial(&csr_reordered, x_reordered, y, 100);
        p_p = create_performance_parameters(csr, full_path, reordered_time, 1, 100);
        report_performance_to_csv(p_p);
        free(p_p->matrix_filename);
        free(p_p);

        
        //--- Cleanup ---
        free(x);
        //free(x_reordered);
        free(y);
        free(csr->row_ptr);
        free(csr->col_idx);
        free(csr->values);
        free(csr);

        //free(csr_reordered.row_ptr);
        //free(csr_reordered.col_idx);
        //free(csr_reordered.values);
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
    free(file_list);*/

    return EXIT_SUCCESS;
}


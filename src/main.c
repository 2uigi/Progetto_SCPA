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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <directory_matrici>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char **file_list;
    int file_count;

    if (list_mtx_files(matrice_path, &file_list, &file_count) != 0) {
        fprintf(stderr, "Errore: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Trovati %d file .mtx:\n", file_count);

    for (int i = 0; i < file_count; i++) {
        char full_path[PATH_MAX_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", matrice_path, file_list[i]);
        printf("\n🔧 Processando il file: %s\n", full_path);

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

        // --- Cleanup ---
        free(x);
        free(x_reordered);
        free(y);
        free(csr->row_ptr);
        free(csr->col_idx);
        free(csr->values);
        free(csr);

        free(csr_reordered.row_ptr);
        free(csr_reordered.col_idx);
        free(csr_reordered.values);
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
    free(file_list);

    return EXIT_SUCCESS;
}


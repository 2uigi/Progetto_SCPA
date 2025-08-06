#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "COOMatrix.h"
#include "mmio.h"  

void print_coo_matrix(COOMatrix *coo) {
    if (coo == NULL) {
        printf("Matrice COO nulla.\n");
        return;
    }

    printf("COO Matrix: %d x %d, nnz = %d\n", coo->rows, coo->cols, coo->nnz);
    printf("Formato: (riga, colonna) -> valore\n");

    for (int i = 0; i < coo->nnz; i++) {
        if (coo->V) {
            printf("(%d, %d) -> %lf\n", coo->I[i], coo->J[i], coo->V[i]);
        } else {
            printf("(%d, %d) -> 1.0 (pattern)\n", coo->I[i], coo->J[i]);
        }
    }
}

COOMatrix* transpose_coo(COOMatrix *mat) {
    COOMatrix *T = (COOMatrix*)malloc(sizeof(COOMatrix));
    T->rows = mat->cols;
    T->cols = mat->rows;
    T->nnz = mat->nnz;
    T->I = (int*)malloc(mat->nnz * sizeof(int));
    T->J = (int*)malloc(mat->nnz * sizeof(int));
    T->V = mat->V ? (double*)malloc(mat->nnz * sizeof(double)) : NULL;

    for (int i = 0; i < mat->nnz; i++) {
        T->I[i] = mat->J[i];  // swap
        T->J[i] = mat->I[i];
        if (mat->V) {
            T->V[i] = mat->V[i];
        }
    }

    return T;
}


// Funzione di confronto per qsort: prima riga, poi colonna
int compare_triplet(const void *a, const void *b) {
    Triplet *t1 = (Triplet *)a;
    Triplet *t2 = (Triplet *)b;

    if (t1->row != t2->row)
        return t1->row - t2->row;
    return t1->col - t2->col;
}

void sort_coo_matrix(COOMatrix *coo) {
    int nnz = coo->nnz;

    // Creiamo array di triplette temporanee
    Triplet *triplets = malloc(nnz * sizeof(Triplet));
    if (!triplets) {
        fprintf(stderr, "Errore allocazione triplets\n");
        exit(EXIT_FAILURE);
    }

    // Popola l'array temporaneo con i dati della COOMatrix
    for (int i = 0; i < nnz; i++) {
        triplets[i].row = coo->I[i];
        triplets[i].col = coo->J[i];
        triplets[i].val = coo->V ? coo->V[i] : 0.0;
    }

    // Ordina le triplette
    qsort(triplets, nnz, sizeof(Triplet), compare_triplet);

    // Scrive i dati ordinati di nuovo nella matrice COO
    for (int i = 0; i < nnz; i++) {
        coo->I[i] = triplets[i].row;
        coo->J[i] = triplets[i].col;
        if (coo->V)
            coo->V[i] = triplets[i].val;
    }

    free(triplets);
}


COOMatrix* read_coo_from_mtx(FILE *f) {
    MM_typecode matcode;
    int M, N, nz;

    if (mm_read_banner(f, &matcode) != 0) {
        fprintf(stderr, "Errore nella lettura del banner MatrixMarket.\n");
        return NULL;
    }

    if (!mm_is_matrix(matcode)) {
        fprintf(stderr, "Non è un oggetto di tipo matrice.\n");
        return NULL;
    }

    int is_symmetric = mm_is_symmetric(matcode);
    int is_pattern = mm_is_pattern(matcode);
    int is_sparse = mm_is_sparse(matcode);
    int is_array = mm_is_array(matcode);

    if (is_sparse && mm_read_mtx_crd_size(f, &M, &N, &nz) != 0) {
        fprintf(stderr, "Errore nella lettura delle dimensioni della matrice (coordinate).\n");
        return NULL;
    } else if (is_array && mm_read_mtx_array_size(f, &M, &N) != 0) {
        fprintf(stderr, "Errore nella lettura delle dimensioni della matrice (array).\n");
        return NULL;
    }

    int max_nnz = is_symmetric ? (is_sparse ? 2 * nz : 2 * M * N) : (is_sparse ? nz : M * N);
    int *I = malloc(max_nnz * sizeof(int));
    int *J = malloc(max_nnz * sizeof(int));
    double *V = is_pattern ? NULL : malloc(max_nnz * sizeof(double));

    int actual_nnz = 0;

    if (is_sparse) {
        for (int i = 0; i < nz; i++) {
            int row, col;
            double val = 1.0;

            if (is_pattern) {
                if (fscanf(f, "%d %d\n", &row, &col) != 2) {
                    fprintf(stderr, "Errore durante la lettura degli indici.\n");
                    exit(1);
                }
            } else {
                if (fscanf(f, "%d %d %lf\n", &row, &col, &val) != 3) {
                    fprintf(stderr, "Errore durante la lettura di riga, colonna e valore.\n");
                    exit(1);
                }
            }

            row--; col--;  // 1-based → 0-based

            I[actual_nnz] = row;
            J[actual_nnz] = col;
            if (!is_pattern) V[actual_nnz] = val;
            actual_nnz++;

            if (is_symmetric && row != col) {
                I[actual_nnz] = col;
                J[actual_nnz] = row;
                if (!is_pattern) V[actual_nnz] = val;
                actual_nnz++;
            }
        }
    } else if (is_array) {
        // Formato denso, ordinamento colonna per colonna
        for (int col = 0; col < N; col++) {
            for (int row = 0; row < M; row++) {
                double val = 1.0;

                if (!is_pattern) {
                    if (fscanf(f, "%lf\n", &val) != 1) {
                        fprintf(stderr, "Errore nella lettura del valore in formato array.\n");
                        exit(1);
                    }
                }

                I[actual_nnz] = row;
                J[actual_nnz] = col;
                if (!is_pattern) V[actual_nnz] = val;
                actual_nnz++;

                if (is_symmetric && row != col) {
                    I[actual_nnz] = col;
                    J[actual_nnz] = row;
                    if (!is_pattern) V[actual_nnz] = val;
                    actual_nnz++;
                }
            }
        }
    }

    COOMatrix *mat = malloc(sizeof(COOMatrix));
    mat->rows = M;
    mat->cols = N;
    mat->nnz = actual_nnz;
    mat->I = I;
    mat->J = J;
    mat->V = V;

    sort_coo_matrix(mat);  // opzionale 

    return mat;
}


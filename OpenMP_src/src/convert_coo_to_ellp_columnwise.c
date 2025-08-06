#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mmio.h"
#include "COOMatrix.h"
#include "ELLPACKMatrix.h"

void print_ellpack_columnwise(ELLPACKMatrix *ellp) {
    printf("ELLPACK Matrix (column-wise view):\n");
    printf("rows = %d, cols = %d, max_nnz = %d\n", ellp->rows, ellp->cols, ellp->max_nnz);
    printf("\n%-6s %-6s %-6s\n", "Col", "Idx", "Val");

    for (int j = 0; j < ellp->max_nnz; j++) {
        for (int i = 0; i < ellp->rows; i++) {
            int offset = i * ellp->max_nnz + j;
            if (offset >= ellp->rows * ellp->max_nnz) {
                printf("Accesso fuori dai limiti: offset = %d\n", offset);
                continue;
            }

            int col = ellp->col_idx[offset];
            double val = ellp->values[offset];

            printf("%-6d %-6d %-6.3f\n", j, col, val);
        }
        printf("\n");  // separazione tra colonne logiche
    }
}



int compare_by_col_then_row(const void *a, const void *b) {
    COOMatrix *coo = (COOMatrix*)b;  // L'array di contesto
    int i = *(int*)a;
    int j = *(int*)b;

    // Prima confronta la colonna, poi la riga
    if (coo->J[i] != coo->J[j]) {
        return coo->J[i] - coo->J[j];
    }
    return coo->I[i] - coo->I[j];
}


int max_non_zeros_per_column(COOMatrix *mat) {
    int max_non_zeros = 0;

    int current_col = mat->J[0];
    int current_col_count = 0;

    for (int i = 0; i < mat->nnz; i++) {
        if (mat->J[i] == current_col) {
            current_col_count++;
        } else {
            if (current_col_count > max_non_zeros) {
                max_non_zeros = current_col_count;
            }
            current_col = mat->J[i];
            current_col_count = 1;
        }
    }

    if (current_col_count > max_non_zeros) {
        max_non_zeros = current_col_count;
    }

    return max_non_zeros;
}

//LA CONVERSIONE NON FUNZIONA MA LA RIPRENDO PIU AVANTI

ELLPACKMatrix* convert_coo_to_ellp_by_columns(COOMatrix *coo) {
    // Conta i non-zeri per colonna
    int max_nnz = max_non_zeros_per_column(coo);

    // Crea la matrice ELLPACK trasposta
    ELLPACKMatrix *ellp = (ELLPACKMatrix*)malloc(sizeof(ELLPACKMatrix));
    ellp->rows = coo->cols;  // Le righe della trasposta sono le colonne della COO
    ellp->cols = coo->rows;  // Le colonne della trasposta sono le righe della COO
    ellp->max_nnz = max_nnz;

    // Alloca memoria per la matrice ELLPACK trasposta
    ellp->col_idx = (int*)malloc(ellp->rows * max_nnz * sizeof(int));
    ellp->values = (double*)malloc(ellp->rows * max_nnz * sizeof(double));

    // Inizializza con valori di default (-1 per col_idx, 0.0 per values)
    for (int i = 0; i < ellp->rows * max_nnz; i++) {
        ellp->col_idx[i] = -1;
        ellp->values[i] = 0.0;
    }

    // Array per tracciare quanti non-zeri sono stati scritti in ogni colonna della matrice trasposta
    int *col_written = (int*)calloc(ellp->rows, sizeof(int));

    // Itera sugli elementi della matrice COO e riempi la matrice ELLPACK trasposta
    for (int i = 0; i < coo->nnz; i++) {
        int row = coo->I[i];
        int col = coo->J[i];
        double val = coo->V ? coo->V[i] : 1.0;

        // Per ogni elemento (row, col) della matrice originale,
        // lo mettiamo nella posizione trasposta
        int offset = row * max_nnz + col_written[row];
        ellp->col_idx[offset] = col;  // La colonna diventa la riga nella trasposta
        ellp->values[offset] = val;
        col_written[row]++;
    }

    // Completa le colonne con valori di default
    for (int col = 0; col < ellp->rows; col++) {
        int filled = col_written[col];
        int last_valid_row = (filled > 0) ? ellp->col_idx[col * max_nnz + filled - 1] : 0;
        for (int j = filled; j < max_nnz; j++) {
            ellp->col_idx[col * max_nnz + j] = last_valid_row;
            ellp->values[col * max_nnz + j] = 0.0;
        }
    }

    free(col_written);
    return ellp;
}
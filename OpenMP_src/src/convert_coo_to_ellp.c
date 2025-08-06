#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mmio.h"
#include "COOMatrix.h"
#include "ELLPACKMatrix.h"

void print_ellpack_matrix(ELLPACKMatrix *ellp) {
    printf("ELLPACK Matrix:\n");
    printf("rows: %d, cols: %d, max_nnz: %d\n", ellp->rows, ellp->cols, ellp->max_nnz);

    // Stampa riga per riga
    for (int i = 0; i < ellp->rows; i++) {
        printf("Row %d: ", i);
        for (int j = 0; j < ellp->max_nnz; j++) {
            int idx = i * ellp->max_nnz + j;
            if (ellp->col_idx[idx] != -1) { // Stampa solo gli elementi validi
                printf("(%d, %.2f) ", ellp->col_idx[idx], ellp->values[idx]);
            }
        }
        printf("\n");
    }
}

int max_non_zeros_per_row(COOMatrix *mat) {
    int max_non_zeros = 0; 

    int current_row = mat->I[0];  
    int current_row_count = 0;    // Contatore degli elementi non nulli per la riga corrente

    // Conta i non-zeri per ogni riga
    for (int i = 0; i < mat->nnz; i++) {
        if (mat->I[i] == current_row) {
            current_row_count++;
        } else {
            if (current_row_count > max_non_zeros) {
                max_non_zeros = current_row_count;
            }
            current_row = mat->I[i];
            current_row_count = 1;  
        }
    }

    if (current_row_count > max_non_zeros) {
        max_non_zeros = current_row_count;
    }

    return max_non_zeros;
}

ELLPACKMatrix* convert_coo_to_ellp(COOMatrix *coo) {
    int max_nnz = max_non_zeros_per_row(coo);

    ELLPACKMatrix *ellp = (ELLPACKMatrix*)malloc(sizeof(ELLPACKMatrix));
    ellp->rows = coo->rows;
    ellp->cols = coo->cols;
    ellp->max_nnz = max_nnz;

    ellp->col_idx = (int*)malloc(coo->rows * max_nnz * sizeof(int));
    ellp->values = (double*)malloc(coo->rows * max_nnz * sizeof(double));

    int *row_written = (int*)calloc(coo->rows, sizeof(int));

    for (int i = 0; i < coo->rows * max_nnz; i++) {
        ellp->col_idx[i] = -1;
        ellp->values[i] = 0.0;
    }

    for (int i = 0; i < coo->nnz; i++) {
        int row = coo->I[i];
        int col = coo->J[i];
        double val = coo->V ? coo->V[i] : 1.0;

        int offset = row * max_nnz + row_written[row];
        ellp->col_idx[offset] = col;
        ellp->values[offset] = val;
        row_written[row]++;
    }

    for (int row = 0; row < coo->rows; row++) {
        int filled = row_written[row];
        int last_valid_col = (filled > 0) ? ellp->col_idx[row * max_nnz + filled - 1] : 0;

        for (int j = filled; j < max_nnz; j++) {
            ellp->col_idx[row * max_nnz + j] = last_valid_col;
            ellp->values[row * max_nnz + j] = 0.0;
        }
    }

    free(row_written);
    return ellp;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "include/mmio.h"
#include "include/CSRMatrix.h"
#include "include/COOMatrix.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>

void print_full_matrix_from_csr(CSRMatrix *csr) {

    double **matrix = (double**) malloc(csr->rows * sizeof(double*));
    for (int i = 0; i < csr->rows; i++) {
        matrix[i] = (double*) calloc(csr->cols, sizeof(double));  
    }

    for (int i = 0; i < csr->rows; i++) {
        for (int j = csr->row_ptr[i]; j < csr->row_ptr[i + 1]; j++) {
            int col = csr->col_idx[j];
            matrix[i][col] = csr->values[j];  
        }
    }


    for (int i = 0; i < csr->rows; i++) {
        for (int j = 0; j < csr->cols; j++) {
            printf("%lf ", matrix[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < csr->rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

CSRMatrix* convert_coo_to_csr(COOMatrix *coo) {
    int M = coo->rows;
    int N = coo->cols;
    int nnz = coo->nnz;

    int *row_ptr = calloc((M + 1), sizeof(int));
    int *col_idx = malloc(nnz * sizeof(int));
    double *values = malloc(nnz * sizeof(double));

    //printf("Fase 1 - Conteggio elementi per riga:\n");
    for (int i = 0; i < nnz; i++) {
        row_ptr[coo->I[i] + 1]++;
        //printf("Elemento COO (%d, %d) → row_ptr[%d] ora %d\n", coo->I[i], coo->J[i], coo->I[i] + 1, row_ptr[coo->I[i] + 1]);
    }

    //printf("\nFase 2 - Somma cumulativa su row_ptr:\n");
    for (int i = 0; i < M; i++) {
        row_ptr[i + 1] += row_ptr[i];
        //printf("row_ptr[%d] = %d\n", i + 1, row_ptr[i + 1]);
    }

    //printf("\nrow_ptr finale:\n");
    for (int i = 0; i <= M; i++) {
        //printf("row_ptr[%d] = %d\n", i, row_ptr[i]);
    }

    // Offset per il riempimento
    int *offset = malloc((M + 1) * sizeof(int));
    memcpy(offset, row_ptr, (M + 1) * sizeof(int));

    //printf("\nFase 3 - Riempimento col_idx e values:\n");
    for (int i = 0; i < nnz; i++) {
        int row = coo->I[i];
        int col = coo->J[i];
    
        
        if (row < 0 || row >= M || col < 0 || col >= N) {
            fprintf(stderr, "Errore: Indice COO fuori limite: (%d, %d)\n", row, col);
            exit(EXIT_FAILURE);
        }
    

        int dest = offset[row]++;
        
        if (dest >= nnz) {
            fprintf(stderr, "Errore: offset overflow. dest = %d, nnz = %d\n", dest, nnz);
            exit(EXIT_FAILURE);
        }
        

        col_idx[dest] = col;
        values[dest] = coo->V ? coo->V[i] : 1.0;
        //printf("Inserisco (%d, %d) → %lf in CSR pos %d\n", row, col, values[dest], dest);
    }

    free(offset);

    CSRMatrix *csr = malloc(sizeof(CSRMatrix));
    csr->rows = M;
    csr->cols = N;
    csr->nnz = nnz;
    csr->row_ptr = row_ptr;
    csr->col_idx = col_idx;
    csr->values = values;

    return csr;
}

// Stampa di debug
void print_csr_matrix(CSRMatrix *csr) {
    printf("\n\n\n------------INIZIO STAMPA CSR------------\n");

    printf("row_ptr: ");
    for (int i = 0; i <= csr->rows; i++)
        printf("%d ", csr->row_ptr[i]);
    printf("\n");

    printf("col_idx: ");
    for (int i = 0; i < csr->nnz; i++)
        printf("%d ", csr->col_idx[i]);
    printf("\n");

    printf("values: ");
    for (int i = 0; i < csr->nnz; i++)
        printf("%.15lf ", csr->values[i]);
    printf("\n");
}


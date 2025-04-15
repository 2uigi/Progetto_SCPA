#include <stdio.h>
#include <stdlib.h>
#include "include/csr_product.h"

double* csr_matrix_vector_multiply(CSRMatrix *A, double *x, int n_rows) {

    double* y = (double*)malloc(sizeof(double)*n_rows);

    for (int i = 0; i < A->rows; i++) {
        double t = 0.0;

        for (int j = A->row_ptr[i]; j < A->row_ptr[i + 1]; j++) {
            t += A->values[j] * x[A->col_idx[j]];
        }

        y[i] = t;
    }

    return y;
}
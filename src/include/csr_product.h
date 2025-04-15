#ifndef CSR_PRODUCT_H
#define CSR_PRODUCT_H

#include "CSRMatrix.h"

double* csr_matrix_vector_multiply(CSRMatrix *A, double *x, int n_rows);

#endif

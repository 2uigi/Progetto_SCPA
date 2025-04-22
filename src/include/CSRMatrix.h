#ifndef CSR_MATRIX_H
#define CSR_MATRIX_H

#include "COOMatrix.h"

typedef struct {
    int rows;
    int cols;
    int nnz;
    int *row_ptr;
    int *col_idx;
    double *values;
} CSRMatrix;

CSRMatrix* convert_coo_to_csr(COOMatrix *coo);

void analyze_matrix_structure(const CSRMatrix *csr, const char *matrix_name, const char *report_path); 

void print_csr_matrix(CSRMatrix *csr);

void print_full_matrix_from_csr(CSRMatrix *csr);

#endif
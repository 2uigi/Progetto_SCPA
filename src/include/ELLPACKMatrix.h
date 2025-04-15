#ifndef ELLPACK_MATRIX_H
#define ELLPACK_MATRIX_H

#include "COOMatrix.h"

typedef struct {
    int rows;
    int cols;
    int max_nnz;
    int *col_idx;
    double *values;
} ELLPACKMatrix;

ELLPACKMatrix *convert_coo_to_ellp(COOMatrix *coo);

ELLPACKMatrix *convert_coo_to_ellp_by_columns(COOMatrix *coo);

int max_non_zeros_per_row(COOMatrix *mat);

int max_non_zeros_per_column(COOMatrix *mat); 

void print_ellpack_matrix(ELLPACKMatrix *ellp);

void print_ellpack_columnwise(ELLPACKMatrix *ellp);

#endif

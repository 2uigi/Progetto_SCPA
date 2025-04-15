#ifndef COO_MATRIX_H
#define COO_MATRIX_H


typedef struct {
    int rows;
    int cols;
    int nnz;       // numero di elementi non nulli (inclusi eventuali duplicati per simmetrici)
    int *I;        // righe
    int *J;        // colonne
    double *V;     // valori (può essere NULL se pattern)
} COOMatrix;

typedef struct {
    int row;
    int col;
    double val;
} Triplet;

void print_coo_matrix(COOMatrix *coo);

COOMatrix* read_coo_from_mtx(FILE *f); 

COOMatrix* transpose_coo(COOMatrix* coo);

int compare_triplet(const void *a, const void *b);

void sort_coo_matrix(COOMatrix *coo);

#endif


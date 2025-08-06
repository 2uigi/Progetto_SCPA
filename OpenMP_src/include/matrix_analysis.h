#ifndef MATRIX_ANALYSIS_H
#define MATRIX_ANALYSIS_H

#include "CSRMatrix.h"

typedef struct {
    int rows;           // numero di righe
    int cols;           // numero di colonne
    int nnz_total;      // numero totale di elementi non-zero
    double mu;          // media NNZ per riga
    double sigma;       // deviazione standard NNZ per riga
    double rho;         // densità globale
    int min_nnz;        // minimo NNZ in una riga
    int max_nnz;        // massimo NNZ in una riga
} MatrixFeatures;

typedef enum { CL_A, CL_B, CL_C, CL_D } cluster_t;


MatrixFeatures compute_csr_features(const CSRMatrix *csr);

void analyze_matrix_structure(const CSRMatrix *csr,
    const char *matrix_name,
    const char *report_path);
#endif
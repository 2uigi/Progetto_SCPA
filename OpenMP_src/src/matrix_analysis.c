#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "mmio.h"
#include "CSRMatrix.h"
#include "COOMatrix.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include "matrix_analysis.h"

/*-------------------------------------------------------------
 * THRESHOLDS – tweak in one place to retune classifier
 *-----------------------------------------------------------*/
#define CV_THR_HEAVY   2.0     /* heavy‑tail threshold          */
#define MU_THR_FEM    40.0     /* FEM‑like mean NNZ/row         */
#define CV_THR_FEM     0.25    /* regularity for FEM meshes     */
#define RHO_THR_DENSE  5e-3    /* global density for cluster D  */

cluster_t detect_cluster(const MatrixFeatures *features)
{
    double cv    = (features->mu > 0.0) ? features->sigma / features->mu : 0.0;
    if (cv > CV_THR_HEAVY)          /* heavy‑tail rows */
        return CL_C;
    else if (features->mu > MU_THR_FEM && cv <= CV_THR_FEM)    /* FEM‑like dense rows */
        return CL_B;
    else if (features->rho > RHO_THR_DENSE)   /* medium‑dense matrix */
        return CL_D;
    else                      /* ultra‑sparse graph */
        return CL_A;
}

MatrixFeatures compute_csr_features(const CSRMatrix *csr) {
    MatrixFeatures feat;
    feat.rows      = csr->rows;
    feat.cols      = csr->cols;
    feat.nnz_total = csr->row_ptr[csr->rows];

    // per accumulare statistiche
    int    min_nnz   = INT_MAX;
    int    max_nnz   = 0;
    double sum_nnz   = 0.0;
    double sum_nnz2  = 0.0;

    for (int i = 0; i < csr->rows; i++) {
        int nnz = csr->row_ptr[i+1] - csr->row_ptr[i];
        if (nnz < min_nnz)   min_nnz = nnz;
        if (nnz > max_nnz)   max_nnz = nnz;
        sum_nnz  += nnz;
        sum_nnz2 += (double)nnz * nnz;
    }

    // media
    feat.mu = sum_nnz / (double)csr->rows;

    // deviazione standard: sqrt(E[X^2] - E[X]^2)
    feat.sigma = sqrt(sum_nnz2 / (double)csr->rows - feat.mu * feat.mu);

    // densità globale = nnz_totali / (righe * colonne)
    uint64_t total_elems = (uint64_t)csr->rows * (uint64_t)csr->cols;
    feat.rho = (double)feat.nnz_total / (double)total_elems;

    feat.min_nnz = min_nnz;
    feat.max_nnz = max_nnz;
    return feat;
}

void analyze_matrix_structure(const CSRMatrix *csr,
                              const char *matrix_name,
                              const char *report_path)
{
    // Calcola tutte le feature in un'unica struct
    MatrixFeatures f = compute_csr_features(csr);

     // Determina il cluster
     cluster_t cluster = detect_cluster(&f);

    // Determina il nome del file di output in base al cluster
    const char *filename;
    switch (cluster) {
        case CL_A: filename = "cluster_A.txt"; break;
        case CL_B: filename = "cluster_B.txt"; break;
        case CL_C: filename = "cluster_C.txt"; break;
        case CL_D: filename = "cluster_D.txt"; break;
        default:
            fprintf(stderr, "Cluster non riconosciuto\n");
            return;
    }

    // Scrive il nome della matrice nel file del cluster
    FILE *cluster_file = fopen(filename, "a");
    if (!cluster_file) {
        perror("Errore nell'apertura del file cluster");
    } else {
        fprintf(cluster_file, "%s\n", matrix_name);
        fclose(cluster_file);
    }

    
    // Apre il file in append (crea se non esiste)
    FILE *report = fopen(report_path, "a");
    if (!report) {
        perror("Errore nell'apertura del file di report");
        return;
    }

    fprintf(report,
            "----------------------------------------\n"
            "Matrice: %s\n"
            "Dimensioni: %d x %d\n"
            "Numero totale di NZ: %d\n"
            "Densità globale (ρ): %.6f\n"
            "NZ per riga:\n"
            "  - min    : %d\n"
            "  - max    : %d\n"
            "  - media (μ): %.4f\n"
            "  - stddev (σ): %.4f\n",
            matrix_name,
            f.rows, f.cols,
            f.nnz_total,
            f.rho,
            f.min_nnz, f.max_nnz,
            f.mu, f.sigma);

    fclose(report);
}
#if !defined(RCM_REORDER_H)
#define RCM_REORDER_H
#include "CSRMatrix.h"

// Struttura di supporto per accodare nodi
typedef struct Queue {
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} Queue;

int *rcm_ordering(const CSRMatrix *csr);

void apply_rcm_to_csr(const CSRMatrix *csr_in, const double *x_in,
    CSRMatrix *csr_out, double **x_out);

#endif // RCM_REORDER_H

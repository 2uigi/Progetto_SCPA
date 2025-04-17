#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "include/CSRMatrix.h"
#include "include/RCM_Reorder.h"

// === QUEUE IMPLEMENTATION ===

Queue* create_queue(int capacity) {
    Queue *q = malloc(sizeof(Queue));
    if (!q) return NULL;
    q->data = malloc(sizeof(int) * capacity);
    if (!q->data) {
        free(q);
        return NULL;
    }
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int value) {
    if (q->size == q->capacity) return;
    q->data[q->rear++] = value;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    q->size--;
    return q->data[q->front++];
}

bool queue_is_empty(Queue *q) {
    return q->size == 0;
}

void free_queue(Queue *q) {
    free(q->data);
    free(q);
}

// === RCM IMPLEMENTATION ===

// Variabile globale per il comparatore (workaround per portabilità)
static const int *global_degree;

int compare_by_degree_global(const void *a, const void *b) {
    int va = *(const int *)a;
    int vb = *(const int *)b;
    return global_degree[va] - global_degree[vb];
}

void compute_degrees(const CSRMatrix *csr, int *degree) {
    for (int i = 0; i < csr->rows; i++) {
        degree[i] = csr->row_ptr[i + 1] - csr->row_ptr[i];
    }
}

int find_start_node(int *degree, bool *visited, int n) {
    int min_deg = __INT_MAX__;
    int start = -1;
    for (int i = 0; i < n; i++) {
        if (!visited[i] && degree[i] < min_deg) {
            min_deg = degree[i];
            start = i;
        }
    }
    return start;
}

int *rcm_ordering(const CSRMatrix *csr) {
    int n = csr->rows;
    bool *visited = calloc(n, sizeof(bool));
    int *degree = malloc(n * sizeof(int));
    int *rcm = malloc(n * sizeof(int));
    int rcm_index = n - 1;

    if (!visited || !degree || !rcm) {
        fprintf(stderr, "Errore allocazione memoria in rcm_ordering\n");
        exit(EXIT_FAILURE);
    }

    compute_degrees(csr, degree);
    Queue *queue = create_queue(n);
    if (!queue) {
        fprintf(stderr, "Errore allocazione coda\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int start = find_start_node(degree, visited, n);
        if (start == -1) break;

        enqueue(queue, start);
        visited[start] = true;

        while (!queue_is_empty(queue)) {
            int u = dequeue(queue);
            rcm[rcm_index--] = u;

            int neighbors_count = 0;
            int max_neighbors = csr->row_ptr[u + 1] - csr->row_ptr[u];
            int *neighbors = malloc(max_neighbors * sizeof(int));
            if (!neighbors) {
                fprintf(stderr, "Errore allocazione neighbors\n");
                exit(EXIT_FAILURE);
            }

            for (int k = csr->row_ptr[u]; k < csr->row_ptr[u + 1]; k++) {
                int v = csr->col_idx[k];
                if (!visited[v]) {
                    neighbors[neighbors_count++] = v;
                    visited[v] = true;
                }
            }

            global_degree = degree;
            qsort(neighbors, neighbors_count, sizeof(int), compare_by_degree_global);

            for (int i = 0; i < neighbors_count; i++) {
                enqueue(queue, neighbors[i]);
            }

            free(neighbors);
        }
    }

    free(visited);
    free(degree);
    free_queue(queue);

    return rcm;
}

// === APPLY PERMUTATION ===

void permute_csr_and_vector(const CSRMatrix *csr_in, const double *x_in,
                            const int *P, CSRMatrix *csr_out, double **x_out) {
    int n = csr_in->rows;
    int nnz = csr_in->nnz;

    csr_out->rows = n;
    csr_out->cols = csr_in->cols;
    csr_out->nnz = nnz;
    csr_out->row_ptr = malloc((n + 1) * sizeof(int));
    csr_out->col_idx = malloc(nnz * sizeof(int));
    csr_out->values = malloc(nnz * sizeof(double));
    *x_out = malloc(n * sizeof(double));

    if (!csr_out->row_ptr || !csr_out->col_idx || !csr_out->values || !(*x_out)) {
        fprintf(stderr, "Errore allocazione permutazione CSR\n");
        exit(EXIT_FAILURE);
    }

    int *Pinv = malloc(n * sizeof(int));
    if (!Pinv) {
        fprintf(stderr, "Errore allocazione Pinv\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        Pinv[P[i]] = i;
    }

    int nz_index = 0;
    csr_out->row_ptr[0] = 0;

    for (int new_i = 0; new_i < n; new_i++) {
        int old_i = P[new_i];
        for (int k = csr_in->row_ptr[old_i]; k < csr_in->row_ptr[old_i + 1]; k++) {
            int old_j = csr_in->col_idx[k];
            int new_j = Pinv[old_j];
            csr_out->col_idx[nz_index] = new_j;
            csr_out->values[nz_index] = csr_in->values[k];
            nz_index++;
        }
        csr_out->row_ptr[new_i + 1] = nz_index;
    }

    for (int i = 0; i < n; i++) {
        (*x_out)[i] = x_in[P[i]];
    }

    free(Pinv);
}

void apply_rcm_to_csr(const CSRMatrix *csr_in, const double *x_in,
                      CSRMatrix *csr_out, double **x_out) {
    int *P = rcm_ordering(csr_in);
    permute_csr_and_vector(csr_in, x_in, P, csr_out, x_out);
    free(P);
}

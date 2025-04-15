#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "include/random_vec.h"

void print_vector(double* vec, int size) {
    for (int i = 0; i < size; ++i) {
        printf("%.11f ", vec[i]);
    }
    printf("\n");
}

double generate_high_precision_random() {
    return (double)rand() / (double)RAND_MAX;
}

double* generate_random_vector_for_csr(int n_cols) {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL)); 
        initialized = 1;
    }

    double* x = (double*)malloc(n_cols * sizeof(double));
    if (!x) {
        perror("Errore nella malloc di x");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_cols; ++i) {
        x[i] = generate_high_precision_random(); // valore tra 0.0 e 1.0
    }

    printf("\nVettore y: ");
    print_vector(x, n_cols);
    return x;
}

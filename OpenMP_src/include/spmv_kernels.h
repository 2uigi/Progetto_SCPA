#ifndef SPMV_KERNELS_H
#define SPMV_KERNELS_H

#include "CSRMatrix.h"

/* Un “kernel SpMV” è qualunque funzione che:      */
/*   in : CSRMatrix*, vettore x                    */
/*   out: vettore y di dimensione rows (già alloc) */
typedef void (*spmv_kernel_t)(const CSRMatrix*,
                              const double*,
                              double*);

/* Prototipi */
void spmv_csr_serial (const CSRMatrix*, const double*, double*);
void spmv_csr_static (const CSRMatrix*, const double*, double*);

#endif

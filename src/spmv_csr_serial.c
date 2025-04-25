#include "spmv_kernels.h"

void spmv_csr_serial(const CSRMatrix *csr,
                     const double    *x,
                     double          *y)
{
    for (int i = 0; i < csr->rows; ++i) {
        double sum = 0.0;
        for (int k = csr->row_ptr[i]; k < csr->row_ptr[i + 1]; ++k)
            sum += csr->values[k] * x[csr->col_idx[k]];
        y[i] = sum;
    }
}

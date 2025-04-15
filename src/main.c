#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "include/mmio.h"
#include "include/file_utils.h"
#include "include/read_mtx.h"
#include "include/CSRMatrix.h"
#include "include/COOMatrix.h"
#include "include/ELLPACKMatrix.h"
#include <immintrin.h>

int main(int argc, char* argv[]){

    const char *matrice_path = "matrici";

    process_files(matrice_path);
    
    return 0;
}
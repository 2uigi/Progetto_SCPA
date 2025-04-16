#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "include/file_utils.h"
#include "include/COOMatrix.h"
#include "include/CSRMatrix.h"
#include "include/ELLPACKMatrix.h"
#include "include/matrix_processing.h"
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#ifndef PATH_MAX_LENGTH
#define PATH_MAX_LENGTH 4096
#endif

const char matrice_path[] = "matrici";    // directory con i .mtx 

void process_files(const char *matrice_path) {
    char **file_list;
    int file_count;

    /*
    if (list_mtx_files(matrice_path, &file_list, &file_count) != 0) {
        fprintf(stderr, "Errore: %s\n", strerror(errno));
        return;
    }

    printf("Trovati %d file .mtx:\n", file_count);

    for (int i = 0; i < file_count; i++) {
        char full_path[PATH_MAX_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", matrice_path, file_list[i]);

        printf("- Processando il file: %s\n", full_path);


        process_matrix_file(full_path);


        free(file_list[i]);
    }

    */

    process_matrix_file("/home/vboxuser/Desktop/SCPA/progetto_SCPA/matrici/lung2.mtx");

    free(file_list);
}


void print_curr_dir(){
    char cwd[4096];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        exit(1);
    }

}

int list_mtx_files(const char *directory, char ***file_list, int *file_count) {
    printf("la mia directory è %s\n", directory);

    struct dirent *entry;
    DIR *dp = opendir(directory);
    if (!dp) {
        perror("Errore nell'apertura della directory");
        return -1;
    }

    *file_count = 0;
    *file_list = NULL;

    while ((entry = readdir(dp))) {
        if (strstr(entry->d_name, ".mtx")) {
            (*file_count)++;
            *file_list = realloc(*file_list, (*file_count) * sizeof(char *));
            (*file_list)[*file_count - 1] = strdup(entry->d_name);
        }
    }

    closedir(dp);
    return 0;
}
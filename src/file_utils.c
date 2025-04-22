#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "include/file_utils.h"
#include "include/COOMatrix.h"
#include "include/CSRMatrix.h"
#include "include/ELLPACKMatrix.h"
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <unistd.h>
#include <math.h>


void print_curr_dir(){
    char cwd[4096];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        exit(1);
    }

}

char* get_filename_from_path(const char* path) {
    if (!path) return NULL;

    const char* last_slash = strrchr(path, '/');
    const char* filename = last_slash ? last_slash + 1 : path;

    char* result = malloc(strlen(filename) + 1);
    if (!result) {
        perror("malloc fallita");
        return NULL;
    }

    strcpy(result, filename);
    return result;
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
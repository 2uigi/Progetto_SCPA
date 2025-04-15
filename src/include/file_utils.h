#ifndef FILE_UTILS_H
#define FILE_UTILS_H

int list_mtx_files(const char *directory, char ***file_list, int *file_count);

void print_curr_dir();

void process_files(const char* matrice_path);

#endif

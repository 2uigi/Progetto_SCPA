#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "include/store_performance.h"

void report_performance_to_csv(performance_parameters* p_p) {
            

    // Estrai il nome base del file della matrice
    const char *base_name = strrchr(p_p->matrix_filename, '/');
    base_name = base_name ? base_name + 1 : p_p->matrix_filename;

    char matrix_name[256];
    strncpy(matrix_name, base_name, sizeof(matrix_name));
    matrix_name[sizeof(matrix_name) - 1] = '\0';

    char *dot = strrchr(matrix_name, '.');
    if (dot) *dot = '\0';

    // Directory di destinazione
    const char *output_dir = "performance_csv";

    // Costruisci il path completo del file CSV
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s_performance.csv", output_dir, matrix_name);

    // Verifica se il file esiste già per scrivere l'intestazione solo la prima volta
    bool new_file = (access(filename, F_OK) != 0);

    // Ottieni il timestamp corrente
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Calcolo FLOPS
    double flops = (2.0 * p_p->NZ) / p_p->avg_time_sec;
    double mflops = flops / 1e6;
    double gflops = flops / 1e9;

    // Apri file in modalità append
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        perror("Errore apertura file CSV");
        exit(EXIT_FAILURE);
    }

    // Scrivi intestazione se nuovo file
    if (new_file) {
        fprintf(fp, "timestamp,matrix,nz,repetitions,threads,avg_time_sec,flops_mflops,flops_gflops\n");
    }

    // Scrivi la riga dei dati
    fprintf(fp, "%s,%s,%d,%d,%d,%.6f,%.2f,%.6f\n",
            timestamp, matrix_name, p_p->NZ, p_p->repetitions, p_p->num_threads,
            p_p->avg_time_sec, mflops, gflops);

    fclose(fp);
}



void report_performance(double avg_time_sec, int NZ) {
    double flops = (2.0 * NZ) / avg_time_sec;

    if (flops < 1e9)
        printf("Performance: %.2f MFLOPS\n", flops / 1e6);
    else
        printf("Performance: %.2f GFLOPS\n", flops / 1e9);
}


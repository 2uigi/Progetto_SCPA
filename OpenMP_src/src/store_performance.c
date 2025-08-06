#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "include/store_performance.h"

void report_performance_to_csv(performance_parameters* p_p) {
    // Estrai nome base
    const char *base_name = strrchr(p_p->matrix_filename, '/');
    base_name = base_name ? base_name + 1 : p_p->matrix_filename;
    char matrix_name[256];
    snprintf(matrix_name, sizeof(matrix_name), "%s", base_name);
    matrix_name[sizeof(matrix_name)-1] = '\0';
    char *dot = strrchr(matrix_name, '.');
    if (dot) *dot = '\0';

    // Prepara directory e file CSV
    const char *output_dir = "performance_csv";
    mkdir(output_dir, 0777);
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s_performance.csv",
             output_dir, matrix_name);

    // CSV header se necessario
    bool new_file = (access(filename, F_OK) != 0);
    FILE *fp = fopen(filename, "a");
    if (!fp) {
        perror("Errore apertura file CSV");
        exit(EXIT_FAILURE);
    }
    if (new_file) {
        fprintf(fp,
          "timestamp,matrix,nz,repetitions,iterations,threads,avg_time_sec,flops_mflops,flops_gflops\n");
    }

    // Dati numerici
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    double flops = (2.0 * p_p->NZ) / p_p->avg_time_sec;
    double mflops = flops / 1e6;
    double gflops = flops / 1e9;

    // Scrivi la riga CSV
    fprintf(fp,
      "%s,%s,%d,%d,%lld,%d,%.6f,%.2f,%.6f\n",
      timestamp,
      matrix_name,
      p_p->NZ,
      p_p->repetitions,
      p_p->iterations,
      p_p->num_threads,
      p_p->avg_time_sec,
      mflops,
      gflops);

    // --- AdessoLa tabella ASCII, sia su stdout che nel file ---
    const char *sep = "+--------------+----------------------+\n";
    const char *row_fmt = "| %-12s | %-20s |\n";
    char buf[32];

    // funzione inline per doppia stampa
    #define DPRINT(fmt, ...)     \
        do {                      \
            printf(fmt, __VA_ARGS__); \
            fprintf(fp, fmt, __VA_ARGS__); \
        } while(0)

    // righe della tabella
    DPRINT("%s", sep);
    DPRINT(row_fmt, "Parameter", "Value");
    DPRINT("%s", sep);

    DPRINT(row_fmt, "Matrix", matrix_name);

    snprintf(buf, sizeof(buf), "%d", p_p->NZ);
    DPRINT(row_fmt, "Nonzeros", buf);

    snprintf(buf, sizeof(buf), "%d", p_p->repetitions);
    DPRINT(row_fmt, "Repetitions", buf);

    snprintf(buf, sizeof(buf), "%lld", p_p->iterations);
    DPRINT(row_fmt, "Iterations", buf);

    snprintf(buf, sizeof(buf), "%d", p_p->num_threads);
    DPRINT(row_fmt, "Threads", buf);

    snprintf(buf, sizeof(buf), "%.6f", p_p->avg_time_sec);
    DPRINT(row_fmt, "Avg Time(s)", buf);

    snprintf(buf, sizeof(buf), "%.2f MFLOPS", mflops);
    DPRINT(row_fmt, "MFLOPS", buf);

    snprintf(buf, sizeof(buf), "%.6f GFLOPS", gflops);
    DPRINT(row_fmt, "GFLOPS", buf);

    DPRINT("%s", sep);

    #undef DPRINT

    fclose(fp);
}

void report_performance(double avg_time_sec, int NZ) {
    double flops = (2.0 * NZ) / avg_time_sec;
    if (flops < 1e9)
        printf("Performance: %.2f MFLOPS\n", flops / 1e6);
    else
        printf("Performance: %.2f GFLOPS\n", flops / 1e9);
}

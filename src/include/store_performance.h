#if !defined(STORE_PERFORMANCE_H)
#define STORE_PERFORMANCE_H

typedef struct { 
    double avg_time_sec;
    int NZ;
    int repetitions;
    int num_threads;
    const char* matrix_filename;
}performance_parameters;

void report_performance_to_csv(performance_parameters*);

void report_performance(double avg_time_sec, int NZ);

#endif // STORE_PERFORMANCE_H

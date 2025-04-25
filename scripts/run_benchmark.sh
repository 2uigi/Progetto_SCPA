#!/bin/bash

# Parametri di esecuzione
WARMUP=10
MEASURE=100
NTHREADS=8

# OpenMP
export OMP_NUM_THREADS=$NTHREADS
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# Eventi di perf
EVENTS="cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses,context-switches,cpu-migrations"

# Verifica presenza di numactl
if ! command -v numactl &> /dev/null; then
    echo "numactl non trovato, procedo senza controllo NUMA..."
    perf stat -r 5 -e $EVENTS ./src/bin/progetto $WARMUP $MEASURE $NTHREADS
else
    echo "numactl trovato, eseguo con affinità NUMA..."
    numactl --cpunodebind=0 --membind=0 \
    perf stat -r 5 -e $EVENTS ./src/bin/progetto $WARMUP $MEASURE $NTHREADS
fi
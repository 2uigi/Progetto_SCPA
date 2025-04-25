#!/usr/bin/env bash
set -euo pipefail

### Parametri generali ----------------------------------------------------
MATRIX_DIR="matrici_cluster_Ultra_Sparse_Regular"
WARMUP=10
MEASURE=100
THREADS_LIST=(2 4 8)           # ← scegli qui
EVENTS="cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses,context-switches,cpu-migrations"

### Binari ---------------------------------------------------------------
BIN="./src/bin/progetto"       # accetta: WARMUP MEASURE NTHREADS
OUTDIR="perf_csv"
mkdir -p "$OUTDIR"

### Funzione helper: run perf su un command string -----------------------
run_perf () {
    local t=$1
    local matrix=$2
    local csv="$OUTDIR/$(basename "$matrix" .mtx)_${t}th.csv"

    export OMP_NUM_THREADS=$t
    export OMP_PLACES=cores
    export OMP_PROC_BIND=close
    omp_args="$WARMUP $MEASURE $t $matrix"   # solo se il tuo bin accetta la matrice come 4° arg.

    if command -v numactl &>/dev/null; then
        CMD="numactl --cpunodebind=0 --membind=0 $BIN $omp_args"
    else
        CMD="$BIN $omp_args"
    fi

    echo "▶  $(basename "$matrix")  |  $t threads"
    perf stat -r 5 -x, -e $EVENTS $CMD 2>&1 | tee >(grep elapsed) > "$csv"
}

### Loop su matrici e thread ---------------------------------------------
for matrix in "$MATRIX_DIR"/*.mtx; do
    for t in "${THREADS_LIST[@]}"; do
        run_perf "$t" "$matrix"
        echo "------------------------------------------------------------"
    done
done

echo "Perf CSV salvati in   $OUTDIR/"

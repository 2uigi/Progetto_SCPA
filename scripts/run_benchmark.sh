#!/usr/bin/env bash
set -euo pipefail

### Parametri generali ----------------------------------------------------
WARMUP=5
MEASURE=100
THREADS_LIST=(1)           # Solo 1 thread per run monothread
EVENTS="cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses,context-switches,cpu-migrations"

BIN="./src/bin/progetto"   # Programma compilato
OUTDIR="perf_csv"
mkdir -p "$OUTDIR"

### Controllo argomenti ---------------------------------------------------
if [ $# -ne 1 ]; then
    echo "Usage: $0 path_to_matrix.mtx"
    exit 1
fi

matrix="$1"

### Funzione helper: run perf su un command string -----------------------
run_perf () {
    local t=$1
    local matrix=$2
    local csv="$OUTDIR/$(basename "$matrix" .mtx)_${t}th.csv"

    export OMP_NUM_THREADS=$t
    export OMP_PLACES=cores
    export OMP_PROC_BIND=close
    omp_args="$WARMUP $MEASURE $t $matrix"

    if command -v numactl &>/dev/null; then
        CMD="numactl --cpunodebind=0 --membind=0 $BIN $omp_args"
    else
        CMD="$BIN $omp_args"
    fi

    echo "▶  $(basename "$matrix")  |  $t threads"
    perf stat -r 5 -x, -e $EVENTS $CMD 2>&1 | tee >(grep elapsed) > "$csv"
}

### Lancio del benchmark -------------------------------------------------
for t in "${THREADS_LIST[@]}"; do
    run_perf "$t" "$matrix"
    echo "------------------------------------------------------------"
done

echo "Perf CSV salvati in   $OUTDIR/"

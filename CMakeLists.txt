# Impostazione della versione minima di CMake
cmake_minimum_required(VERSION 3.10)

# Nome del progetto
project(ProgettoSCPA LANGUAGES C CXX)

# Opzione per abilitare/disabilitare AVX
option(ENABLE_AVX "Enable AVX and AVX2 compilation" OFF)

# Compilatori
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Flags di base per compilazione e linking
set(BASE_C_FLAGS "-I${CMAKE_SOURCE_DIR}/src/include -O3 -fopenmp -DPREFDIST=4")
set(BASE_CXX_FLAGS "-O3 -fopenmp -lm")
set(BASE_LINK_FLAGS "-fopenmp")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${BASE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${BASE_LINK_FLAGS}")

# Se ENABLE_AVX è abilitato, aggiungi i flag AVX o microarchitettura specifica
if(ENABLE_AVX)
    if(AVX512_SUPPORTED)
        # Usa -march=skylake-avx512 e fast-math su CPU compatibili
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=skylake-avx512 -ffast-math")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=skylake-avx512 -ffast-math")
        message(STATUS "Compiling with Skylake AVX-512 and fast-math support.")
    elseif(AVX2_SUPPORTED)
        # Mantieni supporto AVX2 generico
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2 -ffast-math")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -ffast-math")
        message(STATUS "Compiling with AVX2 and fast-math support.")
    else()
        message(STATUS "AVX non supportato su questo sistema. Non abilito compilazione AVX.")
    endif()
else()
    message(STATUS "AVX compilation disabled. Using generic compilation.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

# Abilita OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP: ${OpenMP_CXX_FLAGS}")
endif()

# Lista dei file sorgenti
set(SRC_FILES
    src/main.c
    src/file_utils.c
    src/mmio.c
    src/read_mtx.c
    src/convert_coo_to_csr.c
    src/convert_mtx_to_COO.c
    src/convert_coo_to_ellp.c
    src/convert_coo_to_ellp_columnwise.c
    src/random_vec.c
    src/measure_performance.c
    src/store_performance.c
    src/rcm_reordering.c
    src/spmv_csr_serial.c
    src/spmv_csr_static.c
    src/matrix_analysis.c
)

# Creazione delle cartelle di output
set(OBJ_DIR ${CMAKE_SOURCE_DIR}/src/obj)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/src/bin)
file(MAKE_DIRECTORY ${OBJ_DIR})
file(MAKE_DIRECTORY ${BIN_DIR})

# Target eseguibile
add_executable(progetto ${SRC_FILES})

# Imposta output directory
set_target_properties(progetto PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Link delle librerie necessarie
target_link_libraries(progetto m OpenMP::OpenMP_CXX)


# Impostazione della versione minima di CMake
cmake_minimum_required(VERSION 3.10)

# Nome del progetto
project(ProgettoSCPA)

# Aggiungi un'opzione per abilitare/disabilitare AVX
option(ENABLE_AVX "Enable AVX and AVX2 compilation" OFF)

# Abilita la lingua C (aggiungi CUDA se necessario)
enable_language(C)

# Impostazioni del compilatore
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Flags di compilazione
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_SOURCE_DIR}/src/include -O3 -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp -lm")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

# Se ENABLE_AVX è abilitato, aggiungi i flag AVX
if(ENABLE_AVX)
    if(AVX512_SUPPORTED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx512f")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx512f")
        message(STATUS "Compiling with AVX512F support.")
    elseif(AVX2_SUPPORTED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        message(STATUS "Compiling with AVX2 support.")
    else()
        message(STATUS "AVX is not supported on this system. Not enabling AVX compilation.")
    endif()
else()
    message(STATUS "AVX compilation disabled. Using generic compilation.")
    # Usa -march=native per ottimizzare per la CPU attuale senza causare problemi
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
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
    src/matrix_processing.c
    src/random_vec.c
    src/measure_performance.c
    src/store_performance.c
    src/rcm_reordering.c
)

# Aggiungi le directory di output per oggetti e binari
set(OBJ_DIR ${CMAKE_SOURCE_DIR}/src/obj)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# Crea le cartelle di output se non esistono
file(MAKE_DIRECTORY ${OBJ_DIR})
file(MAKE_DIRECTORY ${BIN_DIR})

# Crea il target eseguibile
add_executable(progetto ${SRC_FILES})

# Imposta il percorso di output per l'eseguibile
set_target_properties(progetto PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Link OpenMP
target_link_libraries(progetto OpenMP::OpenMP_CXX)
cmake_minimum_required(VERSION 3.21)
project(c4 VERSION 1.0 LANGUAGES CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

# The $MODE setting changes compiler options (cmake -DMODE=xyz .):
#   * OPTIMISE - (default) will produce the fastest binary possible.
#   * DEBUG    - runs with assertions and debug symbols enabled.
#   * WASM     - builds a WASM binary for the web UI.
set(MODE "OPTIMISE" CACHE STRING "Goal for the project build: OPTIMISE, DEBUG, WASM")

# Optional build-time overrides of src/solver/settings.h. If empty, the defaults
# in settings.h are used. These allow one source tree to build solvers for many
# board sizes without editing source files (cmake -DBOARD_WIDTH=7 -DBOARD_HEIGHT=5 .).
set(BOARD_WIDTH "" CACHE STRING "Override BOARD_WIDTH in settings.h")
set(BOARD_HEIGHT "" CACHE STRING "Override BOARD_HEIGHT in settings.h")
set(NUM_THREADS "" CACHE STRING "Override NUM_THREADS in settings.h")
set(NUM_TABLE_ENTRIES "" CACHE STRING "Override NUM_TABLE_ENTRIES in settings.h")

# WASM-only options. EXPORT_NAME scopes each build into its own module factory so
# several board sizes can be loaded by one page without clashing globals.
set(WASM_EXPORT_NAME "createSolverModule" CACHE STRING "Name of the generated WASM module factory")
set(WASM_INITIAL_MEMORY "1500053504" CACHE STRING "Initial WASM memory in bytes; must fit the transposition table")

# Collect the settings.h overrides which were set.
set(SETTINGS_OVERRIDES "")
if (BOARD_WIDTH)
    list(APPEND SETTINGS_OVERRIDES "C4_BOARD_WIDTH=${BOARD_WIDTH}")
endif()
if (BOARD_HEIGHT)
    list(APPEND SETTINGS_OVERRIDES "C4_BOARD_HEIGHT=${BOARD_HEIGHT}")
endif()
if (NUM_THREADS)
    list(APPEND SETTINGS_OVERRIDES "C4_NUM_THREADS=${NUM_THREADS}")
endif()
if (NUM_TABLE_ENTRIES)
    list(APPEND SETTINGS_OVERRIDES "C4_NUM_TABLE_ENTRIES=${NUM_TABLE_ENTRIES}ULL")
endif()

# Find source files.
file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/solver/*.cpp")
file(GLOB_RECURSE TSTS "${CMAKE_SOURCE_DIR}/tst/*.cpp")

set(JS_BINDINGS "${CMAKE_SOURCE_DIR}/ui/bindings.cpp")
set(JS_LIBRARY "${CMAKE_SOURCE_DIR}/ui/library.js")
set(JS_OUTPUT "${CMAKE_SOURCE_DIR}/ui/artifact/wasm" CACHE PATH "Output directory of the compiled WASM module")

# Check if we are building WASM.
if (MODE MATCHES "WASM")
    message(STATUS "Compiling with Emscripten.")

    # Setup the WASM target.
    add_executable(solver ${SRCS} ${JS_BINDINGS})

    # Setup Emscripten flags.
    target_compile_definitions(solver PUBLIC NDEBUG ${SETTINGS_OVERRIDES})
    target_compile_options(solver PUBLIC -O3 -flto -sUSE_PTHREADS=1)
    target_link_options(solver PUBLIC -O3 -flto -sUSE_PTHREADS=1
        -lembind -sWASM=1 -sWASM_BIGINT -sINITIAL_MEMORY=${WASM_INITIAL_MEMORY}
        -sMODULARIZE=1 -sEXPORT_NAME=${WASM_EXPORT_NAME}
        --js-library ${JS_LIBRARY})

    # Move compiled output to the UI directory.
    set_target_properties(solver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${JS_OUTPUT}")

    return()
endif()

# Setup targets.
add_executable(book "${CMAKE_SOURCE_DIR}/src/book.cpp")
add_executable(c4 "${CMAKE_SOURCE_DIR}/src/c4.cpp")
add_executable(play "${CMAKE_SOURCE_DIR}/src/play.cpp")
add_executable(random "${CMAKE_SOURCE_DIR}/src/random.cpp")
add_executable(test ${TSTS})

# All executables need to be linked with the solver lib.
add_library(solver OBJECT ${SRCS})
target_compile_definitions(solver PUBLIC ${SETTINGS_OVERRIDES})
target_link_libraries(book PRIVATE solver)
target_link_libraries(c4 PRIVATE solver)
target_link_libraries(play PRIVATE solver)
target_link_libraries(random PRIVATE solver)
target_link_libraries(test PRIVATE solver)

# Add compiler settings depending on target machine and $MODE.
if (MSVC)
    message(STATUS "Compiling with MSVC in ${MODE} mode.")

    if (MODE MATCHES "OPTIMISE")
        target_compile_definitions(solver PUBLIC NDEBUG)
        target_compile_options(solver PUBLIC /W4 /Ox /GL)
        target_link_options(solver PUBLIC /GL)
    elseif(MODE MATCHES "DEBUG")
        target_compile_options(solver PUBLIC /W4 /Zi)
    else()
        message(FATAL_ERROR "Mode $MODE not recognised.")
    endif()
else()
    message(STATUS "Compiling with GCC/Clang in ${MODE} mode.")
    
    if (MODE MATCHES "OPTIMISE")
        target_compile_definitions(solver PUBLIC NDEBUG)
        target_compile_options(solver PUBLIC -Wall -O3 -flto)
        target_link_options(solver PUBLIC -flto=full)
    elseif(MODE MATCHES "DEBUG")
        target_compile_options(solver PUBLIC -Wall -g)
    else()
        message(FATAL_ERROR "Mode $MODE not recognised.")
    endif()
endif()

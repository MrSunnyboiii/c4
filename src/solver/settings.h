#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <cstdint>

// Defines settings which can be tuned for the target machine and target problem.
// In rough order of importance.

// The shape of the board. Can be overridden at build time by defining C4_BOARD_WIDTH and
// C4_BOARD_HEIGHT (see the BOARD_WIDTH/BOARD_HEIGHT CMake options), which allows one source
// tree to produce solvers for many board sizes.
#ifdef C4_BOARD_WIDTH
inline constexpr int BOARD_WIDTH = C4_BOARD_WIDTH;
#else
inline constexpr int BOARD_WIDTH = 7;
#endif

#ifdef C4_BOARD_HEIGHT
inline constexpr int BOARD_HEIGHT = C4_BOARD_HEIGHT;
#else
inline constexpr int BOARD_HEIGHT = 6;
#endif

// Enable/disable strong solutions:
//   * Weak solution: Will find if either player can force a win or if the game will be a draw after
//     perfect play. Does not give the move on which the game will end.
//
//   * Strong solution: Gives the weak solution plus the move on which the game will end assuming
//     perfect play. Slower than a weak solution, and requires more memory.
inline constexpr bool STRONG_SOLVES_ENABLED = true;

// Number of search threads. If 0, the number concurrent threads available on the machine is used.
// Can be overridden at build time by defining C4_NUM_THREADS (see the NUM_THREADS CMake option).
#ifdef C4_NUM_THREADS
inline constexpr int NUM_THREADS = C4_NUM_THREADS;
#else
inline constexpr int NUM_THREADS = 0;
#endif

// This setting controls the amount of memory the solver is allowed to use. Increasing the number
// of entries will reduce solve times significantly.
//
// The transposition table uses the Chinese Remainer Theorem to reduce the number of bits per entry.
// For this to work, the size of the table must be odd. The size of the table should be a prime
// number for fewer collisions.
//
// Some example prime numbers for table sizes, with the memory requirements:
//  * 8388617    : 64 MB
//  * 134217757  :  1 GB
//  * 536870923  :  4 GB
//  * 1073741827 :  8 GB
//  * 4294967311 : 32 GB
//  * 6442450967 : 48 GB
//  * 7247757317 : 54 GB
//
// Can be overridden at build time by defining C4_NUM_TABLE_ENTRIES (see the NUM_TABLE_ENTRIES
// CMake option). Browser builds use a smaller table to fit in WASM memory limits.
#ifdef C4_NUM_TABLE_ENTRIES
inline constexpr uint64_t NUM_TABLE_ENTRIES = C4_NUM_TABLE_ENTRIES;
#else
inline constexpr uint64_t NUM_TABLE_ENTRIES = 134217757;
#endif

// Enable 2 MB pages, instead of 4 KB. Not implemented for Macs.
inline constexpr bool ENABLE_HUGE_PAGES = false;

// Restrict each search thread to a single core. Only implemented on Windows.
inline constexpr bool ENABLE_AFFINITY = false;

// At depths higher than this value, the search will do a transposition table
// lookup for each child in hope of tightening bounds or finding a cut off.
inline constexpr int ENHANCED_TABLE_CUTOFF_PLIES = BOARD_WIDTH * BOARD_HEIGHT - 15;

// Determines how much noise to add to move scores near the root of the search tree
// when searching with multiple threads. This noise helps threads to desync.
// Only used when running with more than one search thread.
inline constexpr float MOVE_SCORE_JITTER = 0.3f;

// Whether an opening book should be read into the transposition table before solving any positions.
inline constexpr bool LOAD_BOOK_FILE = false;

// Table files contain significant results (nodes with millions of child nodes) which are used to
// speed up future runs. These settings control the use of these files.
inline constexpr unsigned long long MIN_NODES_FOR_TABLE_FILE = 1 * 1000 * 1000;
inline constexpr bool LOAD_TABLE_FILE = false;
inline constexpr bool UPDATE_TABLE_FILE = false;

static_assert(!(LOAD_BOOK_FILE && LOAD_TABLE_FILE), "Cannot load an opening book and a table file.");

#endif

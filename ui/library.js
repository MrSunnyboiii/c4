// Callbacks from the C++ side to the JS side. The C++ side invokes these from a
// search thread (a pthread worker), so each callback is proxied to the main thread
// where `Module` is the instance returned by this build's module factory.
//
// Pages receive results by assigning a handler to `Module.onSolverMessage`. Using a
// module-scoped callback (instead of e.g. a BroadcastChannel) keeps replies isolated
// to the page and module instance that started the solve, even if several tabs or
// several solver modules are open at once.
addToLibrary({
    solve_callback__proxy: 'async',
    solve_callback: (score, bestMove, movesLeft) => {
        Module["onSolverMessage"]?.({ type: "solved", score, bestMove, movesLeft });
    },

    win_callback__proxy: 'async',
    win_callback: (score) => {
        Module["onSolverMessage"]?.({ type: "won", score });
    },

    cancelled_callback__proxy: 'async',
    cancelled_callback: () => {
        Module["onSolverMessage"]?.({ type: "cancelled" });
    },
});

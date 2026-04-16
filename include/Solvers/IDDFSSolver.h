#pragma once
// ============================================================================
//  IDDFSSolver  --  Iterative Deepening DFS.
//
//  Runs depth-limited DFS with limit 0, 1, 2, ... until a solution appears.
//  * Optimal like BFS (the first solution found is the shortest).
//  * Memory O(d) like DFS.
//  * Re-doing the shallow levels costs only ~b/(b-1) extra (about 6-8%),
//    because the deepest level dominates an exponential tree.
// ============================================================================
#include <cstdint>
#include <iostream>
#include <vector>

#include "DFSSolver.h"

template <typename T>
class IDDFSSolver {
public:
    IDDFSSolver(const T& root, int maxSearchDepth = 20, bool verbose = false)
        : rubiksCube(root), maxSearchDepth(maxSearchDepth), verbose(verbose) {}

    bool solve(std::vector<RubiksCube::MOVE>& out) {
        for (int depth = 0; depth <= maxSearchDepth; ++depth) {
            DFSSolver<T> dfs(rubiksCube, depth);
            bool found = dfs.solve(out);
            nodes += dfs.nodesExplored();
            if (verbose)
                std::cout << "  IDDFS depth " << depth << " done, nodes so far: " << nodes << '\n';
            if (found) return true;
        }
        return false;
    }

    uint64_t nodesExplored() const { return nodes; }

private:
    T rubiksCube;
    int maxSearchDepth;
    bool verbose;
    uint64_t nodes = 0;
};

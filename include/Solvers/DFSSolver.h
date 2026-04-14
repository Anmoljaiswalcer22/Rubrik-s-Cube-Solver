#pragma once
// ============================================================================
//  DFSSolver  --  Depth-limited Depth-First Search.
//
//  Goes as deep as possible down one path, then backtracks.
//  Recursive: make a move -> recurse -> UNDO the move with its inverse
//  (no copying of the cube needed).
//  * Memory O(d): only the current path.
//  * Does NOT guarantee the shortest solution.
//  * Needs a depth limit, because the cube graph has cycles.
// ============================================================================
#include <cstdint>
#include <vector>

#include "MoveUtils.h"
#include "RubiksCube.h"

template <typename T>
class DFSSolver {
public:
    DFSSolver(const T& root, int maxSearchDepth) : rubiksCube(root), maxSearchDepth(maxSearchDepth) {}

    // Returns true if a solution within maxSearchDepth moves was found.
    bool solve(std::vector<RubiksCube::MOVE>& out) {
        moves.clear();
        bool found = dfs(0, -1);
        if (found) out = moves;
        return found;
    }

    uint64_t nodesExplored() const { return nodes; }

private:
    T rubiksCube;
    int maxSearchDepth;
    uint64_t nodes = 0;
    std::vector<RubiksCube::MOVE> moves;

    bool dfs(int depth, int lastFace) {
        ++nodes;
        if (rubiksCube.isSolved()) return true;
        if (depth == maxSearchDepth) return false;

        for (int i = 0; i < RubiksCube::NUM_MOVES; ++i) {
            auto m = static_cast<RubiksCube::MOVE>(i);
            const int face = RubiksCube::moveFace(m);
            if (isRedundant(face, lastFace)) continue;

            applyMove(rubiksCube, m);
            moves.push_back(m);
            if (dfs(depth + 1, face)) return true;
            moves.pop_back();
            applyInverse(rubiksCube, m);     // backtrack
        }
        return false;
    }
};

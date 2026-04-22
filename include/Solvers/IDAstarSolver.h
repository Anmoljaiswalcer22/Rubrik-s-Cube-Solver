#pragma once
// ============================================================================
//  IDAstarSolver  --  Korf's Iterative Deepening A* (1985 / Rubik's 1997).
//
//  Like IDDFS, but the limit is on f = g + h instead of depth:
//     g = moves made so far,  h = heuristic (corner pattern database)
//  Algorithm:
//     threshold = h(start)
//     repeat:
//        DFS; at each state compute f = g + h
//           f > threshold  -> prune, remember the smallest such f
//           state solved   -> return the path
//        threshold = smallest f that exceeded it
//  With an admissible h (never overestimates) the solution is OPTIMAL,
//  and memory stays O(d) -- no open/closed lists like A*.
// ============================================================================
#include <climits>
#include <cstdint>
#include <iostream>
#include <vector>

#include "MoveUtils.h"
#include "PatternDatabases/CornerPatternDatabase.h"
#include "RubiksCube.h"

template <typename T>
class IDAstarSolver {
public:
    IDAstarSolver(const T& root, const CornerPatternDatabase& db, bool verbose = false)
        : rubiksCube(root), cornerDB(db), verbose(verbose) {}

    std::vector<RubiksCube::MOVE> solve() {
        int threshold = cornerDB.getNumMoves(rubiksCube);
        moves.clear();
        while (true) {
            if (verbose)
                std::cout << "  IDA* threshold " << threshold << " (nodes so far: " << nodes << ")\n";
            int t = search(0, threshold, -1);
            if (t == FOUND) return moves;
            if (t == INT_MAX) return {};  // unreachable for a valid cube
            threshold = t;
        }
    }

    uint64_t nodesExplored() const { return nodes; }

private:
    static constexpr int FOUND = -1;

    T rubiksCube;
    const CornerPatternDatabase& cornerDB;
    bool verbose;
    uint64_t nodes = 0;
    std::vector<RubiksCube::MOVE> moves;

    int search(int g, int threshold, int lastFace) {
        ++nodes;
        const int h = cornerDB.getNumMoves(rubiksCube);
        const int f = g + h;
        if (f > threshold) return f;                    // prune this branch
        if (h == 0 && rubiksCube.isSolved()) return FOUND;

        int minNext = INT_MAX;
        for (int i = 0; i < RubiksCube::NUM_MOVES; ++i) {
            auto m = static_cast<RubiksCube::MOVE>(i);
            const int face = RubiksCube::moveFace(m);
            if (isRedundant(face, lastFace)) continue;

            applyMove(rubiksCube, m);
            moves.push_back(m);
            int t = search(g + 1, threshold, face);
            if (t == FOUND) return FOUND;
            if (t < minNext) minNext = t;
            moves.pop_back();
            applyInverse(rubiksCube, m);
        }
        return minNext;
    }
};

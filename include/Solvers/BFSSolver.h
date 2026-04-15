#pragma once
// ============================================================================
//  BFSSolver  --  Breadth-First Search.
//
//  Explores level by level (all states 1 move away, then 2, ...).
//  * Always finds the SHORTEST solution.
//  * Memory O(b^d): every visited state is stored -> explodes beyond ~6-7 moves.
//
//  T = cube model,  H = hash function for that model.
//  The visited map stores, for every state, the move used to reach it, so the
//  path can be rebuilt by walking backwards from the solved state.
// ============================================================================
#include <algorithm>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "MoveUtils.h"
#include "RubiksCube.h"

template <typename T, typename H>
class BFSSolver {
public:
    explicit BFSSolver(const T& root, size_t maxStates = 20'000'000)
        : rubiksCube(root), maxStates(maxStates) {}

    // Returns the optimal move sequence. Throws std::runtime_error if the
    // number of stored states exceeds maxStates (memory guard).
    std::vector<RubiksCube::MOVE> solve() {
        T solved = bfs();
        // Path reconstruction: walk back from solved to root applying inverses
        T cur = solved;
        moves.clear();
        while (!(cur == rubiksCube)) {
            RubiksCube::MOVE m = moveDone.at(cur);
            moves.push_back(m);
            applyInverse(cur, m);
        }
        std::reverse(moves.begin(), moves.end());
        return moves;
    }

    size_t statesExplored() const { return explored; }
    size_t statesStored() const { return moveDone.size() + 1; }

private:
    T rubiksCube;
    size_t maxStates;
    size_t explored = 0;
    std::vector<RubiksCube::MOVE> moves;
    // visited map: state -> move used to reach it (the root has no entry)
    std::unordered_map<T, RubiksCube::MOVE, H> moveDone;

    bool isVisited(const T& c) const { return c == rubiksCube || moveDone.count(c); }

    T bfs() {
        if (rubiksCube.isSolved()) return rubiksCube;
        std::queue<T> q;
        q.push(rubiksCube);

        while (!q.empty()) {
            T node = q.front();
            q.pop();
            ++explored;

            int lastFace = -1;
            auto it = moveDone.find(node);
            if (it != moveDone.end()) lastFace = RubiksCube::moveFace(it->second);

            for (int i = 0; i < RubiksCube::NUM_MOVES; ++i) {
                auto m = static_cast<RubiksCube::MOVE>(i);
                if (isRedundant(RubiksCube::moveFace(m), lastFace)) continue;
                applyMove(node, m);
                if (!isVisited(node)) {
                    moveDone[node] = m;
                    // goal test on generation: stop as soon as solved is reached
                    if (node.isSolved()) return node;
                    q.push(node);
                    if (moveDone.size() > maxStates)
                        throw std::runtime_error("BFS: memory limit reached (" +
                                                 std::to_string(maxStates) + " states)");
                }
                applyInverse(node, m);
            }
        }
        throw std::runtime_error("BFS: no solution found");
    }
};

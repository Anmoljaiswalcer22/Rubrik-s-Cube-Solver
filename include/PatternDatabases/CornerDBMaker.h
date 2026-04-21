#pragma once
// ============================================================================
//  CornerDBMaker  --  builds the corner pattern database with a BFS outward
//  from the solved cube.
//
//  The corner permutation and the corner orientation change independently
//  under a move, so we pre-compute two small move tables:
//      permMove[40320][18]   and   orientMove[2187][18]
//  (derived from the real cube model, so they match it exactly).
//  Then the BFS works directly on indices:
//      next = permMove[p][m] * 2187 + orientMove[o][m]
//  Layer by layer: every state at depth d expands to depth d+1 if unseen.
// ============================================================================
#include <string>
#include <vector>

#include "CornerPatternDatabase.h"

class CornerDBMaker {
public:
    // Builds the database into `db`. Returns the per-depth state counts.
    static std::vector<uint64_t> build(CornerPatternDatabase& db, bool verbose = true);

    // Load from file if it exists, otherwise build it and save it.
    static bool loadOrBuild(CornerPatternDatabase& db, const std::string& path, bool verbose = true);

    // Move effect on the 8 corner positions (exposed for tests):
    // after move m, position i holds the piece that was in cp[m][i],
    // with its twist increased by co[m][i].
    struct MoveEffect { uint8_t cp[8]; uint8_t co[8]; };
    static std::vector<MoveEffect> computeMoveEffects();

    static void buildMoveTables(std::vector<uint16_t>& permMove, std::vector<uint16_t>& orientMove);
};

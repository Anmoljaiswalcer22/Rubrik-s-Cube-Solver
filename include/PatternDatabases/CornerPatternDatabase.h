#pragma once
// ============================================================================
//  CornerPatternDatabase  --  the IDA* heuristic.
//
//  For every arrangement of the 8 corners it stores the EXACT minimum number
//  of moves needed to solve the corners (edges ignored). Solving just the
//  corners can never take more moves than solving the whole cube, so the
//  heuristic is ADMISSIBLE (and consistent).
//
//  Size:  8! permutations (40,320) x 3^7 orientations (2,187)
//         = 88,179,840 states, 4 bits each  ->  ~44 MB
//
//  Index: permutationRank (Lehmer code) * 2187 + orientationIndex (base 3)
//         -> perfect, collision-free, O(1) lookup.
// ============================================================================
#include <cstdint>
#include <string>

#include "CubeGeometry.h"
#include "NibbleArray.h"
#include "PermutationIndexer.h"

class CornerPatternDatabase {
public:
    static constexpr uint32_t NUM_PERMS   = 40320;     // 8!
    static constexpr uint32_t NUM_ORIENTS = 2187;      // 3^7
    static constexpr uint32_t NUM_STATES  = NUM_PERMS * NUM_ORIENTS;  // 88,179,840
    static constexpr uint8_t  EMPTY       = 0xF;

    CornerPatternDatabase() : db(NUM_STATES, EMPTY) {}

    // corner state -> unique index
    static uint32_t permRank(const uint8_t perm[8]) { return PermutationIndexer<8>::rank(perm); }
    static uint32_t orientIndex(const uint8_t ori[8]) {
        uint32_t o = 0;
        for (int i = 0; i < 7; ++i) o = o * 3 + ori[i];   // the 8th is implied
        return o;
    }
    static uint32_t encode(const uint8_t perm[8], const uint8_t ori[8]) {
        return permRank(perm) * NUM_ORIENTS + orientIndex(ori);
    }

    template <typename Cube>
    static uint32_t getIndex(const Cube& cube) {
        uint8_t perm[8], ori[8];
        geom::readCorners(cube, perm, ori);
        return encode(perm, ori);
    }

    template <typename Cube>
    uint8_t getNumMoves(const Cube& cube) const { return db.get(getIndex(cube)); }

    uint8_t getNumMoves(uint32_t index) const { return db.get(index); }
    void    setNumMoves(uint32_t index, uint8_t moves) { db.set(index, moves); }

    bool save(const std::string& path) const;
    bool load(const std::string& path);

    NibbleArray& raw() { return db; }
    const NibbleArray& raw() const { return db; }

private:
    NibbleArray db;
};

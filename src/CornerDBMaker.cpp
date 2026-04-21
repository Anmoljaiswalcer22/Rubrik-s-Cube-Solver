#include "PatternDatabases/CornerDBMaker.h"

#include <chrono>
#include <iostream>

#include "MoveUtils.h"
#include "RubiksCube3dArray.h"

using MOVE = RubiksCube::MOVE;

std::vector<CornerDBMaker::MoveEffect> CornerDBMaker::computeMoveEffects() {
    std::vector<MoveEffect> fx(RubiksCube::NUM_MOVES);
    for (int m = 0; m < RubiksCube::NUM_MOVES; ++m) {
        RubiksCube3dArray c;
        applyMove(c, static_cast<MOVE>(m));
        geom::readCorners(c, fx[m].cp, fx[m].co);
    }
    return fx;
}

void CornerDBMaker::buildMoveTables(std::vector<uint16_t>& permMove, std::vector<uint16_t>& orientMove) {
    const auto fx = computeMoveEffects();
    const int M = RubiksCube::NUM_MOVES;
    permMove.assign(CornerPatternDatabase::NUM_PERMS * M, 0);
    orientMove.assign(CornerPatternDatabase::NUM_ORIENTS * M, 0);

    uint8_t perm[8], next[8];
    for (uint32_t p = 0; p < CornerPatternDatabase::NUM_PERMS; ++p) {
        PermutationIndexer<8>::unrank(p, perm);
        for (int m = 0; m < M; ++m) {
            for (int i = 0; i < 8; ++i) next[i] = perm[fx[m].cp[i]];
            permMove[p * M + m] = static_cast<uint16_t>(PermutationIndexer<8>::rank(next));
        }
    }

    uint8_t ori[8], nori[8];
    for (uint32_t o = 0; o < CornerPatternDatabase::NUM_ORIENTS; ++o) {
        uint32_t x = o, sum = 0;
        for (int i = 6; i >= 0; --i) { ori[i] = static_cast<uint8_t>(x % 3); x /= 3; sum += ori[i]; }
        ori[7] = static_cast<uint8_t>((3 - sum % 3) % 3);   // total twist is always 0 mod 3
        for (int m = 0; m < M; ++m) {
            for (int i = 0; i < 8; ++i) nori[i] = static_cast<uint8_t>((ori[fx[m].cp[i]] + fx[m].co[i]) % 3);
            orientMove[o * M + m] = static_cast<uint16_t>(CornerPatternDatabase::orientIndex(nori));
        }
    }
}

std::vector<uint64_t> CornerDBMaker::build(CornerPatternDatabase& db, bool verbose) {
    using clock = std::chrono::steady_clock;
    const auto t0 = clock::now();
    const int M = RubiksCube::NUM_MOVES;
    const uint32_t N = CornerPatternDatabase::NUM_STATES;
    const uint32_t NO = CornerPatternDatabase::NUM_ORIENTS;

    std::vector<uint16_t> permMove, orientMove;
    buildMoveTables(permMove, orientMove);

    // Build in a byte array (fast), then pack into the nibble array.
    std::vector<uint8_t> depth(N, CornerPatternDatabase::EMPTY);
    RubiksCube3dArray solved;
    depth[CornerPatternDatabase::getIndex(solved)] = 0;

    std::vector<uint64_t> counts{1};
    uint64_t total = 1;
    if (verbose) std::cout << "  depth  0: 1 state\n";

    for (uint8_t d = 0; total < N; ++d) {
        uint64_t added = 0;
        for (uint32_t idx = 0; idx < N; ++idx) {
            if (depth[idx] != d) continue;
            const uint32_t p = idx / NO, o = idx % NO;
            const uint16_t* pm = &permMove[p * M];
            const uint16_t* om = &orientMove[o * M];
            for (int m = 0; m < M; ++m) {
                const uint32_t ni = static_cast<uint32_t>(pm[m]) * NO + om[m];
                if (depth[ni] == CornerPatternDatabase::EMPTY) {
                    depth[ni] = static_cast<uint8_t>(d + 1);
                    ++added;
                }
            }
        }
        if (added == 0) break;
        total += added;
        counts.push_back(added);
        if (verbose) {
            const double s = std::chrono::duration<double>(clock::now() - t0).count();
            std::cout << "  depth " << (d + 1 < 10 ? " " : "") << int(d + 1) << ": " << added
                      << " states  (total " << total << ", " << s << " s)\n";
        }
    }

    NibbleArray& nib = db.raw();
    for (uint32_t i = 0; i < N; ++i) nib.set(i, depth[i]);
    return counts;
}

bool CornerDBMaker::loadOrBuild(CornerPatternDatabase& db, const std::string& path, bool verbose) {
    if (db.load(path)) {
        if (verbose) std::cout << "Loaded corner pattern database from " << path << '\n';
        return true;
    }
    if (verbose) std::cout << "Building corner pattern database (one-time, BFS from solved)...\n";
    build(db, verbose);
    if (db.save(path)) {
        if (verbose) std::cout << "Saved database to " << path << '\n';
    } else if (verbose) {
        std::cout << "Warning: could not save database to " << path << '\n';
    }
    return true;
}

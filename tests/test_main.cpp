// ============================================================================
//  Unit tests (no external framework needed):  make test
// ============================================================================
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>

#include "PatternDatabases/CornerDBMaker.h"
#include "RubiksCube1dArray.h"
#include "RubiksCube3dArray.h"
#include "RubiksCubeBitboard.h"
#include "Solvers/BFSSolver.h"
#include "Solvers/DFSSolver.h"
#include "Solvers/IDAstarSolver.h"
#include "Solvers/IDDFSSolver.h"

using MOVE = RubiksCube::MOVE;

static int failures = 0, passes = 0;
#define CHECK(cond, msg)                                                         \
    do {                                                                         \
        if (cond) ++passes;                                                      \
        else { ++failures; std::cout << "  FAIL: " << msg << "  (" #cond ")\n"; } \
    } while (0)

template <typename T>
static void testMoveIdentities(const std::string& name) {
    std::cout << "[" << name << "] move identities\n";
    for (int i = 0; i < 18; ++i) {
        auto m = static_cast<MOVE>(i);
        T c;
        c.randomShuffleCube(20, 100 + i);
        const T start = c;
        for (int k = 0; k < 4; ++k) c.move(m);
        CHECK(c == start, name << ": " << RubiksCube::getMove(m) << " x4 != identity");
        c.move(m).invert(m);
        CHECK(c == start, name << ": " << RubiksCube::getMove(m) << " + inverse != identity");
        T a = start, b = start;
        a.move(m).move(m);
        int base = i / 3 * 3;
        b.move(static_cast<MOVE>(base + 2));
        if (i % 3 == 0) CHECK(a == b, name << ": X X != X2 for " << RubiksCube::getMove(m));
    }
    T c;
    for (int k = 0; k < 6; ++k) for (MOVE m : RubiksCube::parseMoves("R U R' U'")) c.move(m);
    CHECK(c.isSolved(), name << ": (R U R' U') x6 != solved");
    T c2;
    for (int k = 0; k < 105; ++k) c2.r().u();
    CHECK(c2.isSolved(), name << ": (R U) x105 != solved");
    T c4;
    c4.r().u();
    CHECK(!c4.isSolved(), name << ": R U should not be solved");
    // Superflip has every centre fixed and every corner solved
    T sf;
    for (MOVE m : RubiksCube::parseMoves("U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2")) sf.move(m);
    uint8_t perm[8], ori[8];
    geom::readCorners(sf, perm, ori);
    bool cornersSolved = true;
    for (int i = 0; i < 8; ++i) cornersSolved &= (perm[i] == i && ori[i] == 0);
    CHECK(cornersSolved && !sf.isSolved(), name << ": superflip should solve corners but not edges");
    int flippedEdges = 0;
    std::string s = sf.toString();
    for (int f = 0; f < 6; ++f)
        for (int p : {1, 3, 5, 7}) flippedEdges += (s[f * 9 + p] != s[f * 9 + 4]);
    CHECK(flippedEdges == 24, name << ": superflip should flip all 12 edges (24 wrong edge stickers)");
}

static void testModelsAgree() {
    std::cout << "[all] three models agree after 10,000 random moves\n";
    RubiksCube3dArray a;
    RubiksCube1dArray b;
    RubiksCubeBitboard c;
    std::mt19937 rng(1);
    bool ok = true;
    for (int i = 0; i < 10000; ++i) {
        auto m = static_cast<MOVE>(rng() % 18);
        a.move(m); b.move(m); c.move(m);
        if (a.toString() != b.toString() || b.toString() != c.toString()) { ok = false; break; }
    }
    CHECK(ok, "models diverged");
    // every colour appears exactly 9 times
    int count[6] = {};
    for (char ch : c.toString()) {
        const std::string letters = "WOGRBY";
        count[letters.find(ch)]++;
    }
    bool nine = true;
    for (int x : count) nine &= (x == 9);
    CHECK(nine, "each colour must appear 9 times");
}

template <typename T, typename H>
static void testHashing(const std::string& name) {
    std::cout << "[" << name << "] hashing & equality\n";
    T a, b;
    a.u().d();
    b.d().u();   // U D == D U
    CHECK(a == b, name << ": U D should equal D U");
    CHECK(H{}(a) == H{}(b), name << ": equal states must hash equally");
    std::unordered_set<T, H> set;
    T c;
    for (int i = 0; i < 18; ++i) { T x; x.move(static_cast<MOVE>(i)); set.insert(x); }
    CHECK(set.size() == 18, name << ": 18 single moves give 18 distinct states");
    c = a;
    CHECK(c == a, name << ": operator= copies the state");
}

static void testPermutationIndexer() {
    std::cout << "[pdb] Lehmer code rank/unrank\n";
    bool ok = true;
    uint8_t p[8], q[8];
    for (uint32_t r = 0; r < 40320; r += 7) {
        PermutationIndexer<8>::unrank(r, p);
        if (PermutationIndexer<8>::rank(p) != r) ok = false;
    }
    uint8_t id[8] = {0, 1, 2, 3, 4, 5, 6, 7}, rev[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    CHECK(ok, "rank(unrank(r)) == r");
    CHECK(PermutationIndexer<8>::rank(id) == 0, "identity has rank 0");
    CHECK(PermutationIndexer<8>::rank(rev) == 40319, "reverse has rank 8!-1");
    (void)q;
}

static void testMoveTablesMatchCube() {
    std::cout << "[pdb] corner move tables match the real cube\n";
    std::vector<uint16_t> pm, om;
    CornerDBMaker::buildMoveTables(pm, om);
    std::mt19937 rng(5);
    bool ok = true;
    RubiksCubeBitboard c;
    for (int i = 0; i < 20000 && ok; ++i) {
        const uint32_t idx = CornerPatternDatabase::getIndex(c);
        const int m = static_cast<int>(rng() % 18);
        c.move(static_cast<MOVE>(m));
        const uint32_t p = idx / 2187, o = idx % 2187;
        const uint32_t predicted = pm[p * 18 + m] * 2187u + om[o * 18 + m];
        if (predicted != CornerPatternDatabase::getIndex(c)) ok = false;
    }
    CHECK(ok, "table transition != cube transition");
}

template <typename T, typename H>
static void testSolvers(const std::string& name, const CornerPatternDatabase* db) {
    std::cout << "[" << name << "] solvers on 5-move scrambles\n";
    for (unsigned seed = 1; seed <= 3; ++seed) {
        T c;
        c.randomShuffleCube(5, seed);

        BFSSolver<T, H> bfs(c);
        auto sBfs = bfs.solve();
        T x = c; for (MOVE m : sBfs) x.move(m);
        CHECK(x.isSolved(), name << ": BFS solution invalid");

        std::vector<MOVE> sDfs;
        DFSSolver<T> dfs(c, 5);
        CHECK(dfs.solve(sDfs), name << ": DFS found nothing");
        x = c; for (MOVE m : sDfs) x.move(m);
        CHECK(x.isSolved(), name << ": DFS solution invalid");

        std::vector<MOVE> sId;
        IDDFSSolver<T> iddfs(c, 10);
        CHECK(iddfs.solve(sId), name << ": IDDFS found nothing");
        x = c; for (MOVE m : sId) x.move(m);
        CHECK(x.isSolved(), name << ": IDDFS solution invalid");
        CHECK(sId.size() == sBfs.size(), name << ": IDDFS and BFS must both be optimal");

        if (db) {
            IDAstarSolver<T> ida(c, *db);
            auto sIda = ida.solve();
            x = c; for (MOVE m : sIda) x.move(m);
            CHECK(x.isSolved(), name << ": IDA* solution invalid");
            CHECK(sIda.size() == sBfs.size(), name << ": IDA* must be optimal");
        }
    }
}

int main(int argc, char** argv) {
    const bool full = argc > 1 && std::string(argv[1]) == "--full";

    testMoveIdentities<RubiksCube3dArray>("3D array");
    testMoveIdentities<RubiksCube1dArray>("1D array");
    testMoveIdentities<RubiksCubeBitboard>("Bitboard");
    testModelsAgree();
    testHashing<RubiksCube3dArray, Hash3d>("3D array");
    testHashing<RubiksCube1dArray, Hash1d>("1D array");
    testHashing<RubiksCubeBitboard, HashBitboard>("Bitboard");
    testPermutationIndexer();
    testMoveTablesMatchCube();

    CornerPatternDatabase* db = nullptr;
    CornerPatternDatabase storage;
    if (full) {
        std::cout << "[pdb] building / loading the full corner database\n";
        CornerDBMaker::loadOrBuild(storage, "data/cornerDepth.db", false);
        db = &storage;
        uint64_t filled = 0;
        uint8_t maxDepth = 0;
        for (uint32_t i = 0; i < CornerPatternDatabase::NUM_STATES; ++i) {
            uint8_t v = storage.getNumMoves(i);
            if (v != CornerPatternDatabase::EMPTY) { ++filled; if (v > maxDepth) maxDepth = v; }
        }
        CHECK(filled == CornerPatternDatabase::NUM_STATES, "every one of the 88,179,840 states reached");
        CHECK(maxDepth == 11, "maximum corner distance is 11");
        RubiksCubeBitboard solved;
        CHECK(storage.getNumMoves(solved) == 0, "solved corners have distance 0");
    } else {
        std::cout << "(run with --full to also build the pattern database and test IDA*)\n";
    }

    testSolvers<RubiksCube3dArray, Hash3d>("3D array", db);
    testSolvers<RubiksCube1dArray, Hash1d>("1D array", db);
    testSolvers<RubiksCubeBitboard, HashBitboard>("Bitboard", db);

    std::cout << "\n" << passes << " checks passed, " << failures << " failed\n";
    return failures == 0 ? 0 : 1;
}

// ============================================================================
//  Rubik's Cube Solver -- command-line driver
//
//  ./rubiks demo                          quick tour of every model & solver
//  ./rubiks solve [options]               solve one scramble
//  ./rubiks benchmark [--depth N]         compare the 3 models
//  ./rubiks build-db [path]               build the corner pattern database
//  ./rubiks help
// ============================================================================
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "PatternDatabases/CornerDBMaker.h"
#include "RubiksCube1dArray.h"
#include "RubiksCube3dArray.h"
#include "RubiksCubeBitboard.h"
#include "Solvers/BFSSolver.h"
#include "Solvers/DFSSolver.h"
#include "Solvers/IDAstarSolver.h"
#include "Solvers/IDDFSSolver.h"

using std::cout;
using std::string;
using std::vector;
using MOVE = RubiksCube::MOVE;
using Clock = std::chrono::steady_clock;

static const string DEFAULT_DB = "data/cornerDepth.db";

struct Options {
    string model = "bitboard";
    string algo = "idastar";
    int depth = 8;
    unsigned seed = 42;
    string scramble;       // explicit scramble, overrides depth/seed
    string db = DEFAULT_DB;
    int maxDepth = 20;
    bool verbose = false;
};

static double secondsSince(Clock::time_point t) {
    return std::chrono::duration<double>(Clock::now() - t).count();
}

static void usage() {
    cout << "Rubik's Cube Solver (BFS / DFS / IDDFS / Korf's IDA*)\n\n"
            "Usage:\n"
            "  rubiks demo                       tour of all models and solvers\n"
            "  rubiks solve [options]            scramble and solve a cube\n"
            "  rubiks benchmark [--depth N]      compare 3D array / 1D array / bitboard\n"
            "  rubiks build-db [path]            build corner pattern database\n\n"
            "Solve options:\n"
            "  --model   3d | 1d | bitboard      (default bitboard)\n"
            "  --algo    bfs | dfs | iddfs | idastar   (default idastar)\n"
            "  --depth   N                       random scramble length (default 8)\n"
            "  --seed    S                       random seed (default 42)\n"
            "  --scramble \"R U R' U'\"            explicit scramble instead of random\n"
            "  --max-depth N                     DFS / IDDFS depth limit (default 20)\n"
            "  --db      path                    pattern database file (default " << DEFAULT_DB << ")\n"
            "  --verbose                         print search progress\n";
}

// ---------------------------------------------------------------------------
//  Generic solve: works for any cube model T with hash H
// ---------------------------------------------------------------------------
template <typename T, typename H>
static int runSolve(const Options& opt) {
    T cube;
    vector<MOVE> scramble;
    if (!opt.scramble.empty()) {
        scramble = RubiksCube::parseMoves(opt.scramble);
        for (MOVE m : scramble) cube.move(m);
    } else {
        scramble = cube.randomShuffleCube(opt.depth, opt.seed);
    }

    cout << "Model: " << opt.model << "   Algorithm: " << opt.algo << '\n';
    cout << "Scramble (" << scramble.size() << " moves): " << RubiksCube::movesToString(scramble) << "\n\n";
    cube.print();

    vector<MOVE> solution;
    bool found = false;
    uint64_t nodes = 0;
    double secs = 0;

    try {
        if (opt.algo == "bfs") {
            auto t = Clock::now();
            BFSSolver<T, H> s(cube);
            solution = s.solve();
            secs = secondsSince(t);
            nodes = s.statesExplored();
            found = true;
            cout << "States stored in visited map: " << s.statesStored() << '\n';
        } else if (opt.algo == "dfs") {
            int limit = opt.scramble.empty() ? opt.depth : static_cast<int>(scramble.size());
            auto t = Clock::now();
            DFSSolver<T> s(cube, limit);
            found = s.solve(solution);
            secs = secondsSince(t);
            nodes = s.nodesExplored();
        } else if (opt.algo == "iddfs") {
            auto t = Clock::now();
            IDDFSSolver<T> s(cube, opt.maxDepth, opt.verbose);
            found = s.solve(solution);
            secs = secondsSince(t);
            nodes = s.nodesExplored();
        } else if (opt.algo == "idastar") {
            CornerPatternDatabase db;
            auto tl = Clock::now();
            CornerDBMaker::loadOrBuild(db, opt.db, true);
            cout << "Database ready in " << secondsSince(tl) << " s\n";
            auto t = Clock::now();
            IDAstarSolver<T> s(cube, db, opt.verbose);
            solution = s.solve();
            secs = secondsSince(t);
            nodes = s.nodesExplored();
            found = !solution.empty() || cube.isSolved();
        } else {
            std::cerr << "Unknown algorithm: " << opt.algo << '\n';
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    if (!found) {
        cout << "No solution found within the depth limit.\n";
        return 1;
    }

    T check = cube;
    for (MOVE m : solution) check.move(m);

    cout << "Solution (" << solution.size() << " moves): " << RubiksCube::movesToString(solution) << '\n';
    cout << "Nodes explored: " << nodes << '\n';
    cout << std::fixed << std::setprecision(3) << "Time: " << secs << " s\n";
    cout << "Verified solved: " << (check.isSolved() ? "YES" : "NO") << '\n';
    return check.isSolved() ? 0 : 1;
}

static int solveCommand(const Options& opt) {
    if (opt.model == "3d")       return runSolve<RubiksCube3dArray, Hash3d>(opt);
    if (opt.model == "1d")       return runSolve<RubiksCube1dArray, Hash1d>(opt);
    if (opt.model == "bitboard") return runSolve<RubiksCubeBitboard, HashBitboard>(opt);
    std::cerr << "Unknown model: " << opt.model << '\n';
    return 1;
}

// ---------------------------------------------------------------------------
//  Benchmark: moves/sec and an IDDFS solve on each model
// ---------------------------------------------------------------------------
template <typename T, typename H>
static void benchModel(const string& name, int depth, unsigned seed) {
    // 1. raw move throughput
    T c;
    const int N = 20'000'000;
    auto t = Clock::now();
    for (int i = 0; i < N; ++i) applyMove(c, static_cast<MOVE>(i % 18));
    double moveSecs = secondsSince(t);

    // 2. hashing throughput
    H h;
    size_t acc = 0;
    t = Clock::now();
    for (int i = 0; i < 5'000'000; ++i) { applyMove(c, static_cast<MOVE>(i % 18)); acc += h(c); }
    double hashSecs = secondsSince(t);

    // 3. IDDFS solve
    T s;
    auto scr = s.randomShuffleCube(depth, seed);
    vector<MOVE> sol;
    t = Clock::now();
    IDDFSSolver<T> solver(s, depth);
    solver.solve(sol);
    double solveSecs = secondsSince(t);

    cout << std::left << std::setw(10) << name << std::right << std::fixed << std::setprecision(1)
         << std::setw(10) << sizeof(T) << " B" << std::setw(12) << (N / moveSecs / 1e6) << " M/s"
         << std::setw(12) << (5e6 / hashSecs / 1e6) << " M/s" << std::setprecision(3) << std::setw(12)
         << solveSecs << " s" << std::setw(14) << solver.nodesExplored() << "   (" << sol.size()
         << " moves)" << (acc == 42 ? " " : "") << '\n';
}

static int benchmarkCommand(int depth, unsigned seed) {
    cout << "Benchmark: move speed, move+hash speed, and IDDFS on a " << depth << "-move scramble (seed "
         << seed << ")\n\n";
    cout << std::left << std::setw(10) << "Model" << std::right << std::setw(12) << "State size" << std::setw(16)
         << "Moves" << std::setw(16) << "Move+hash" << std::setw(14) << "IDDFS" << std::setw(14) << "Nodes"
         << '\n';
    benchModel<RubiksCube3dArray, Hash3d>("3D array", depth, seed);
    benchModel<RubiksCube1dArray, Hash1d>("1D array", depth, seed);
    benchModel<RubiksCubeBitboard, HashBitboard>("Bitboard", depth, seed);
    return 0;
}

// ---------------------------------------------------------------------------
//  Demo
// ---------------------------------------------------------------------------
static int demoCommand(const string& dbPath) {
    cout << "=== 1. The three models agree ===\n";
    RubiksCube3dArray c3;
    RubiksCube1dArray c1;
    RubiksCubeBitboard cb;
    auto scr = c3.randomShuffleCube(6, 7);
    for (MOVE m : scr) { c1.move(m); cb.move(m); }
    cout << "Scramble: " << RubiksCube::movesToString(scr) << '\n';
    cb.print();
    cout << "3D == 1D == Bitboard sticker strings: "
         << ((c3.toString() == c1.toString() && c1.toString() == cb.toString()) ? "YES" : "NO") << "\n\n";

    auto report = [](const string& name, const RubiksCubeBitboard& start, const vector<MOVE>& sol,
                     double secs, uint64_t nodes) {
        RubiksCubeBitboard chk = start;
        for (MOVE m : sol) chk.move(m);
        cout << std::left << std::setw(7) << name << std::right << std::fixed << std::setprecision(3)
             << std::setw(8) << secs << " s " << std::setw(12) << nodes << " nodes   " << std::setw(2)
             << sol.size() << " moves: " << RubiksCube::movesToString(sol)
             << (chk.isSolved() ? "   [solved]" : "   [FAILED]") << '\n';
    };

    cout << "=== 2. Blind search on a 6-move scramble (bitboard model) ===\n";
    {
        auto t = Clock::now();
        BFSSolver<RubiksCubeBitboard, HashBitboard> bfs(cb);
        auto sol = bfs.solve();
        report("BFS", cb, sol, secondsSince(t), bfs.statesExplored());
    }
    {
        vector<MOVE> sol;
        auto t = Clock::now();
        DFSSolver<RubiksCubeBitboard> dfs(cb, 6);
        dfs.solve(sol);
        report("DFS", cb, sol, secondsSince(t), dfs.nodesExplored());
    }
    {
        vector<MOVE> sol;
        auto t = Clock::now();
        IDDFSSolver<RubiksCubeBitboard> iddfs(cb, 20);
        iddfs.solve(sol);
        report("IDDFS", cb, sol, secondsSince(t), iddfs.nodesExplored());
    }

    cout << "\n=== 3. Korf's IDA* with the corner pattern database ===\n";
    CornerPatternDatabase db;
    auto tl = Clock::now();
    CornerDBMaker::loadOrBuild(db, dbPath, true);
    cout << "Database ready in " << std::setprecision(2) << secondsSince(tl) << " s\n";
    {
        auto t = Clock::now();
        IDAstarSolver<RubiksCubeBitboard> ida(cb, db);
        auto sol = ida.solve();
        report("IDA*", cb, sol, secondsSince(t), ida.nodesExplored());
    }
    for (int depth : {9, 11}) {
        RubiksCubeBitboard c;
        auto s = c.randomShuffleCube(depth, 2024);
        cout << "\nScramble (" << depth << "): " << RubiksCube::movesToString(s) << '\n';
        auto t = Clock::now();
        IDAstarSolver<RubiksCubeBitboard> ida(c, db);
        auto sol = ida.solve();
        report("IDA*", c, sol, secondsSince(t), ida.nodesExplored());
    }
    cout << "\nTry: ./rubiks solve --algo idastar --depth 13 --seed 1\n";
    return 0;
}

// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc < 2) { usage(); return 0; }
    const string cmd = argv[1];

    Options opt;
    for (int i = 2; i < argc; ++i) {
        const string a = argv[i];
        auto next = [&]() -> string {
            if (i + 1 >= argc) { std::cerr << "Missing value for " << a << '\n'; std::exit(1); }
            return argv[++i];
        };
        if (a == "--model") opt.model = next();
        else if (a == "--algo") opt.algo = next();
        else if (a == "--depth") opt.depth = std::stoi(next());
        else if (a == "--seed") opt.seed = static_cast<unsigned>(std::stoul(next()));
        else if (a == "--scramble") opt.scramble = next();
        else if (a == "--db") opt.db = next();
        else if (a == "--max-depth") opt.maxDepth = std::stoi(next());
        else if (a == "--verbose") opt.verbose = true;
        else if (cmd == "build-db" && a[0] != '-') opt.db = a;
        else { std::cerr << "Unknown option: " << a << '\n'; return 1; }
    }

    if (cmd == "help" || cmd == "--help" || cmd == "-h") { usage(); return 0; }
    if (cmd == "demo") return demoCommand(opt.db);
    if (cmd == "solve") return solveCommand(opt);
    if (cmd == "benchmark") return benchmarkCommand(opt.depth, opt.seed);
    if (cmd == "build-db") {
        CornerPatternDatabase db;
        auto t = Clock::now();
        auto counts = CornerDBMaker::build(db, true);
        cout << "Built in " << secondsSince(t) << " s, max depth " << counts.size() - 1 << ", "
             << db.raw().storageBytes() / (1024.0 * 1024.0) << " MiB\n";
        if (!db.save(opt.db)) { std::cerr << "Could not write " << opt.db << '\n'; return 1; }
        cout << "Saved to " << opt.db << '\n';
        return 0;
    }
    usage();
    return 1;
}

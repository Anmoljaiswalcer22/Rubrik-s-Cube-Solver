# Rubik's Cube Solver (Korf's IDA* Algorithm) in C++17

A virtual 3x3 Rubik's Cube with **three data representations** and **four search
algorithms**. Every solver works with every model.

| Models | Solvers | Heuristic |
|---|---|---|
| 3D array `uint8_t[6][3][3]` | BFS | Corner pattern database |
| 1D array `uint8_t[54]` | DFS (depth-limited) | 88,179,840 states, 4 bits each (~42 MiB) |
| Bitboard `uint64_t[6]` | IDDFS | Lehmer-code + base-3 perfect index |
| | **Korf's IDA\*** | built by BFS from solved in ~4 s |

## Build & run

```bash
make                 # builds ./rubiks and ./run_tests   (or: cmake -B b && cmake --build b)
make test            # fast unit tests
make test-full       # also builds the pattern DB and checks IDA* is optimal

./rubiks demo                                   # tour of everything
./rubiks build-db                               # one-time: writes data/cornerDepth.db
./rubiks solve --algo idastar --depth 13 --seed 1
./rubiks solve --algo iddfs --model 1d --depth 7
./rubiks solve --algo bfs --scramble "R U R' U' F2"
./rubiks benchmark --depth 7                    # compare the three models
```

`solve` options: `--model 3d|1d|bitboard`, `--algo bfs|dfs|iddfs|idastar`,
`--depth N`, `--seed S`, `--scramble "..."`, `--max-depth N`, `--db path`, `--verbose`.
If the database file is missing, IDA* builds and saves it automatically.

## Project layout

```
include/
  RubiksCube.h              abstract base class: pure virtual getColor/isSolved/18 moves,
                            shared move(), invert(), randomShuffleCube(), print()
  CubeGeometry.h            sticker ring tables for each face turn + corner tables
  RubiksCube3dArray.h       model 1 (+ Hash3d)
  RubiksCube1dArray.h       model 2 (+ Hash1d)
  RubiksCubeBitboard.h      model 3 (+ HashBitboard), moves templated on face
  MoveUtils.h               applyMove<T>() (compile-time dispatch) + move pruning
  Solvers/                  BFSSolver<T,H>, DFSSolver<T>, IDDFSSolver<T>, IDAstarSolver<T>
  PatternDatabases/         NibbleArray, PermutationIndexer (Lehmer code),
                            CornerPatternDatabase, CornerDBMaker
src/                        model implementations, DB build/save/load, main.cpp (CLI)
tests/test_main.cpp         framework-free unit tests
```

## Design notes

* **Moves.** Every face turn = rotate the face's 3x3 grid + shift the 12 adjacent
  stickers by one strip. `CubeGeometry.h` lists those 12 stickers per face, so all
  three models share one verified geometry.
* **Bitboard.** Centres never move, so a face has 8 moving stickers × 8 one-hot bits
  = 64 bits, stored clockwise. A clockwise face rotation is a single
  `rotl(word, 16)`; adjacent strips move with byte masks. `turn<FACE, Q>()` is a
  template so every shift and mask is a compile-time constant.
* **OOP + templates.** Models inherit from the abstract `RubiksCube` (abstraction,
  inheritance, polymorphism, encapsulation; `operator==` / `operator=` overloaded).
  Models are `final` and solvers are templates, so in the hot loop
  `applyMove<T>()` calls `c.u()` etc. with no virtual dispatch.
* **Move pruning.** No two turns of the same face in a row. Opposite faces commute,
  so only one order is allowed. This cuts the branching factor from 18 to about 13.35.
* **BFS** stores one `unordered_map<state, move>` (visited set + path
  reconstruction) and tests for the goal when a state is generated.
* **IDA\*.** `threshold = h(start)`; DFS prunes when `g + h > threshold`, then the
  threshold rises to the smallest f that exceeded it. The heuristic (corner-only
  distance) is admissible and consistent, so solutions are optimal.
* **Pattern DB build.** The corner permutation and orientation change independently
  under a move, so two small move tables (40320×18 and 2187×18, derived from the
  real cube model) let the BFS run on indices directly.

## Verified results (this machine, single core, `-O3`)

Correctness (all in `make test-full`, 234 checks):
every move ×4 = identity, move + inverse = identity, `X X = X2`,
`(R U R' U')×6` and `(R U)×105` = solved, the superflip flips all 12 edges and leaves
the corners solved, the three models agree after 10,000 random moves, the move tables
match the real cube, and BFS, IDDFS and IDA* return solutions of the same (optimal) length.

Corner database, states per depth (matches Korf's published distribution):
`1, 18, 243, 2874, 28000, 205416, 1168516, 5402628, 20776176, 45391616, 15139616, 64736`
→ 88,179,840 states, max depth 11, built in ~4.2 s, loads in ~0.01 s.

Model benchmark (`./rubiks benchmark --depth 7`, same 108.7M-node IDDFS search):

| Model | Moves/s | Move+hash/s | IDDFS 7-move |
|---|---|---|---|
| 3D array | 34 M | 23 M | 6.9 s |
| 1D array | 65 M | 31 M | 4.3 s |
| Bitboard | **251 M** | **114 M** | **1.5 s** |

(`sizeof` shows 64/64/56 bytes: 54/54/48 bytes of state, plus the vtable pointer
and padding.)

Solvers (bitboard):

| Search | Scramble | Time | Nodes |
|---|---|---|---|
| BFS | 6 moves | 5.8 s (millions of stored states) | 0.46 M expanded |
| IDDFS | 7 moves | 1.5 s | 109 M |
| IDDFS | 8 moves | 18 s | 1.37 B |
| IDA* | 11 moves | 0.15 s | 2.1 M |
| IDA* | 13 moves (seeds 1/2/3) | 2.8 s / 5.9 s / 0.1 s | 38 M / 80 M / 1.3 M |

IDA* returns optimal solutions, sometimes shorter than the scramble
(seed 1: 13-move scramble → 12-move solution).

## Possible extensions
* Edge pattern databases (two groups of 6–7 edges) with `h = max(corner, edge1, edge2)`
* Kociemba's two-phase algorithm for fast near-optimal solving of any scramble
* Multithreaded IDA* (split the first-level moves across threads)

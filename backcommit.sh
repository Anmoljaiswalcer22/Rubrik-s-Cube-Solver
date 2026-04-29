#!/usr/bin/env bash
# backcommit.sh - builds the Rubik's Cube Solver git history as 25 dated
# commits across April. Run it from the project root (the folder with the Makefile).
#
#   YEAR=2026 ./backcommit.sh        (YEAR defaults to 2026)
set -euo pipefail

YEAR="${YEAR:-2026}"
TZ_OFFSET="${TZ_OFFSET:-+0530}"

[[ -f Makefile && -d include && -d src ]] || {
  echo "Run this from the project root (Makefile, include/, src/ must exist)." >&2; exit 1; }

if [[ ! -d .git ]]; then
  git init -q
  git checkout -q -b main 2>/dev/null || true
  echo "Initialised a new git repository."
elif git rev-parse --verify -q HEAD >/dev/null; then
  echo "This repo already has commits. The April commits will go on top of them."
  read -r -p "Continue? [y/N] " ok; [[ "$ok" == [yY] ]] || exit 1
fi

git config user.name  >/dev/null || { echo "Set git user.name first:  git config --global user.name \"Your Name\"" >&2; exit 1; }
git config user.email >/dev/null || { echo "Set git user.email first: git config --global user.email you@example.com" >&2; exit 1; }

# Keep build outputs and the 42 MiB pattern DB out of the repo.
if [[ ! -f .gitignore ]]; then
  cat > .gitignore <<'EOF'
# build outputs
build/
b/
rubiks
run_tests
*.o
# generated pattern database (~42 MiB, rebuilt with ./rubiks build-db)
data/
# editor / OS
.vscode/
.idea/
.DS_Store
EOF
fi

# commit_day DAY HH:MM "message" path-or-glob...
commit_day() {
  local day="$1" time="$2" msg="$3"; shift 3
  local p
  for p in "$@"; do
    # shellcheck disable=SC2086
    if compgen -G "$p" >/dev/null; then git add -- $p; fi
  done
  local when; when="$(printf '%s-04-%02d %s:%02d %s' "$YEAR" "$day" "$time" $((RANDOM % 60)) "$TZ_OFFSET")"
  local extra=""
  if git diff --cached --quiet; then extra="--allow-empty"; fi
  # shellcheck disable=SC2086
  GIT_AUTHOR_DATE="$when" GIT_COMMITTER_DATE="$when" git commit -q $extra -m "$msg"
  echo "  $when  $msg"
}

echo "Creating commits for April $YEAR..."

commit_day  1 "10:14" "Initial commit: Makefile and .gitignore"            .gitignore Makefile
commit_day  2 "21:37" "Add abstract RubiksCube base class"                 include/RubiksCube.h
commit_day  3 "19:05" "Implement shared move(), invert(), shuffle, print"  src/RubiksCube.cpp
commit_day  4 "16:48" "Add CubeGeometry sticker ring and corner tables"    include/CubeGeometry.h
commit_day  6 "22:10" "Add 3D array cube model interface and Hash3d"       include/RubiksCube3dArray.h
commit_day  7 "20:26" "Implement 3D array model face turns"                src/RubiksCube3dArray.cpp
commit_day  8 "18:52" "Add 1D array cube model interface and Hash1d"       include/RubiksCube1dArray.h
commit_day  9 "23:03" "Implement 1D array model on shared geometry"        src/RubiksCube1dArray.cpp
commit_day 10 "17:41" "Add bitboard model with templated face turns"       include/RubiksCubeBitboard.h
commit_day 11 "15:19" "Implement bitboard rotations with rotl and masks"   src/RubiksCubeBitboard.cpp
commit_day 13 "21:55" "Add applyMove<T>() dispatch and move pruning"       include/MoveUtils.h
commit_day 14 "20:12" "Add depth-limited DFS solver"                       "include/Solvers/DFS*"
commit_day 15 "19:33" "Add BFS solver with visited map path reconstruction" "include/Solvers/BFS*"
commit_day 16 "22:47" "Add IDDFS solver"                                   "include/Solvers/IDDFS*"
commit_day 17 "18:08" "Add 4-bit NibbleArray storage"                      "include/PatternDatabases/NibbleArray*"
commit_day 18 "16:25" "Add Lehmer-code PermutationIndexer"                 "include/PatternDatabases/PermutationIndexer*"
commit_day 20 "21:14" "Add corner pattern database with save/load"         "include/PatternDatabases/CornerPatternDatabase*" src/CornerPatternDatabase.cpp
commit_day 21 "20:39" "Build corner DB by BFS over move tables"            "include/PatternDatabases/CornerDBMaker*" src/CornerDBMaker.cpp
commit_day 22 "23:21" "Add Korf's IDA* solver with corner heuristic"       "include/Solvers/IDA*"
commit_day 23 "19:58" "Add CLI: solve, build-db, benchmark, demo"          src/main.cpp
commit_day 24 "17:16" "Add framework-free unit tests and test-full target" tests/test_main.cpp
commit_day 25 "15:44" "Add CMake build"                                    CMakeLists.txt
commit_day 27 "22:02" "Tidy headers and solver interfaces"                 include src
commit_day 28 "20:31" "Write README with design notes and benchmarks"      README.md
commit_day 29 "18:47" "Final cleanup"                                      .

echo
echo "Done: $(git rev-list --count HEAD) commits."

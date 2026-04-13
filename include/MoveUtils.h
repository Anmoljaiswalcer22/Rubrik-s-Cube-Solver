#pragma once
// ============================================================================
//  MoveUtils.h  --  compile-time move dispatch + move pruning for the solvers.
//
//  RubiksCube::move() goes through virtual functions. Inside the solvers'
//  hot loops (millions of calls) we instead call applyMove<T>(): because every
//  model is declared `final`, c.u() etc. are resolved at compile time and can
//  be inlined -- no virtual dispatch overhead.
// ============================================================================
#include "RubiksCube.h"

template <typename T>
inline void applyMove(T& c, RubiksCube::MOVE m) {
    using M = RubiksCube::MOVE;
    switch (m) {
        case M::L:      c.l();      break;
        case M::LPRIME: c.lPrime(); break;
        case M::L2:     c.l2();     break;
        case M::R:      c.r();      break;
        case M::RPRIME: c.rPrime(); break;
        case M::R2:     c.r2();     break;
        case M::U:      c.u();      break;
        case M::UPRIME: c.uPrime(); break;
        case M::U2:     c.u2();     break;
        case M::D:      c.d();      break;
        case M::DPRIME: c.dPrime(); break;
        case M::D2:     c.d2();     break;
        case M::F:      c.f();      break;
        case M::FPRIME: c.fPrime(); break;
        case M::F2:     c.f2();     break;
        case M::B:      c.b();      break;
        case M::BPRIME: c.bPrime(); break;
        case M::B2:     c.b2();     break;
    }
}

template <typename T>
inline void applyInverse(T& c, RubiksCube::MOVE m) { applyMove(c, RubiksCube::getInverse(m)); }

// Move pruning (removes redundant sequences, branching factor 18 -> ~13.35):
//   * never turn the same face twice in a row   (F F = F2,  F F' = nothing)
//   * opposite faces commute (U D = D U), so allow only one order:
//     after face X, its opposite face is allowed only if it has the higher id.
// Face ids: 0=L 1=R 2=U 3=D 4=F 5=B   ->   opposite(f) = f ^ 1
inline bool isRedundant(int face, int lastFace) {
    if (lastFace < 0) return false;
    if (face == lastFace) return true;
    if ((face ^ 1) == lastFace && face < lastFace) return true;
    return false;
}

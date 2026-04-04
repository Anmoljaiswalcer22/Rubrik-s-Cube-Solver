#pragma once
// ============================================================================
//  CubeGeometry.h  --  Shared geometric tables used by all three models.
//
//  A face turn does two things:
//    1. rotates the 3x3 grid of the turned face by 90 degrees, and
//    2. cycles the 12 adjacent stickers (4 strips of 3) on the neighbouring
//       faces.
//
//  RING[face] lists those 12 stickers in order such that, on a CLOCKWISE turn
//  of `face`, the sticker at RING[face][k] moves to RING[face][(k + 3) % 12].
//  A counter-clockwise turn shifts by 9, a half turn by 6.
// ============================================================================
#include <array>
#include <cstdint>

namespace geom {

struct Sticker { uint8_t face, row, col; };

// Face ids: U=0 L=1 F=2 R=3 B=4 D=5
inline constexpr Sticker RING[6][12] = {
    // U : F top -> L top -> B top -> R top
    {{2,0,0},{2,0,1},{2,0,2}, {1,0,0},{1,0,1},{1,0,2}, {4,0,0},{4,0,1},{4,0,2}, {3,0,0},{3,0,1},{3,0,2}},
    // L : U left col -> F left col -> D left col -> B right col (reversed)
    {{0,0,0},{0,1,0},{0,2,0}, {2,0,0},{2,1,0},{2,2,0}, {5,0,0},{5,1,0},{5,2,0}, {4,2,2},{4,1,2},{4,0,2}},
    // F : U bottom row -> R left col -> D top row (rev) -> L right col (rev)
    {{0,2,0},{0,2,1},{0,2,2}, {3,0,0},{3,1,0},{3,2,0}, {5,0,2},{5,0,1},{5,0,0}, {1,2,2},{1,1,2},{1,0,2}},
    // R : F right col (up) -> U right col (up) -> B left col (down) -> D right col (up)
    {{2,2,2},{2,1,2},{2,0,2}, {0,2,2},{0,1,2},{0,0,2}, {4,0,0},{4,1,0},{4,2,0}, {5,2,2},{5,1,2},{5,0,2}},
    // B : U top row (rev) -> L left col -> D bottom row -> R right col (rev)
    {{0,0,2},{0,0,1},{0,0,0}, {1,0,0},{1,1,0},{1,2,0}, {5,2,0},{5,2,1},{5,2,2}, {3,2,2},{3,1,2},{3,0,2}},
    // D : F bottom -> R bottom -> B bottom -> L bottom
    {{2,2,0},{2,2,1},{2,2,2}, {3,2,0},{3,2,1},{3,2,2}, {4,2,0},{4,2,1},{4,2,2}, {1,2,0},{1,2,1},{1,2,2}},
};

// The 8 corner positions. Stickers are listed U/D sticker first, then
// clockwise around the corner (seen from outside). This is the standard
// orientation convention: orientation = index of the U/D-coloured sticker.
//   0 URF  1 UFL  2 ULB  3 UBR  4 DFR  5 DLF  6 DBL  7 DRB
inline constexpr Sticker CORNERS[8][3] = {
    {{0,2,2},{3,0,0},{2,0,2}},   // URF
    {{0,2,0},{2,0,0},{1,0,2}},   // UFL
    {{0,0,0},{1,0,0},{4,0,2}},   // ULB
    {{0,0,2},{4,0,0},{3,0,2}},   // UBR
    {{5,0,2},{2,2,2},{3,2,0}},   // DFR
    {{5,0,0},{1,2,2},{2,2,0}},   // DLF
    {{5,2,0},{4,2,2},{1,2,0}},   // DBL
    {{5,2,2},{3,2,2},{4,2,0}},   // DRB
};

// colour-set bitmask (1<<colour for each of the 3 stickers) -> corner piece id
inline constexpr std::array<uint8_t, 64> makeCornerLookup() {
    std::array<uint8_t, 64> t{};
    for (auto& x : t) x = 0xFF;
    for (int k = 0; k < 8; ++k) {
        int mask = 0;
        for (int s = 0; s < 3; ++s) mask |= 1 << CORNERS[k][s].face;  // solved colour == face id
        t[mask] = static_cast<uint8_t>(k);
    }
    return t;
}
inline constexpr std::array<uint8_t, 64> CORNER_BY_MASK = makeCornerLookup();

// Read the corner state (which piece sits in each position, and its twist)
// from any cube model. Templated so that with a `final` model the getColor
// calls are resolved at compile time (no virtual dispatch).
template <typename Cube>
inline void readCorners(const Cube& c, uint8_t perm[8], uint8_t ori[8]) {
    using F = typename Cube::FACE;
    for (int p = 0; p < 8; ++p) {
        int mask = 0;
        uint8_t o = 0;
        for (int s = 0; s < 3; ++s) {
            const Sticker& st = CORNERS[p][s];
            int col = static_cast<int>(c.getColor(static_cast<F>(st.face), st.row, st.col));
            mask |= 1 << col;
            if (col == 0 || col == 5) o = static_cast<uint8_t>(s);   // white or yellow
        }
        perm[p] = CORNER_BY_MASK[mask];
        ori[p]  = o;
    }
}

} // namespace geom

#include "RubiksCube1dArray.h"

#include <cstring>

#include "CubeGeometry.h"

namespace {

// Pre-computed flat indices, so a move is just a handful of byte copies.
struct Tables {
    int ring[6][12];      // flat index of the 12 adjacent stickers
    int faceCycle[6][8];  // the 8 non-centre stickers of each face, clockwise
};

Tables buildTables() {
    Tables t{};
    for (int f = 0; f < 6; ++f) {
        for (int k = 0; k < 12; ++k) {
            const geom::Sticker& s = geom::RING[f][k];
            t.ring[f][k] = RubiksCube1dArray::idx(s.face, s.row, s.col);
        }
        // clockwise order around a face: (0,0)(0,1)(0,2)(1,2)(2,2)(2,1)(2,0)(1,0)
        static const int rc[8][2] = {{0,0},{0,1},{0,2},{1,2},{2,2},{2,1},{2,0},{1,0}};
        for (int i = 0; i < 8; ++i) t.faceCycle[f][i] = RubiksCube1dArray::idx(f, rc[i][0], rc[i][1]);
    }
    return t;
}

const Tables T = buildTables();

} // namespace

RubiksCube1dArray::RubiksCube1dArray() {
    for (int i = 0; i < 54; ++i) cube[i] = static_cast<uint8_t>(i / 9);
}

bool RubiksCube1dArray::isSolved() const {
    for (int i = 0; i < 54; ++i)
        if (cube[i] != i / 9) return false;
    return true;
}

void RubiksCube1dArray::turn(int face, int quarterTurns) {
    // 1. rotate the face: stickers move 2 places along the clockwise cycle per quarter turn
    const int* fc = T.faceCycle[face];
    uint8_t tmpF[8];
    for (int i = 0; i < 8; ++i) tmpF[i] = cube[fc[i]];
    const int fs = 2 * quarterTurns;
    for (int i = 0; i < 8; ++i) cube[fc[(i + fs) % 8]] = tmpF[i];

    // 2. cycle the 12 adjacent stickers
    const int* ring = T.ring[face];
    uint8_t tmp[12];
    for (int k = 0; k < 12; ++k) tmp[k] = cube[ring[k]];
    const int rs = 3 * quarterTurns;
    for (int k = 0; k < 12; ++k) cube[ring[(k + rs) % 12]] = tmp[k];
}

bool RubiksCube1dArray::operator==(const RubiksCube1dArray& other) const {
    return std::memcmp(cube, other.cube, sizeof cube) == 0;
}

RubiksCube1dArray& RubiksCube1dArray::operator=(const RubiksCube1dArray& other) {
    if (this != &other) std::memcpy(cube, other.cube, sizeof cube);
    return *this;
}

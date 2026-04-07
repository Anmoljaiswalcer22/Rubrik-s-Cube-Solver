#include "RubiksCube3dArray.h"

#include <cstring>

#include "CubeGeometry.h"

RubiksCube3dArray::RubiksCube3dArray() {
    for (int f = 0; f < 6; ++f)
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) cube[f][r][c] = static_cast<uint8_t>(f);
}

bool RubiksCube3dArray::isSolved() const {
    for (int f = 0; f < 6; ++f)
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                if (cube[f][r][c] != f) return false;
    return true;
}

// A face turn:
//   1. rotate the face's own 3x3 grid by 90 degrees clockwise
//      (new[r][c] = old[2-c][r]  == transpose + reverse rows)
//   2. shift the 12 adjacent stickers on U/R/D/L (relative) one strip along
void RubiksCube3dArray::turn(int face, int quarterTurns) {
    for (int t = 0; t < quarterTurns; ++t) {
        uint8_t old[3][3];
        std::memcpy(old, cube[face], sizeof old);
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) cube[face][r][c] = old[2 - c][r];
    }

    const geom::Sticker* ring = geom::RING[face];
    uint8_t tmp[12];
    for (int k = 0; k < 12; ++k) tmp[k] = cube[ring[k].face][ring[k].row][ring[k].col];
    const int shift = 3 * quarterTurns;
    for (int k = 0; k < 12; ++k) {
        const geom::Sticker& dst = ring[(k + shift) % 12];
        cube[dst.face][dst.row][dst.col] = tmp[k];
    }
}

bool RubiksCube3dArray::operator==(const RubiksCube3dArray& other) const {
    return std::memcmp(cube, other.cube, sizeof cube) == 0;
}

RubiksCube3dArray& RubiksCube3dArray::operator=(const RubiksCube3dArray& other) {
    if (this != &other) std::memcpy(cube, other.cube, sizeof cube);
    return *this;
}

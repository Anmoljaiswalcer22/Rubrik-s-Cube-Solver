#include "RubiksCubeBitboard.h"

// Moves are defined inline in the header (templated turn<FACE, Q>()), so the
// compiler sees all shifts and masks as constants.

RubiksCubeBitboard::RubiksCubeBitboard() {
    for (int f = 0; f < 6; ++f) bitboard[f] = solvedFace(f);
}

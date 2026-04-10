#pragma once
// ============================================================================
//  Model 3: Bitboard  --  uint64_t bitboard[6]   (48 bytes, fastest)
//
//  Key insight: centres never move, so each face has only 8 moving stickers.
//  Each sticker is 8 bits, one-hot (bit `colour` set)  ->  8 x 8 = 64 bits.
//  Stickers are stored CLOCKWISE around the face, sticker i in bits [8i, 8i+8):
//
//        0 1 2
//        7 . 3        (centre '.' is implied by the face)
//        6 5 4
//
//  A clockwise face rotation moves every sticker 2 places along the cycle,
//  which is a single 16-bit ROTATE of the 64-bit word. Adjacent strips are
//  moved with byte masks.
// ============================================================================
#include <cstdint>

#include "CubeGeometry.h"
#include "RubiksCube.h"

class RubiksCubeBitboard final : public RubiksCube {
public:
    RubiksCubeBitboard();

    COLOR getColor(FACE face, unsigned row, unsigned col) const override final {
        const int f = static_cast<int>(face);
        const int i = POS[row][col];
        if (i == 8) return static_cast<COLOR>(f);  // centre
        const unsigned byte = static_cast<unsigned>((bitboard[f] >> (8 * i)) & 0xFFu);
        return static_cast<COLOR>(__builtin_ctz(byte));
    }

    bool isSolved() const override final {
        for (int f = 0; f < 6; ++f)
            if (bitboard[f] != solvedFace(f)) return false;
        return true;
    }

    RubiksCube& u() override final      { turn<0, 1>(); return *this; }
    RubiksCube& uPrime() override final { turn<0, 3>(); return *this; }
    RubiksCube& u2() override final     { turn<0, 2>(); return *this; }
    RubiksCube& l() override final      { turn<1, 1>(); return *this; }
    RubiksCube& lPrime() override final { turn<1, 3>(); return *this; }
    RubiksCube& l2() override final     { turn<1, 2>(); return *this; }
    RubiksCube& f() override final      { turn<2, 1>(); return *this; }
    RubiksCube& fPrime() override final { turn<2, 3>(); return *this; }
    RubiksCube& f2() override final     { turn<2, 2>(); return *this; }
    RubiksCube& r() override final      { turn<3, 1>(); return *this; }
    RubiksCube& rPrime() override final { turn<3, 3>(); return *this; }
    RubiksCube& r2() override final     { turn<3, 2>(); return *this; }
    RubiksCube& b() override final      { turn<4, 1>(); return *this; }
    RubiksCube& bPrime() override final { turn<4, 3>(); return *this; }
    RubiksCube& b2() override final     { turn<4, 2>(); return *this; }
    RubiksCube& d() override final      { turn<5, 1>(); return *this; }
    RubiksCube& dPrime() override final { turn<5, 3>(); return *this; }
    RubiksCube& d2() override final     { turn<5, 2>(); return *this; }

    bool operator==(const RubiksCubeBitboard& o) const {
        for (int f = 0; f < 6; ++f)
            if (bitboard[f] != o.bitboard[f]) return false;
        return true;
    }
    RubiksCubeBitboard& operator=(const RubiksCubeBitboard& o) {
        for (int f = 0; f < 6; ++f) bitboard[f] = o.bitboard[f];
        return *this;
    }
    RubiksCubeBitboard(const RubiksCubeBitboard& o) = default;

    uint64_t face(int f) const { return bitboard[f]; }

    // (row, col) -> sticker slot (8 = centre)
    static constexpr int POS[3][3] = {{0, 1, 2}, {7, 8, 3}, {6, 5, 4}};

private:
    uint64_t bitboard[6];

    static constexpr uint64_t solvedFace(int f) { return 0x0101010101010101ULL * (1ULL << f); }
    static uint64_t rotl(uint64_t x, unsigned s) { return (x << s) | (x >> (64 - s)); }

    // Bit position of sticker k of the 12-sticker ring around `face`.
    static constexpr int ringShift(int face, int k) {
        return 8 * POS[geom::RING[face][k].row][geom::RING[face][k].col];
    }
    static constexpr int ringFace(int face, int k) { return geom::RING[face][k].face; }

    // A face turn. Templated on the face and number of quarter turns, so every
    // shift and mask below is a compile-time constant and the loops unroll.
    template <int FACE_ID, int Q>
    void turn() {
        // 1. rotate the face itself: one 64-bit rotation by 16 bits per quarter turn
        bitboard[FACE_ID] = rotl(bitboard[FACE_ID], 16u * Q);

        // 2. move the 4 strips of 3 stickers with byte masks
        uint64_t tmp[12];
        for (int k = 0; k < 12; ++k)
            tmp[k] = (bitboard[ringFace(FACE_ID, k)] >> ringShift(FACE_ID, k)) & 0xFFULL;
        for (int k = 0; k < 12; ++k) {
            const int dst = (k + 3 * Q) % 12;
            uint64_t& w = bitboard[ringFace(FACE_ID, dst)];
            w = (w & ~(0xFFULL << ringShift(FACE_ID, dst))) | (tmp[k] << ringShift(FACE_ID, dst));
        }
    }
};

// Hash: mix the 6 words (splitmix64-style multiply / xor-shift)
struct HashBitboard {
    size_t operator()(const RubiksCubeBitboard& c) const {
        uint64_t h = 0x9E3779B97F4A7C15ULL;
        for (int f = 0; f < 6; ++f) {
            uint64_t x = c.face(f) + 0x9E3779B97F4A7C15ULL * (f + 1);
            x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
            x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
            h ^= x ^ (x >> 31);
            h = (h << 7) | (h >> 57);
        }
        return static_cast<size_t>(h);
    }
};

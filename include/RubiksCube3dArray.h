#pragma once
// ============================================================================
//  Model 1: 3D array  --  uint8_t cube[6][3][3]
//  Most intuitive: maps directly onto how we picture the cube. 54 bytes.
// ============================================================================
#include <functional>
#include <string>

#include "RubiksCube.h"

class RubiksCube3dArray final : public RubiksCube {
public:
    RubiksCube3dArray();

    COLOR getColor(FACE face, unsigned row, unsigned col) const override final {
        return static_cast<COLOR>(cube[static_cast<int>(face)][row][col]);
    }
    bool isSolved() const override final;

    RubiksCube& u() override final      { turn(0, 1); return *this; }
    RubiksCube& uPrime() override final { turn(0, 3); return *this; }
    RubiksCube& u2() override final     { turn(0, 2); return *this; }
    RubiksCube& l() override final      { turn(1, 1); return *this; }
    RubiksCube& lPrime() override final { turn(1, 3); return *this; }
    RubiksCube& l2() override final     { turn(1, 2); return *this; }
    RubiksCube& f() override final      { turn(2, 1); return *this; }
    RubiksCube& fPrime() override final { turn(2, 3); return *this; }
    RubiksCube& f2() override final     { turn(2, 2); return *this; }
    RubiksCube& r() override final      { turn(3, 1); return *this; }
    RubiksCube& rPrime() override final { turn(3, 3); return *this; }
    RubiksCube& r2() override final     { turn(3, 2); return *this; }
    RubiksCube& b() override final      { turn(4, 1); return *this; }
    RubiksCube& bPrime() override final { turn(4, 3); return *this; }
    RubiksCube& b2() override final     { turn(4, 2); return *this; }
    RubiksCube& d() override final      { turn(5, 1); return *this; }
    RubiksCube& dPrime() override final { turn(5, 3); return *this; }
    RubiksCube& d2() override final     { turn(5, 2); return *this; }

    bool operator==(const RubiksCube3dArray& other) const;
    RubiksCube3dArray& operator=(const RubiksCube3dArray& other);
    RubiksCube3dArray(const RubiksCube3dArray& other) = default;

    std::string rawString() const { return std::string(reinterpret_cast<const char*>(cube), 54); }

private:
    uint8_t cube[6][3][3];
    void turn(int face, int quarterTurns);  // quarterTurns: 1 = cw, 2 = half, 3 = ccw
};

// Hash: convert the 54 stickers to a string, then std::hash<std::string>
struct Hash3d {
    size_t operator()(const RubiksCube3dArray& c) const {
        return std::hash<std::string>{}(c.rawString());
    }
};

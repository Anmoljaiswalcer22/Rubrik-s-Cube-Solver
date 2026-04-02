#pragma once
// ============================================================================
//  RubiksCube.h  --  Abstract base class for every cube model.
//
//  Faces (net layout used everywhere in the project):
//
//              +-------+
//              |   U   |
//      +-------+-------+-------+-------+
//      |   L   |   F   |   R   |   B   |
//      +-------+-------+-------+-------+
//              |   D   |
//              +-------+
//
//  Every face is a 3x3 grid (row, col) seen from OUTSIDE the cube.
//  Solved colours:  U=White  L=Orange  F=Green  R=Red  B=Blue  D=Yellow
//  (COLOR and FACE share the same numbering, so solved colour == face id.)
// ============================================================================
#include <cstdint>
#include <string>
#include <vector>

class RubiksCube {
public:
    enum class FACE  : uint8_t { UP = 0, LEFT = 1, FRONT = 2, RIGHT = 3, BACK = 4, DOWN = 5 };
    enum class COLOR : uint8_t { WHITE = 0, ORANGE = 1, GREEN = 2, RED = 3, BLUE = 4, YELLOW = 5 };

    // 18 moves: 6 faces x {clockwise, counter-clockwise, 180 degrees}
    enum class MOVE : uint8_t {
        L, LPRIME, L2,
        R, RPRIME, R2,
        U, UPRIME, U2,
        D, DPRIME, D2,
        F, FPRIME, F2,
        B, BPRIME, B2
    };
    static constexpr int NUM_MOVES = 18;

    virtual ~RubiksCube() = default;

    // ------------------------------------------------------------------
    //  Pure virtual interface -- every model implements these its own way
    // ------------------------------------------------------------------
    virtual COLOR getColor(FACE face, unsigned row, unsigned col) const = 0;
    virtual bool  isSolved() const = 0;

    virtual RubiksCube& u() = 0;  virtual RubiksCube& uPrime() = 0;  virtual RubiksCube& u2() = 0;
    virtual RubiksCube& l() = 0;  virtual RubiksCube& lPrime() = 0;  virtual RubiksCube& l2() = 0;
    virtual RubiksCube& f() = 0;  virtual RubiksCube& fPrime() = 0;  virtual RubiksCube& f2() = 0;
    virtual RubiksCube& r() = 0;  virtual RubiksCube& rPrime() = 0;  virtual RubiksCube& r2() = 0;
    virtual RubiksCube& b() = 0;  virtual RubiksCube& bPrime() = 0;  virtual RubiksCube& b2() = 0;
    virtual RubiksCube& d() = 0;  virtual RubiksCube& dPrime() = 0;  virtual RubiksCube& d2() = 0;

    // ------------------------------------------------------------------
    //  Common (non-virtual) behaviour, written once for all models
    // ------------------------------------------------------------------
    RubiksCube& move(MOVE m);              // perform move m
    RubiksCube& invert(MOVE m);            // perform the inverse of m (used for backtracking)
    std::vector<MOVE> randomShuffleCube(unsigned times, unsigned seed);
    void print() const;                    // 2D unfolded layout
    std::string toString() const;          // 54 colour letters (U,L,F,R,B,D order)

    // Static helpers
    static char        getColorLetter(COLOR color);
    static std::string getMove(MOVE m);
    static MOVE        getInverse(MOVE m);
    static int         moveFace(MOVE m) { return static_cast<int>(m) / 3; } // 0=L 1=R 2=U 3=D 4=F 5=B
    static bool        parseMove(const std::string& token, MOVE& out);
    static std::vector<MOVE> parseMoves(const std::string& text);  // "R U R' U'"
    static std::string movesToString(const std::vector<MOVE>& moves);
};

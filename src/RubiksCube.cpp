#include "RubiksCube.h"

#include <iostream>
#include <random>
#include <sstream>

using std::string;
using std::vector;

char RubiksCube::getColorLetter(COLOR color) {
    static const char letters[] = {'W', 'O', 'G', 'R', 'B', 'Y'};
    return letters[static_cast<int>(color)];
}

string RubiksCube::getMove(MOVE m) {
    static const char* names[] = {"L", "L'", "L2", "R", "R'", "R2", "U", "U'", "U2",
                                  "D", "D'", "D2", "F", "F'", "F2", "B", "B'", "B2"};
    return names[static_cast<int>(m)];
}

// F <-> F',  F2 <-> F2
RubiksCube::MOVE RubiksCube::getInverse(MOVE m) {
    int i = static_cast<int>(m);
    int base = i / 3 * 3, kind = i % 3;
    static const int inv[] = {1, 0, 2};
    return static_cast<MOVE>(base + inv[kind]);
}

bool RubiksCube::parseMove(const string& token, MOVE& out) {
    for (int i = 0; i < NUM_MOVES; ++i) {
        string name = getMove(static_cast<MOVE>(i));
        if (token == name || (name.size() == 2 && name[1] == '\'' && token.size() == 2 &&
                              token[0] == name[0] && (token[1] == 'i' || token[1] == '`'))) {
            out = static_cast<MOVE>(i);
            return true;
        }
    }
    return false;
}

vector<RubiksCube::MOVE> RubiksCube::parseMoves(const string& text) {
    vector<MOVE> moves;
    std::istringstream in(text);
    string tok;
    while (in >> tok) {
        MOVE m;
        if (!parseMove(tok, m)) throw std::invalid_argument("Unknown move: " + tok);
        moves.push_back(m);
    }
    return moves;
}

string RubiksCube::movesToString(const vector<MOVE>& moves) {
    string s;
    for (size_t i = 0; i < moves.size(); ++i) {
        if (i) s += ' ';
        s += getMove(moves[i]);
    }
    return s.empty() ? "(none)" : s;
}

RubiksCube& RubiksCube::move(MOVE m) {
    switch (m) {
        case MOVE::L:      return l();
        case MOVE::LPRIME: return lPrime();
        case MOVE::L2:     return l2();
        case MOVE::R:      return r();
        case MOVE::RPRIME: return rPrime();
        case MOVE::R2:     return r2();
        case MOVE::U:      return u();
        case MOVE::UPRIME: return uPrime();
        case MOVE::U2:     return u2();
        case MOVE::D:      return d();
        case MOVE::DPRIME: return dPrime();
        case MOVE::D2:     return d2();
        case MOVE::F:      return f();
        case MOVE::FPRIME: return fPrime();
        case MOVE::F2:     return f2();
        case MOVE::B:      return b();
        case MOVE::BPRIME: return bPrime();
        case MOVE::B2:     return b2();
    }
    return *this;
}

RubiksCube& RubiksCube::invert(MOVE m) { return move(getInverse(m)); }

// Scramble with `times` random moves. Consecutive moves never turn the same
// face (that would just collapse into fewer moves).
vector<RubiksCube::MOVE> RubiksCube::randomShuffleCube(unsigned times, unsigned seed) {
    std::mt19937 rng(seed);
    vector<MOVE> moves;
    int lastFace = -1;
    while (moves.size() < times) {
        MOVE m = static_cast<MOVE>(rng() % NUM_MOVES);
        if (moveFace(m) == lastFace) continue;
        lastFace = moveFace(m);
        move(m);
        moves.push_back(m);
    }
    return moves;
}

string RubiksCube::toString() const {
    string s;
    s.reserve(54);
    for (int f = 0; f < 6; ++f)
        for (unsigned r = 0; r < 3; ++r)
            for (unsigned c = 0; c < 3; ++c)
                s += getColorLetter(getColor(static_cast<FACE>(f), r, c));
    return s;
}

void RubiksCube::print() const {
    auto row = [&](FACE f, unsigned r) {
        string s;
        for (unsigned c = 0; c < 3; ++c) { s += getColorLetter(getColor(f, r, c)); s += ' '; }
        return s;
    };
    const string pad(8, ' ');
    std::cout << "Rubik's Cube:\n\n";
    for (unsigned r = 0; r < 3; ++r) std::cout << pad << row(FACE::UP, r) << '\n';
    std::cout << '\n';
    for (unsigned r = 0; r < 3; ++r)
        std::cout << row(FACE::LEFT, r) << "  " << row(FACE::FRONT, r) << "  "
                  << row(FACE::RIGHT, r) << "  " << row(FACE::BACK, r) << '\n';
    std::cout << '\n';
    for (unsigned r = 0; r < 3; ++r) std::cout << pad << row(FACE::DOWN, r) << '\n';
    std::cout << '\n';
}

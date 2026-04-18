#pragma once
// ============================================================================
//  PermutationIndexer  --  Lehmer code (factorial number system).
//
//  rank():   permutation of 0..N-1  ->  unique number in [0, N!)
//            For each position, count how many SMALLER elements appear later
//            (= smaller elements not used yet); digit i has weight (N-1-i)!.
//  unrank(): the inverse.
// ============================================================================
#include <cstdint>

template <int N>
struct PermutationIndexer {
    static constexpr uint32_t factorial(int n) { return n <= 1 ? 1u : n * factorial(n - 1); }

    static uint32_t rank(const uint8_t perm[N]) {
        uint32_t r = 0;
        for (int i = 0; i < N; ++i) {
            uint32_t smaller = 0;
            for (int j = i + 1; j < N; ++j)
                if (perm[j] < perm[i]) ++smaller;
            r += smaller * factorial(N - 1 - i);
        }
        return r;
    }

    static void unrank(uint32_t r, uint8_t perm[N]) {
        bool used[N] = {};
        for (int i = 0; i < N; ++i) {
            const uint32_t fact = factorial(N - 1 - i);
            uint32_t digit = r / fact;
            r %= fact;
            for (int v = 0; v < N; ++v) {
                if (used[v]) continue;
                if (digit == 0) { perm[i] = static_cast<uint8_t>(v); used[v] = true; break; }
                --digit;
            }
        }
    }
};

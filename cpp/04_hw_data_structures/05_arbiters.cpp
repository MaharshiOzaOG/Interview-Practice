/*
 * 04_hw_data_structures / Q05 : Fixed-priority & round-robin arbiters   Medium
 * ---------------------------------------------------------------------------
 * N requesters (N <= 32), request vector `req` (bit i = requester i).
 * (a) fixed_priority(req)       : lowest index wins. -1 if no request.
 * (b) class RoundRobin          : grant(req) -> index; next search starts after
 *                                 the last winner (fair, starvation-free).
 *     Write it two ways:
 *       - loop version (easy to explain)
 *       - "double mask" version exactly like RTL: mask off requesters at or
 *         below the last grant; if any remain pick the lowest, else fall back to
 *         fixed priority on the full vector.
 * Test: both versions agree on random input; all-requesting gives 0,1,2,3,0...
 */
#include <cstdint>
#include <cstdio>
#include <random>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
int fixed_priority(uint32_t req) {
    if (!req) return -1;
    int i = 0;
    while (!((req >> i) & 1u)) i++;
    return i;
}

class RoundRobinLoop {
    int n_, last_ = -1;
public:
    explicit RoundRobinLoop(int n) : n_(n) {}
    int grant(uint32_t req) {
        for (int k = 1; k <= n_; k++) {
            int i = (last_ + k) % n_;         // start right after the last winner
            if ((req >> i) & 1u) { last_ = i; return i; }
        }
        return -1;
    }
};

class RoundRobinMask {
    int last_ = -1;
public:
    int grant(uint32_t req) {
        // mask keeps only bits strictly above the last grant
        uint32_t mask;
        if (last_ < 0)        mask = 0xFFFFFFFFu;                // nothing granted yet
        else if (last_ >= 31) mask = 0;
        else                  mask = ~((2u << last_) - 1u);       // bits above last_
        int g = fixed_priority(req & mask);
        if (g < 0) g = fixed_priority(req);    // wrap around
        if (g >= 0) last_ = g;
        return g;
    }
};

// ======================= TESTS ==========================
int main() {
    CHECK(fixed_priority(0) == -1);
    CHECK(fixed_priority(0b1100) == 2);

    RoundRobinLoop rr(4);
    int seq[8];
    for (int i = 0; i < 8; i++) seq[i] = rr.grant(0xF);
    int exp[8] = {0, 1, 2, 3, 0, 1, 2, 3};
    for (int i = 0; i < 8; i++) CHECK(seq[i] == exp[i]);

    RoundRobinLoop r2(4);
    CHECK(r2.grant(0b1010) == 1);
    CHECK(r2.grant(0b1010) == 3);
    CHECK(r2.grant(0b1010) == 1);
    CHECK(r2.grant(0) == -1);

    // both implementations must agree
    std::mt19937 rng(7);
    RoundRobinLoop a(8);
    RoundRobinMask b;
    for (int i = 0; i < 20000; i++) {
        uint32_t req = rng() & 0xFF;
        CHECK(a.grant(req) == b.grant(req));
    }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

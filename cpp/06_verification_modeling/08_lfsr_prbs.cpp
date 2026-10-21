/*
 * 06_verification_modeling / Q08 : LFSR / PRBS generator and checker   Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) PRBS7  : x^7 + x^6 + 1   (Fibonacci LFSR) - prove period = 2^7 - 1 = 127
 * (b) PRBS15 : x^15 + x^14 + 1 - prove period = 32767
 * (c) Self-synchronising checker: predict each received bit from the previous
 *     7 RECEIVED bits (no seed needed). Inject k isolated bit errors and show
 *     the checker reports exactly 3k mismatches - each error is seen once
 *     directly and again when it passes each of the 2 taps.
 *
 * HW link: SerDes/PCIe/Ethernet link testing uses PRBS patterns; emulation
 *          bring-up uses LFSRs for cheap pseudo-random stimulus in hardware.
 */
#include <cstdint>
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct Lfsr {
    int n, tap_a, tap_b;          // x^n + x^tap + 1  -> feedback from bits n-1 and tap-1
    uint32_t state;
    int step() {
        int fb = ((state >> (tap_a - 1)) ^ (state >> (tap_b - 1))) & 1;
        state = ((state << 1) | (uint32_t)fb) & ((1u << n) - 1);
        return fb;                // output bit
    }
};

uint32_t period(Lfsr l) {
    uint32_t start = l.state, p = 0;
    do { l.step(); p++; } while (l.state != start);
    return p;
}

int self_sync_check(const std::vector<int>& rx) {   // PRBS7 checker, returns mismatches
    uint32_t hist = 0;                               // last 7 received bits
    int errors = 0;
    for (size_t i = 0; i < rx.size(); i++) {
        if (i >= 7) {
            int pred = ((hist >> 6) ^ (hist >> 5)) & 1;
            if (pred != rx[i]) errors++;
        }
        hist = ((hist << 1) | (uint32_t)rx[i]) & 0x7F;
    }
    return errors;
}

// ======================= TESTS ==========================
int main() {
    CHECK(period(Lfsr{7, 7, 6, 0x01}) == 127);
    CHECK(period(Lfsr{15, 15, 14, 0x0001}) == 32767);

    Lfsr g{7, 7, 6, 0x5A};
    std::vector<int> stream;
    for (int i = 0; i < 2000; i++) stream.push_back(g.step());

    CHECK(self_sync_check(stream) == 0);             // clean link

    std::vector<int> bad = stream;
    int injected = 0;
    for (size_t i = 100; i < bad.size(); i += 97) { bad[i] ^= 1; injected++; }   // spaced > 7
    CHECK(self_sync_check(bad) == 3 * injected);
    std::printf("injected=%d detected mismatches=%d\n", injected, self_sync_check(bad));

    int ones = 0;                                    // PRBS balance: 64 ones / 63 zeros
    Lfsr b{7, 7, 6, 0x01};
    for (int i = 0; i < 127; i++) ones += b.step();
    CHECK(ones == 64);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

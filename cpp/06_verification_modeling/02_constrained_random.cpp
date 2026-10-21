/*
 * 06_verification_modeling / Q02 : Constrained random in plain C++    Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Generate AXI INCR bursts that satisfy (like SV constraints):
 *   c1: addr in [BASE, BASE + RANGE)
 *   c2: addr aligned to size (size = 4 bytes here)
 *   c3: len distribution  dist { [1:4] := 60, [5:8] := 30, [9:16] := 10 }
 *   c4: burst must NOT cross a 4KB boundary
 * Two approaches:
 *   (a) rejection sampling: pick everything, retry if any constraint fails
 *   (b) "solve before": pick len first, then pick addr only from the legal range
 *       -> no retries (this is what `solve len before addr` does in SV)
 * Verify all constraints on 20k samples and that the distribution is ~60/30/10.
 */
#include <cstdint>
#include <cstdio>
#include <random>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

const uint64_t BASE = 0x80000000ull, RANGE = 0x10000, SIZE = 4;
struct Burst { uint64_t addr; int len; };

// ======================= SOLUTION =======================
static int pick_len(std::mt19937& rng) {
    std::discrete_distribution<int> bucket({60, 30, 10});     // weighted choice
    int b = bucket(rng);
    int lo[] = {1, 5, 9}, hi[] = {4, 8, 16};
    return std::uniform_int_distribution<int>(lo[b], hi[b])(rng);
}

static bool legal(const Burst& b) {
    uint64_t end = b.addr + (uint64_t)b.len * SIZE - 1;        // last byte touched
    return b.addr >= BASE && b.addr < BASE + RANGE &&
           b.addr % SIZE == 0 &&
           (b.addr >> 12) == (end >> 12);                     // same 4KB page
}

Burst gen_rejection(std::mt19937& rng, int& tries) {
    std::uniform_int_distribution<uint64_t> ad(BASE, BASE + RANGE - 1);
    for (;;) {
        tries++;
        Burst b{ad(rng) & ~(SIZE - 1), pick_len(rng)};
        if (legal(b)) return b;
    }
}

Burst gen_solve_before(std::mt19937& rng) {
    int len = pick_len(rng);
    uint64_t bytes = (uint64_t)len * SIZE;
    // pick a page, then an offset that leaves room for the whole burst
    uint64_t pages = RANGE / 4096;
    uint64_t page  = std::uniform_int_distribution<uint64_t>(0, pages - 1)(rng);
    uint64_t max_slot = (4096 - bytes) / SIZE;                // last legal aligned slot
    uint64_t slot  = std::uniform_int_distribution<uint64_t>(0, max_slot)(rng);
    return {BASE + page * 4096 + slot * SIZE, len};
}

// ======================= TESTS ==========================
int main() {
    std::mt19937 rng(99);
    const int N = 20000;
    int tries = 0, bucket[3] = {0, 0, 0}, bucket2[3] = {0, 0, 0};
    for (int i = 0; i < N; i++) {
        Burst a = gen_rejection(rng, tries);
        Burst b = gen_solve_before(rng);
        CHECK(legal(a));
        CHECK(legal(b));
        bucket[a.len <= 4 ? 0 : a.len <= 8 ? 1 : 2]++;
        bucket2[b.len <= 4 ? 0 : b.len <= 8 ? 1 : 2]++;
    }
    // solve-before keeps the requested distribution exactly (+- noise)
    CHECK(bucket2[0] > N * 0.57 && bucket2[0] < N * 0.63);
    CHECK(bucket2[1] > N * 0.27 && bucket2[1] < N * 0.33);
    CHECK(bucket2[2] > N * 0.08 && bucket2[2] < N * 0.12);
    std::printf("rejection: %.3f tries/item, len dist %d/%d/%d\n",
                (double)tries / N, bucket[0], bucket[1], bucket[2]);
    std::printf("solve-before: len dist %d/%d/%d\n", bucket2[0], bucket2[1], bucket2[2]);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

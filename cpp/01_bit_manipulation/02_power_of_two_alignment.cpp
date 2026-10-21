/*
 * 01_bit_manipulation / Q02 : Power of two & address alignment  Difficulty: Easy
 * ---------------------------------------------------------------------------
 * Implement:
 *   bool     is_pow2(uint32_t x)
 *   uint32_t next_pow2(uint32_t x)              // smallest 2^k >= x (x=0 -> 1)
 *   int      log2_floor(uint32_t x)             // x > 0
 *   uint64_t align_up(uint64_t a, uint64_t n)   // n is a power of two
 *   uint64_t align_down(uint64_t a, uint64_t n)
 *   bool     is_aligned(uint64_t a, uint64_t n)
 *
 * Concepts : x & (x-1), smearing bits right, ~(n-1) masks
 * HW link  : AXI address alignment, FIFO depth rounding, 4KB page math.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
bool is_pow2(uint32_t x) { return x && !(x & (x - 1)); }

uint32_t next_pow2(uint32_t x) {
    if (x <= 1) return 1;
    x--;                         // so exact powers stay the same
    x |= x >> 1;  x |= x >> 2;  x |= x >> 4;  x |= x >> 8;  x |= x >> 16;  // smear MSB right
    return x + 1;
}

int log2_floor(uint32_t x) {     // index of the highest set bit
    int r = 0;
    while (x >>= 1) r++;
    return r;
}

uint64_t align_down(uint64_t a, uint64_t n) { return a & ~(n - 1); }
uint64_t align_up(uint64_t a, uint64_t n)   { return (a + n - 1) & ~(n - 1); }
bool     is_aligned(uint64_t a, uint64_t n) { return (a & (n - 1)) == 0; }

// ======================= TESTS ==========================
int main() {
    CHECK(!is_pow2(0));   CHECK(is_pow2(1));   CHECK(is_pow2(64));
    CHECK(!is_pow2(96));  CHECK(is_pow2(0x80000000u));

    CHECK(next_pow2(0) == 1);  CHECK(next_pow2(1) == 1);   CHECK(next_pow2(5) == 8);
    CHECK(next_pow2(8) == 8);  CHECK(next_pow2(1000) == 1024);

    CHECK(log2_floor(1) == 0); CHECK(log2_floor(1023) == 9); CHECK(log2_floor(1024) == 10);

    CHECK(align_up(0x1003, 4) == 0x1004);
    CHECK(align_up(0x1000, 4096) == 0x1000);
    CHECK(align_down(0x1FFF, 0x1000) == 0x1000);
    CHECK(is_aligned(0x2000, 0x1000));
    CHECK(!is_aligned(0x2004, 8));

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

/*
 * 01_bit_manipulation / Q01 : Count set bits (popcount)       Difficulty: Easy
 * ---------------------------------------------------------------------------
 * Write popcount(uint32_t x) three ways:
 *   (a) loop over all 32 bits
 *   (b) Brian Kernighan: x &= (x - 1) clears the lowest set bit
 *   (c) SWAR (parallel add of bit groups, no loop)
 *
 * Concepts : shifts, masks, x & (x - 1)
 * Follow-up: Which is fastest when only a few bits are set?  -> (b), it loops
 *            once per SET bit, not once per bit position.
 * HW link  : popcount = "ones counter" in RTL; used for parity, ECC, onehot checks.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
int popcount_loop(uint32_t x) {
    int c = 0;
    for (int i = 0; i < 32; i++) c += (x >> i) & 1u;
    return c;
}

int popcount_kernighan(uint32_t x) {
    int c = 0;
    while (x) { x &= x - 1; c++; }   // each iteration removes the lowest 1
    return c;
}

int popcount_swar(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555u);                  // counts in 2-bit groups
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);  // counts in 4-bit groups
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;                  // counts in 8-bit groups
    return (int)((x * 0x01010101u) >> 24);             // sum the 4 bytes into top byte
}

// ======================= TESTS ==========================
int main() {
    uint32_t in[]  = {0, 1, 0xFFFFFFFFu, 0x80000000u, 0xF0F0F0F0u, 0x12345678u};
    int      exp[] = {0, 1, 32,          1,           16,          13};
    for (int i = 0; i < 6; i++) {
        CHECK(popcount_loop(in[i]) == exp[i]);
        CHECK(popcount_kernighan(in[i]) == exp[i]);
        CHECK(popcount_swar(in[i]) == exp[i]);
    }
    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

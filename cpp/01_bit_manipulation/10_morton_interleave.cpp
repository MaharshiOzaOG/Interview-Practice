/*
 * 01_bit_manipulation / Q10 : Bit interleave (Morton / Z-order)   Difficulty: Hard
 * ---------------------------------------------------------------------------
 * interleave(x, y): 16-bit x and y -> 32-bit z where
 *      z bit 2i   = x bit i
 *      z bit 2i+1 = y bit i
 * deinterleave(z, &x, &y): inverse.
 * Do it with "magic number" spreading (no loop). Verify against a naive loop.
 *
 * Concepts : spreading bits apart with shift + mask stages (log2(16) = 4 stages)
 * HW link  : DRAM/bank address hashing, texture tiling, tiled memory layouts.
 */
#include <cstdint>
#include <cstdio>
#include <random>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
static uint32_t spread(uint32_t x) {          // abcd -> 0a0b0c0d
    x &= 0x0000FFFFu;
    x = (x | (x << 8)) & 0x00FF00FFu;
    x = (x | (x << 4)) & 0x0F0F0F0Fu;
    x = (x | (x << 2)) & 0x33333333u;
    x = (x | (x << 1)) & 0x55555555u;
    return x;
}
static uint32_t compact(uint32_t x) {         // 0a0b0c0d -> abcd
    x &= 0x55555555u;
    x = (x | (x >> 1)) & 0x33333333u;
    x = (x | (x >> 2)) & 0x0F0F0F0Fu;
    x = (x | (x >> 4)) & 0x00FF00FFu;
    x = (x | (x >> 8)) & 0x0000FFFFu;
    return x;
}

uint32_t interleave(uint16_t x, uint16_t y) { return spread(x) | (spread(y) << 1); }
void deinterleave(uint32_t z, uint16_t* x, uint16_t* y) {
    *x = (uint16_t)compact(z);
    *y = (uint16_t)compact(z >> 1);
}

uint32_t interleave_naive(uint16_t x, uint16_t y) {
    uint32_t z = 0;
    for (int i = 0; i < 16; i++) {
        z |= (uint32_t)((x >> i) & 1u) << (2 * i);
        z |= (uint32_t)((y >> i) & 1u) << (2 * i + 1);
    }
    return z;
}

// ======================= TESTS ==========================
int main() {
    CHECK(interleave(0b11, 0b00) == 0b0101);
    CHECK(interleave(0, 0xFFFF) == 0xAAAAAAAAu);
    CHECK(interleave(0xFFFF, 0) == 0x55555555u);

    std::mt19937 rng(1234);
    for (int i = 0; i < 10000; i++) {
        uint16_t x = (uint16_t)rng(), y = (uint16_t)rng(), x2, y2;
        uint32_t z = interleave(x, y);
        CHECK(z == interleave_naive(x, y));
        deinterleave(z, &x2, &y2);
        CHECK(x2 == x && y2 == y);
    }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

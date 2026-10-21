/*
 * 01_bit_manipulation / Q03 : Reverse bits                    Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * (a) reverse32_loop(x)  : reverse all 32 bits with a loop
 * (b) reverse32_swap(x)  : swap halves, then bytes, nibbles, pairs, bits (no loop)
 * (c) reverse_n(x, n)    : reverse only the lower n bits (upper bits become 0)
 *
 * Concepts : divide-and-conquer bit swapping with masks
 * HW link  : bit-order conversion (LSB-first serial links), CRC reflection.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint32_t reverse32_loop(uint32_t x) {
    uint32_t r = 0;
    for (int i = 0; i < 32; i++) {
        r = (r << 1) | (x & 1u);   // take LSB of x, push into r
        x >>= 1;
    }
    return r;
}

uint32_t reverse32_swap(uint32_t x) {
    x = (x >> 16) | (x << 16);                                  // swap 16-bit halves
    x = ((x & 0xFF00FF00u) >> 8) | ((x & 0x00FF00FFu) << 8);    // swap bytes
    x = ((x & 0xF0F0F0F0u) >> 4) | ((x & 0x0F0F0F0Fu) << 4);    // swap nibbles
    x = ((x & 0xCCCCCCCCu) >> 2) | ((x & 0x33333333u) << 2);    // swap pairs
    x = ((x & 0xAAAAAAAAu) >> 1) | ((x & 0x55555555u) << 1);    // swap bits
    return x;
}

uint32_t reverse_n(uint32_t x, int n) {   // 1 <= n <= 32
    return reverse32_swap(x) >> (32 - n);
}

// ======================= TESTS ==========================
int main() {
    uint32_t in[]  = {1, 0x0000000Fu, 0x12345678u, 0xFFFFFFFFu, 0};
    uint32_t exp[] = {0x80000000u, 0xF0000000u, 0x1E6A2C48u, 0xFFFFFFFFu, 0};
    for (int i = 0; i < 5; i++) {
        CHECK(reverse32_loop(in[i]) == exp[i]);
        CHECK(reverse32_swap(in[i]) == exp[i]);
    }
    CHECK(reverse_n(0b1101, 4) == 0b1011);
    CHECK(reverse_n(0b001, 3) == 0b100);
    CHECK(reverse_n(0xAB, 8) == 0xD5);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

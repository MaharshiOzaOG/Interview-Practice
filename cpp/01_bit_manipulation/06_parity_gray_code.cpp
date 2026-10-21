/*
 * 01_bit_manipulation / Q06 : Parity & Gray code                 Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) parity(x)    : 1 if odd number of 1s. Do it in O(log n) by XOR-folding.
 * (b) bin2gray(b)  : g = b ^ (b >> 1)
 * (c) gray2bin(g)  : b = g ^ (g>>1) ^ (g>>2) ^ ...   (prefix XOR from MSB)
 * (d) Show that consecutive Gray codes differ in exactly one bit.
 *
 * HW link  : Gray-coded pointers cross clock domains in an ASYNC FIFO because
 *            only one bit changes per increment -> a synchronizer can never see
 *            a wildly wrong value, only old-or-new.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
int parity(uint32_t x) {
    x ^= x >> 16;  x ^= x >> 8;  x ^= x >> 4;  x ^= x >> 2;  x ^= x >> 1;
    return x & 1;          // bit 0 now holds XOR of all 32 bits
}

uint32_t bin2gray(uint32_t b) { return b ^ (b >> 1); }

uint32_t gray2bin(uint32_t g) {
    for (int s = 1; s < 32; s <<= 1) g ^= g >> s;   // log-step prefix XOR
    return g;
}

static int ones(uint32_t x) { int c = 0; while (x) { x &= x - 1; c++; } return c; }

// ======================= TESTS ==========================
int main() {
    CHECK(parity(0b1011) == 1);
    CHECK(parity(0b11) == 0);
    CHECK(parity(0xFFFFFFFFu) == 0);
    CHECK(parity(0x80000000u) == 1);

    uint32_t g3[] = {0, 1, 3, 2, 6, 7, 5, 4};          // 3-bit Gray sequence
    for (uint32_t i = 0; i < 8; i++) CHECK(bin2gray(i) == g3[i]);

    for (uint32_t i = 0; i < 5000; i++) {
        CHECK(gray2bin(bin2gray(i)) == i);
        CHECK(ones(bin2gray(i) ^ bin2gray(i + 1)) == 1);  // one-bit change
    }
    CHECK(ones(bin2gray(15) ^ bin2gray(0)) == 1);         // 4-bit wrap also 1 bit

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

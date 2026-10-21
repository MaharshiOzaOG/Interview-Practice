/*
 * 01_bit_manipulation / Q08 : Rotate, sign-extend, overflow detect   Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) rotl32 / rotr32 (n may be 0 or >= 32; no undefined behaviour)
 * (b) sign_extend(v, bits) : treat the low `bits` of v as a signed number
 *                            e.g. 12-bit immediate 0xFFF -> -1
 * (c) add_overflows(a, b)  : detect signed 32-bit overflow WITHOUT doing
 *                            signed overflow (which is UB in C/C++)
 *
 * HW link  : RISC-V immediates are sign-extended; ALU V flag = (c).
 */
#include <cstdint>
#include <cstdio>
#include <climits>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint32_t rotl32(uint32_t x, unsigned n) { n &= 31; return (x << n) | (x >> ((32 - n) & 31)); }
uint32_t rotr32(uint32_t x, unsigned n) { n &= 31; return (x >> n) | (x << ((32 - n) & 31)); }

int32_t sign_extend(uint32_t v, int bits) {        // 1 <= bits <= 32
    uint32_t m = 1u << (bits - 1);                 // sign bit position
    if (bits < 32) v &= (1u << bits) - 1u;         // drop junk above the field
    return (int32_t)((v ^ m) - m);                 // flip sign bit, subtract its weight
}

bool add_overflows(int32_t a, int32_t b) {
    uint32_t ua = (uint32_t)a, ub = (uint32_t)b, r = ua + ub;   // unsigned add wraps safely
    // overflow iff a and b have the same sign and r has a different sign
    return ((ua ^ r) & (ub ^ r)) >> 31;
}

// ======================= TESTS ==========================
int main() {
    CHECK(rotl32(0x80000001u, 1) == 0x00000003u);
    CHECK(rotr32(0x00000003u, 1) == 0x80000001u);
    CHECK(rotl32(0x12345678u, 0) == 0x12345678u);
    CHECK(rotl32(0x12345678u, 32) == 0x12345678u);
    CHECK(rotl32(0x12345678u, 8) == 0x34567812u);

    CHECK(sign_extend(0xFFF, 12) == -1);
    CHECK(sign_extend(0x7FF, 12) == 2047);
    CHECK(sign_extend(0x800, 12) == -2048);
    CHECK(sign_extend(0b101, 3) == -3);
    CHECK(sign_extend(0xABC123, 8) == 0x23);       // upper junk ignored
    CHECK(sign_extend(0xFFFFFFFFu, 32) == -1);

    CHECK(add_overflows(INT_MAX, 1));
    CHECK(add_overflows(INT_MIN, -1));
    CHECK(!add_overflows(-1, 1));
    CHECK(!add_overflows(INT_MAX, INT_MIN));

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

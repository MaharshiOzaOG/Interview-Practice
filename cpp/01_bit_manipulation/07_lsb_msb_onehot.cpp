/*
 * 01_bit_manipulation / Q07 : Lowest/highest set bit, one-hot      Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Implement:
 *   isolate_lsb(x)  -> keep only the lowest set bit         (x & -x)
 *   ctz(x)          -> count trailing zeros  (x=0 -> 32)
 *   clz(x)          -> count leading zeros   (x=0 -> 32), binary-search style
 *   msb_index(x)    -> 31 - clz(x)           (x=0 -> -1)
 *   is_onehot(x)    -> exactly one bit set
 *   onehot_to_index / index_to_onehot  (encoder / decoder)
 *
 * HW link  : fixed-priority arbiter = ctz(req); one-hot FSM state checks;
 *            leading-zero counters in FP normalisation.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint32_t isolate_lsb(uint32_t x) { return x & (~x + 1u); }   // same as x & -x for unsigned

int ctz(uint32_t x) {
    if (x == 0) return 32;
    int n = 0;
    while (!(x & 1u)) { x >>= 1; n++; }
    return n;
}

int clz(uint32_t x) {           // binary search: 5 steps for 32 bits
    if (x == 0) return 32;
    int n = 0;
    if (!(x & 0xFFFF0000u)) { n += 16; x <<= 16; }
    if (!(x & 0xFF000000u)) { n += 8;  x <<= 8;  }
    if (!(x & 0xF0000000u)) { n += 4;  x <<= 4;  }
    if (!(x & 0xC0000000u)) { n += 2;  x <<= 2;  }
    if (!(x & 0x80000000u)) { n += 1; }
    return n;
}

int  msb_index(uint32_t x)        { return x ? 31 - clz(x) : -1; }
bool is_onehot(uint32_t x)        { return x && !(x & (x - 1)); }
int  onehot_to_index(uint32_t x)  { return is_onehot(x) ? ctz(x) : -1; }
uint32_t index_to_onehot(int i)   { return 1u << i; }

// ======================= TESTS ==========================
int main() {
    CHECK(isolate_lsb(0b10100) == 0b100);
    CHECK(isolate_lsb(0) == 0);
    CHECK(ctz(0b10100) == 2);  CHECK(ctz(1) == 0);  CHECK(ctz(0) == 32);
    CHECK(clz(1) == 31);       CHECK(clz(0) == 32); CHECK(clz(0x80000000u) == 0);
    CHECK(clz(0x00010000u) == 15);
    CHECK(msb_index(0b10100) == 4);  CHECK(msb_index(0) == -1);
    CHECK(!is_onehot(0));  CHECK(is_onehot(8));  CHECK(!is_onehot(12));
    CHECK(onehot_to_index(0x40) == 6);
    CHECK(onehot_to_index(0x41) == -1);
    CHECK(index_to_onehot(5) == 0x20);

    // brute-force cross check of clz against a simple loop
    for (uint32_t x = 1; x < 70000; x += 7) {
        int ref = 0; for (int b = 31; b >= 0 && !((x >> b) & 1u); b--) ref++;
        CHECK(clz(x) == ref);
    }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

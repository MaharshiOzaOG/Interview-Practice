/*
 * 01_bit_manipulation / Q04 : Set / clear / toggle / extract / insert fields   Easy
 * ---------------------------------------------------------------------------
 * Implement (this is THE most common embedded/emulation C question):
 *   set_bit, clear_bit, toggle_bit, test_bit
 *   field_mask(msb, lsb)            -> mask with bits [msb:lsb] set
 *   get_field(x, msb, lsb)          -> x[msb:lsb]  (like Verilog part-select)
 *   set_field(x, msb, lsb, val)     -> x with [msb:lsb] replaced by val (val truncated)
 *
 * Trap     : (1u << 32) is undefined behaviour -> handle width == 32 separately.
 * HW link  : register field programming = read-modify-write with these masks.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint32_t set_bit(uint32_t x, int n)    { return x |  (1u << n); }
uint32_t clear_bit(uint32_t x, int n)  { return x & ~(1u << n); }
uint32_t toggle_bit(uint32_t x, int n) { return x ^  (1u << n); }
bool     test_bit(uint32_t x, int n)   { return (x >> n) & 1u; }

uint32_t field_mask(int msb, int lsb) {
    int width = msb - lsb + 1;
    uint32_t ones = (width == 32) ? 0xFFFFFFFFu : ((1u << width) - 1u);
    return ones << lsb;
}

uint32_t get_field(uint32_t x, int msb, int lsb) {
    return (x & field_mask(msb, lsb)) >> lsb;
}

uint32_t set_field(uint32_t x, int msb, int lsb, uint32_t val) {
    uint32_t m = field_mask(msb, lsb);
    return (x & ~m) | ((val << lsb) & m);   // clear field, OR in shifted+masked value
}

// ======================= TESTS ==========================
int main() {
    CHECK(set_bit(0, 31) == 0x80000000u);
    CHECK(clear_bit(0xFF, 0) == 0xFE);
    CHECK(toggle_bit(0x10, 4) == 0);
    CHECK(test_bit(0x8, 3) && !test_bit(0x8, 2));

    CHECK(field_mask(7, 4) == 0xF0);
    CHECK(field_mask(31, 0) == 0xFFFFFFFFu);
    CHECK(get_field(0xABCD1234u, 15, 8) == 0x12);
    CHECK(get_field(0xABCD1234u, 31, 0) == 0xABCD1234u);
    CHECK(set_field(0, 7, 4, 0xF) == 0xF0);
    CHECK(set_field(0xFFFFFFFFu, 7, 4, 0) == 0xFFFFFF0Fu);
    CHECK(set_field(0, 3, 0, 0x1F) == 0xF);     // value wider than field is truncated

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

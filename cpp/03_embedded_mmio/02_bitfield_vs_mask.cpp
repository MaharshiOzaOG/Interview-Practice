/*
 * 03_embedded_mmio / Q02 : Register bit-fields vs shift/mask        Difficulty: Medium
 * ---------------------------------------------------------------------------
 * CTRL register layout:  [0] en | [3:1] mode | [7:4] rsvd | [15:8] len | [31:16] rsvd
 *
 * (a) Describe it as a C bit-field struct.
 * (b) Write pack()/unpack() with shifts and masks.
 * (c) Prove both give the same 32-bit value (convert with memcpy, not a union -
 *     union type-punning is legal in C but undefined behaviour in C++).
 *
 * Interview Q: "Why do many drivers avoid bit-fields for registers?"
 *   - bit order and padding are implementation-defined (compiler/ABI)
 *   - access width is not guaranteed (compiler may do byte accesses to a
 *     32-bit-only register)
 *   - each field write is a hidden read-modify-write
 *   -> shift/mask macros are portable and explicit.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct CtrlBits {            // LSB-first on GCC/Clang/MSVC for x86 and ARM
    uint32_t en    : 1;
    uint32_t mode  : 3;
    uint32_t rsvd0 : 4;
    uint32_t len   : 8;
    uint32_t rsvd1 : 16;
};
static_assert(sizeof(CtrlBits) == 4, "CtrlBits must be one 32-bit word");

// portable version
constexpr uint32_t EN_SHIFT = 0,  EN_MASK   = 0x1u  << EN_SHIFT;
constexpr uint32_t MODE_SHIFT = 1, MODE_MASK = 0x7u  << MODE_SHIFT;
constexpr uint32_t LEN_SHIFT = 8,  LEN_MASK  = 0xFFu << LEN_SHIFT;

struct Ctrl { uint32_t en, mode, len; };

uint32_t pack(const Ctrl& c) {
    return ((c.en   << EN_SHIFT)   & EN_MASK) |
           ((c.mode << MODE_SHIFT) & MODE_MASK) |
           ((c.len  << LEN_SHIFT)  & LEN_MASK);
}

Ctrl unpack(uint32_t r) {
    return { (r & EN_MASK) >> EN_SHIFT, (r & MODE_MASK) >> MODE_SHIFT, (r & LEN_MASK) >> LEN_SHIFT };
}

uint32_t bits_to_raw(const CtrlBits& b) { uint32_t r; std::memcpy(&r, &b, 4); return r; }
CtrlBits raw_to_bits(uint32_t r)        { CtrlBits b; std::memcpy(&b, &r, 4); return b; }

// ======================= TESTS ==========================
int main() {
    Ctrl c{1, 5, 0xAB};
    uint32_t raw = pack(c);
    CHECK(raw == 0x0000AB0Bu);                // 1 | (5<<1) | (0xAB<<8)

    Ctrl u = unpack(raw);
    CHECK(u.en == 1 && u.mode == 5 && u.len == 0xAB);

    CtrlBits b{};
    b.en = 1; b.mode = 5; b.len = 0xAB;
    CHECK(bits_to_raw(b) == raw);             // same layout on mainstream compilers

    CtrlBits b2 = raw_to_bits(0x0000FF0Fu);
    CHECK(b2.en == 1 && b2.mode == 7 && b2.len == 0xFF);

    CHECK(pack({1, 9, 0x1FF}) == 0x0000FF03u);  // over-wide values are masked

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

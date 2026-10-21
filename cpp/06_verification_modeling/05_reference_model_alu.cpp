/*
 * 06_verification_modeling / Q05 : ALU reference model with flags  Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * Write  AluOut alu(Op op, uint32_t a, uint32_t b)  for a 32-bit ALU:
 *   ADD SUB AND OR XOR SLL SRL SRA SLT SLTU
 *   Flags: Z (zero), N (bit 31), C (carry out for ADD; NOT-borrow for SUB, ARM style),
 *          V (signed overflow) - C/V only meaningful for ADD/SUB.
 *   Shift amount uses only b[4:0] (like RISC-V).
 * Rules: do all math in UNSIGNED (signed overflow is UB in C++); implement SRA
 *        without relying on >> of a negative int.
 *
 * HW link: this is the "predictor" a scoreboard calls; DPI-C often exposes
 *          exactly such a function to the SV testbench (see 07_dpi_c).
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

enum class Op { ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT, SLTU };
struct AluOut { uint32_t r; bool z, n, c, v; };

// ======================= SOLUTION =======================
AluOut alu(Op op, uint32_t a, uint32_t b) {
    AluOut o{0, false, false, false, false};
    unsigned sh = b & 31u;
    switch (op) {
        case Op::ADD:
            o.r = a + b;
            o.c = o.r < a;                                   // wrapped -> carry
            o.v = ((a ^ o.r) & (b ^ o.r)) >> 31;             // same-sign inputs, sign flipped
            break;
        case Op::SUB:
            o.r = a - b;
            o.c = a >= b;                                    // no borrow
            o.v = ((a ^ b) & (a ^ o.r)) >> 31;               // different signs, result sign != a
            break;
        case Op::AND:  o.r = a & b;  break;
        case Op::OR:   o.r = a | b;  break;
        case Op::XOR:  o.r = a ^ b;  break;
        case Op::SLL:  o.r = a << sh; break;
        case Op::SRL:  o.r = a >> sh; break;
        case Op::SRA:  o.r = (a >> 31) ? ~(~a >> sh) : (a >> sh); break;   // fill with sign
        case Op::SLT:  o.r = (int32_t)a < (int32_t)b; break;
        case Op::SLTU: o.r = a < b; break;
    }
    o.z = (o.r == 0);
    o.n = o.r >> 31;
    return o;
}

// ======================= TESTS ==========================
int main() {
    AluOut o;
    o = alu(Op::ADD, 0xFFFFFFFFu, 1);  CHECK(o.r == 0 && o.z && o.c && !o.v);
    o = alu(Op::ADD, 0x7FFFFFFFu, 1);  CHECK(o.r == 0x80000000u && o.n && o.v && !o.c);
    o = alu(Op::ADD, 2, 3);            CHECK(o.r == 5 && !o.z && !o.c && !o.v);
    o = alu(Op::SUB, 5, 5);            CHECK(o.r == 0 && o.z && o.c && !o.v);
    o = alu(Op::SUB, 3, 5);            CHECK(o.r == 0xFFFFFFFEu && o.n && !o.c && !o.v);
    o = alu(Op::SUB, 0x80000000u, 1);  CHECK(o.r == 0x7FFFFFFFu && o.v && o.c);
    CHECK(alu(Op::SLT,  0xFFFFFFFFu, 1).r == 1);          // -1 < 1 signed
    CHECK(alu(Op::SLTU, 0xFFFFFFFFu, 1).r == 0);          // big unsigned
    CHECK(alu(Op::SRA,  0x80000000u, 4).r == 0xF8000000u);
    CHECK(alu(Op::SRA,  0x40000000u, 4).r == 0x04000000u);
    CHECK(alu(Op::SRL,  0x80000000u, 4).r == 0x08000000u);
    CHECK(alu(Op::SLL,  1, 33).r == 2);                   // only b[4:0] used
    CHECK(alu(Op::XOR,  0xF0F0, 0xFFFF).r == 0x0F0F);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

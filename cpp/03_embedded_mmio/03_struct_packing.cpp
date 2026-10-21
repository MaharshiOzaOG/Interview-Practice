/*
 * 03_embedded_mmio / Q03 : Struct padding, alignment, packing       Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Predict sizeof() BEFORE running (assume a 64-bit platform: x86-64 / AArch64):
 *
 *   struct A { char c; int i; char d; };            -> ?
 *   struct B { int i; char c; char d; };            -> ?
 *   #pragma pack(1) struct P { char c; int i; char d; };  -> ?
 *   struct DmaDescBad  { uint32_t ctrl; uint64_t src; uint64_t dst; uint32_t len; }; -> ?
 *   struct DmaDescGood { uint64_t src; uint64_t dst; uint32_t ctrl; uint32_t len; }; -> ?
 *
 * Rules:
 *  1. Each member starts at an offset that is a multiple of its alignment.
 *  2. sizeof(struct) is rounded up to the largest member alignment
 *     (so arrays of the struct stay aligned).
 *  3. Order members largest -> smallest to minimise padding.
 *
 * HW link: a descriptor struct shared between C firmware and an RTL DMA must
 *          match the hardware layout byte-for-byte -> use fixed-width types,
 *          explicit padding, and static_assert(sizeof/offsetof).
 */
#include <cstddef>
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct A { char c; int i; char d; };        // 1 + 3pad + 4 + 1 + 3pad = 12
struct B { int i; char c; char d; };        // 4 + 1 + 1 + 2pad        = 8

#pragma pack(push, 1)
struct P { char c; int i; char d; };        // no padding              = 6
#pragma pack(pop)

struct DmaDescBad  { uint32_t ctrl; uint64_t src; uint64_t dst; uint32_t len; };
// ctrl@0, pad 4, src@8, dst@16, len@24, pad 4 -> 32

struct DmaDescGood { uint64_t src; uint64_t dst; uint32_t ctrl; uint32_t len; };
// src@0, dst@8, ctrl@16, len@20 -> 24

// Hardware-matched descriptor: layout locked at compile time
struct HwDesc {
    uint64_t src;
    uint64_t dst;
    uint32_t len;
    uint32_t ctrl;
    uint64_t next;          // pointer to next descriptor
};
static_assert(sizeof(HwDesc) == 32,            "HW expects 32-byte descriptors");
static_assert(offsetof(HwDesc, len)  == 16,    "len must be at byte 16");
static_assert(offsetof(HwDesc, next) == 24,    "next must be at byte 24");

// ======================= TESTS ==========================
int main() {
    std::printf("sizeof A=%zu B=%zu P=%zu Bad=%zu Good=%zu\n",
                sizeof(A), sizeof(B), sizeof(P), sizeof(DmaDescBad), sizeof(DmaDescGood));
    CHECK(sizeof(A) == 12);
    CHECK(offsetof(A, i) == 4);
    CHECK(offsetof(A, d) == 8);
    CHECK(sizeof(B) == 8);
    CHECK(sizeof(P) == 6);
    CHECK(offsetof(P, i) == 1);          // unaligned int! slower / may fault on some CPUs
    CHECK(alignof(A) == 4);
    if (sizeof(void*) == 8) {            // 64-bit ABI expectations
        CHECK(sizeof(DmaDescBad) == 32);
        CHECK(sizeof(DmaDescGood) == 24);
    }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

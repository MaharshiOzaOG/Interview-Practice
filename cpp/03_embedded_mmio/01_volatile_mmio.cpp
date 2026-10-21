/*
 * 03_embedded_mmio / Q01 : Memory-mapped register access with volatile   Easy
 * ---------------------------------------------------------------------------
 * A DMA block sits at BASE with these 32-bit registers:
 *     0x00 CTRL    [0] enable  [3:1] mode  [15:8] burst_len
 *     0x04 STATUS  [0] busy    [1] done   [2] error
 *     0x08 SRC     0x0C DST    0x10 LEN
 * Write:
 *     reg_write(off, val), reg_read(off)
 *     reg_rmw(off, mask, val)   // read-modify-write a field
 *     dma_program(src, dst, len, mode, burst)
 *
 * Interview Qs:
 *  - Why `volatile`?  The compiler must not cache, merge or remove accesses:
 *    every read really goes to the bus (status may change), every write really
 *    happens (writes have side effects).
 *  - Is volatile enough for multi-threading? NO - it gives no atomicity or
 *    ordering between CPUs. Use atomics / memory barriers.
 *
 * Here "hardware" is a plain array so the code runs anywhere; on real HW,
 * BASE would be a fixed physical address like 0x4000_0000.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

static uint32_t fake_hw[16];                         // stands in for the device
static const uintptr_t BASE = (uintptr_t)fake_hw;     // real code: 0x40000000

enum : uint32_t { CTRL = 0x00, STATUS = 0x04, SRC = 0x08, DST = 0x0C, LEN = 0x10 };

// ======================= SOLUTION =======================
#define REG32(off) (*(volatile uint32_t*)(BASE + (off)))

inline void     reg_write(uint32_t off, uint32_t v) { REG32(off) = v; }
inline uint32_t reg_read(uint32_t off)              { return REG32(off); }

inline void reg_rmw(uint32_t off, uint32_t mask, uint32_t val) {
    uint32_t r = reg_read(off);        // 1 read
    r = (r & ~mask) | (val & mask);    // modify only the field
    reg_write(off, r);                 // 1 write
}

void dma_program(uint32_t src, uint32_t dst, uint32_t len, uint32_t mode, uint32_t burst) {
    reg_write(SRC, src);
    reg_write(DST, dst);
    reg_write(LEN, len);
    reg_rmw(CTRL, 0x7u << 1,   mode << 1);       // mode  [3:1]
    reg_rmw(CTRL, 0xFFu << 8,  burst << 8);      // burst [15:8]
    reg_rmw(CTRL, 0x1u,        1u);              // enable LAST, after config
}

// ======================= TESTS ==========================
int main() {
    reg_write(CTRL, 0xFFFF0000u);                 // pretend reserved bits are set
    dma_program(0x1000, 0x2000, 256, 5, 16);
    CHECK(reg_read(SRC) == 0x1000);
    CHECK(reg_read(DST) == 0x2000);
    CHECK(reg_read(LEN) == 256);
    CHECK((reg_read(CTRL) & 1u) == 1u);
    CHECK(((reg_read(CTRL) >> 1) & 0x7u) == 5);
    CHECK(((reg_read(CTRL) >> 8) & 0xFFu) == 16);
    CHECK((reg_read(CTRL) & 0xFFFF0000u) == 0xFFFF0000u);   // other bits preserved

    reg_rmw(CTRL, 0x1u, 0);                                  // disable
    CHECK((reg_read(CTRL) & 1u) == 0);
    CHECK(fake_hw[CTRL / 4] == reg_read(CTRL));              // really went to "HW"

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

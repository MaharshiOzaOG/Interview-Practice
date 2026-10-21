/*
 * 03_embedded_mmio / Q04 : Driver: start DMA, poll with timeout, W1C clear   Medium
 * ---------------------------------------------------------------------------
 * Registers (32-bit):
 *   0x00 CTRL   [0] start (self-clearing)
 *   0x04 STATUS [0] busy  [1] done (W1C)  [2] error (W1C)
 *   0x08 SRC  0x0C DST  0x10 LEN
 *
 * Write   int dma_copy(src, dst, len, max_polls)
 *   returns 0 on success, -1 on timeout, -2 if the error bit was set, -3 if busy.
 *   Steps: check not busy -> program -> start -> poll STATUS until done/error
 *          or timeout -> clear done/error by writing 1s (W1C).
 *
 * The "hardware" is a small C++ class: each STATUS read advances it one step,
 * which mimics a device finishing after some time.
 *
 * Interview points: never poll forever (always a timeout), clear sticky status
 * bits, check error before done, program config before the start bit.
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

enum : uint32_t { CTRL = 0x00, STATUS = 0x04, SRC = 0x08, DST = 0x0C, LEN = 0x10 };
enum : uint32_t { ST_BUSY = 1u << 0, ST_DONE = 1u << 1, ST_ERR = 1u << 2 };

// ---------- fake device (stands in for RTL on the emulator) ----------
struct FakeDma {
    uint32_t regs[8] = {0};
    int  cycles_left = 0;
    int  latency     = 5;        // reads of STATUS until done
    bool inject_err  = false;
    bool hang        = false;    // never completes

    uint32_t read(uint32_t off) {
        if (off == STATUS && (regs[STATUS / 4] & ST_BUSY) && !hang) {
            if (--cycles_left <= 0) {
                regs[STATUS / 4] &= ~ST_BUSY;
                regs[STATUS / 4] |= inject_err ? ST_ERR : ST_DONE;
            }
        }
        return regs[off / 4];
    }
    void write(uint32_t off, uint32_t v) {
        if (off == STATUS) { regs[STATUS / 4] &= ~(v & (ST_DONE | ST_ERR)); return; }  // W1C
        if (off == CTRL && (v & 1u)) {                                              // start
            regs[STATUS / 4] |= ST_BUSY;
            cycles_left = latency;
            return;                                     // start bit self-clears
        }
        regs[off / 4] = v;
    }
};
static FakeDma dev;
uint32_t reg_read(uint32_t off)              { return dev.read(off); }
void     reg_write(uint32_t off, uint32_t v) { dev.write(off, v); }

// ======================= SOLUTION =======================
int dma_copy(uint32_t src, uint32_t dst, uint32_t len, int max_polls) {
    if (reg_read(STATUS) & ST_BUSY) return -3;

    reg_write(SRC, src);
    reg_write(DST, dst);
    reg_write(LEN, len);
    reg_write(CTRL, 1u);                          // start last

    for (int i = 0; i < max_polls; i++) {
        uint32_t st = reg_read(STATUS);
        if (st & ST_ERR)  { reg_write(STATUS, ST_ERR);  return -2; }
        if (st & ST_DONE) { reg_write(STATUS, ST_DONE); return 0;  }
    }
    return -1;                                    // timeout: report, don't hang
}

// ======================= TESTS ==========================
int main() {
    dev = FakeDma{};
    CHECK(dma_copy(0x1000, 0x2000, 64, 100) == 0);
    CHECK(dev.regs[SRC / 4] == 0x1000 && dev.regs[LEN / 4] == 64);
    CHECK((dev.regs[STATUS / 4] & ST_DONE) == 0);          // done was cleared (W1C)

    dev = FakeDma{};  dev.latency = 50;
    CHECK(dma_copy(0, 0, 4, 10) == -1);                    // too slow -> timeout

    dev = FakeDma{};  dev.inject_err = true;
    CHECK(dma_copy(0, 0, 4, 100) == -2);
    CHECK((dev.regs[STATUS / 4] & ST_ERR) == 0);

    dev = FakeDma{};  dev.hang = true;
    CHECK(dma_copy(0, 0, 4, 20) == -1);                    // hung device
    CHECK(dma_copy(0, 0, 4, 20) == -3);                    // still busy -> refuse

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

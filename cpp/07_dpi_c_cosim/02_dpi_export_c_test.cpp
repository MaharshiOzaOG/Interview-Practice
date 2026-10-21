/*
 * 07_dpi_c_cosim / Q02 : DPI-C export - a C test drives the SV testbench    Medium
 * ---------------------------------------------------------------------------
 * In emulation/co-sim, tests are often written in C (like firmware) and call
 * SV tasks that perform bus cycles:
 *
 *   // SV side
 *   export "DPI-C" task sv_write32;    task sv_write32(input int unsigned a, d); ... endtask
 *   export "DPI-C" task sv_read32;     task sv_read32(input int unsigned a, output int unsigned d);
 *   import "DPI-C" context task c_test_main();   // SV calls this from run_phase
 *
 * Write c_test_main(): a C test that
 *   1. reads the DMA ID register (0x00) and checks it equals 0xD1A0_0001
 *   2. programs SRC/DST/LEN, sets CTRL.start
 *   3. polls STATUS.done (max 100 reads), then clears it (W1C)
 *   4. returns 0 on pass, non-zero error code otherwise
 *
 * Here the "SV side" exports are plain functions backed by a tiny DMA model so
 * the file runs anywhere. In a real simulator they would consume sim time.
 *
 * Interview Qs: import vs export; why `context` is needed for C->SV calls;
 * exported TASKS may consume time, exported FUNCTIONS may not.
 */
#include <cstdint>
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ---------------- "SV side": exported tasks backed by a DMA model ----------------
static uint32_t regs[8];
static int busy_cycles = 0;
static std::vector<char> bus_log;             // 'W' / 'R' per bus cycle

extern "C" void sv_write32(unsigned int a, unsigned int d) {
    bus_log.push_back('W');
    if (a == 0x14) { regs[5] &= ~d; return; }                    // STATUS W1C
    if (a == 0x10 && (d & 1u)) { busy_cycles = 3; return; }      // CTRL.start
    regs[a / 4] = d;
}
extern "C" void sv_read32(unsigned int a, unsigned int* d) {
    bus_log.push_back('R');
    if (a == 0x14 && busy_cycles > 0 && --busy_cycles == 0) regs[5] |= 1u;   // done
    *d = regs[a / 4];
}

// ======================= SOLUTION (the C test) =======================
enum : unsigned { ID = 0x00, SRC = 0x04, DST = 0x08, LEN = 0x0C, CTRL = 0x10, STATUS = 0x14 };

extern "C" int c_test_main() {
    unsigned int v = 0;
    sv_read32(ID, &v);
    if (v != 0xD1A00001u) { std::printf("bad ID 0x%08X\n", v); return 1; }

    sv_write32(SRC, 0x80000000u);
    sv_write32(DST, 0x80010000u);
    sv_write32(LEN, 4096);
    sv_write32(CTRL, 1);                       // start

    for (int i = 0; i < 100; i++) {
        sv_read32(STATUS, &v);
        if (v & 1u) {
            sv_write32(STATUS, 1u);            // clear done (W1C)
            return 0;
        }
    }
    return 2;                                  // timeout
}

// ======================= TESTS ==========================
int main() {
    regs[0] = 0xD1A00001u;
    CHECK(c_test_main() == 0);
    CHECK(regs[1] == 0x80000000u && regs[3] == 4096);
    CHECK((regs[5] & 1u) == 0);                // done cleared
    CHECK(bus_log.size() == 1 + 4 + 3 + 1);    // ID read, 4 writes, 3 polls, 1 clear

    regs[0] = 0xBAD;                           // wrong silicon ID
    CHECK(c_test_main() == 1);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

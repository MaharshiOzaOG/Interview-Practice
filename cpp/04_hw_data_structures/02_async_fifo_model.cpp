/*
 * 04_hw_data_structures / Q02 : Cycle model of an ASYNC FIFO     Difficulty: Hard
 * ---------------------------------------------------------------------------
 * Model the classic Cummings async FIFO in C++ (depth 8):
 *   - binary pointers with 1 extra bit, converted to Gray code
 *   - Gray write pointer synchronised into the read clock via 2 flops
 *   - Gray read  pointer synchronised into the write clock via 2 flops
 *   - empty (read side):  rgray == synced wgray
 *   - full  (write side): wgray == synced rgray with its TOP TWO bits inverted
 * Then simulate two unrelated clocks (write every 2 time units, read every 3,
 * and the opposite) and check: data in order, no overflow, no underflow.
 *
 * Why it matters: you will be asked "why Gray code?", "why is full pessimistic?"
 * (the synced pointer is stale, so full/empty assert early and de-assert late,
 * which is SAFE - never wrong in the dangerous direction).
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
static uint32_t bin2gray(uint32_t b) { return b ^ (b >> 1); }

struct AsyncFifo {
    static const int A = 3, DEPTH = 1 << A;          // address bits, depth
    static const uint32_t PMASK = (2u * DEPTH) - 1;  // A+1 bit pointers
    uint32_t mem[DEPTH] = {0};

    // write domain
    uint32_t wbin = 0, wgray = 0, wq1_rgray = 0, wq2_rgray = 0;
    // read domain
    uint32_t rbin = 0, rgray = 0, rq1_wgray = 0, rq2_wgray = 0;

    bool full()  const { return wgray == (wq2_rgray ^ (3u << (A - 1))); }
    bool empty() const { return rgray == rq2_wgray; }

    // one rising edge of wclk; returns true if the write happened
    bool wclk_edge(bool winc, uint32_t data) {
        bool do_wr = winc && !full();               // decisions use pre-edge values
        uint32_t sample = rgray;                    // value seen by the 1st sync flop
        if (do_wr) {
            mem[wbin & (DEPTH - 1)] = data;
            wbin  = (wbin + 1) & PMASK;
            wgray = bin2gray(wbin);
        }
        wq2_rgray = wq1_rgray;                      // 2-flop synchroniser shifts
        wq1_rgray = sample;
        return do_wr;
    }

    bool rclk_edge(bool rinc, uint32_t& data) {
        bool do_rd = rinc && !empty();
        uint32_t sample = wgray;
        if (do_rd) {
            data  = mem[rbin & (DEPTH - 1)];
            rbin  = (rbin + 1) & PMASK;
            rgray = bin2gray(rbin);
        }
        rq2_wgray = rq1_wgray;
        rq1_wgray = sample;
        return do_rd;
    }

    uint32_t true_count() const { return (wbin - rbin) & PMASK; }   // god's-eye view
};

// Run a simulation: wper/rper = clock periods in time units
static bool run(int wper, int rper, int items) {
    AsyncFifo f;
    uint32_t next_wr = 0, next_exp = 0;
    bool ok = true;
    for (int t = 0; t < 100000 && next_exp < (uint32_t)items; t++) {
        if (t % wper == 0)
            if (f.wclk_edge(next_wr < (uint32_t)items, next_wr)) next_wr++;
        if (t % rper == 0) {
            uint32_t d;
            if (f.rclk_edge(true, d)) { if (d != next_exp) ok = false; next_exp++; }
        }
        if (f.true_count() > (uint32_t)AsyncFifo::DEPTH) ok = false;   // overflow?
    }
    return ok && next_exp == (uint32_t)items;
}

// ======================= TESTS ==========================
int main() {
    CHECK(run(2, 3, 500));        // fast writer, slow reader -> FIFO fills, full works
    CHECK(run(3, 2, 500));        // slow writer, fast reader -> empty works
    CHECK(run(1, 7, 300));
    CHECK(run(5, 1, 300));
    CHECK(run(1, 1, 300));

    // full flag check: write with reader stopped
    AsyncFifo f;
    int written = 0;
    for (int i = 0; i < 20; i++) written += f.wclk_edge(true, i);
    CHECK(written == AsyncFifo::DEPTH && f.full());

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

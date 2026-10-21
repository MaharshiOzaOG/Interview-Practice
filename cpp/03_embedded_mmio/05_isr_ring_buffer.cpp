/*
 * 03_embedded_mmio / Q05 : Lock-free SPSC ring buffer (ISR -> main loop)   Medium
 * ---------------------------------------------------------------------------
 * An interrupt handler receives bytes from a UART and must hand them to the
 * main loop. Write a single-producer / single-consumer ring buffer:
 *     bool push(uint8_t b)   // called ONLY from the ISR  (producer)
 *     bool pop(uint8_t& b)   // called ONLY from main     (consumer)
 * No locks (you can't block in an ISR). Count dropped bytes when full.
 *
 * Design:
 *   - head written only by producer, tail written only by consumer
 *   - free-running unsigned counters; index = counter & (N-1); size = head - tail
 *   - std::atomic with release (publish data) / acquire (see data) ordering.
 *     On a bare-metal single core, `volatile` + compiler barrier is the C way.
 *
 * The test simulates "interrupts" arriving between main-loop steps so it runs
 * on any online compiler (no threads needed).
 */
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
template <unsigned N>
class SpscRing {
    static_assert(N && !(N & (N - 1)), "N must be a power of two");
    uint8_t buf_[N];
    std::atomic<uint32_t> head_{0};    // next write (producer owns)
    std::atomic<uint32_t> tail_{0};    // next read  (consumer owns)
public:
    uint32_t dropped = 0;

    bool push(uint8_t b) {                                    // ISR side
        uint32_t h = head_.load(std::memory_order_relaxed);
        uint32_t t = tail_.load(std::memory_order_acquire);
        if (h - t == N) { dropped++; return false; }          // full
        buf_[h & (N - 1)] = b;
        head_.store(h + 1, std::memory_order_release);        // publish AFTER data write
        return true;
    }

    bool pop(uint8_t& b) {                                    // main-loop side
        uint32_t t = tail_.load(std::memory_order_relaxed);
        uint32_t h = head_.load(std::memory_order_acquire);
        if (h == t) return false;                             // empty
        b = buf_[t & (N - 1)];
        tail_.store(t + 1, std::memory_order_release);        // free the slot
        return true;
    }

    uint32_t size() const { return head_.load() - tail_.load(); }
};

// ======================= TESTS ==========================
int main() {
    SpscRing<8> r;
    uint8_t b;
    CHECK(!r.pop(b));                               // empty

    for (int i = 0; i < 8; i++) CHECK(r.push((uint8_t)i));
    CHECK(!r.push(99));                             // full -> dropped
    CHECK(r.dropped == 1 && r.size() == 8);
    for (int i = 0; i < 8; i++) { CHECK(r.pop(b)); CHECK(b == i); }

    // Simulated traffic: bursts of "interrupts" between slower main-loop polls
    SpscRing<16> u;
    std::vector<uint8_t> got;
    uint8_t next = 0;
    for (int step = 0; step < 2000; step++) {
        int burst = (step % 7) + 1;                 // ISR fires 1..7 times
        for (int k = 0; k < burst; k++) if (u.push(next)) next++; else break;
        for (int k = 0; k < 4; k++) if (u.pop(b)) got.push_back(b);   // main drains 4
    }
    while (u.pop(b)) got.push_back(b);
    bool in_order = true;
    for (size_t i = 0; i < got.size(); i++) if (got[i] != (uint8_t)i) in_order = false;
    CHECK(in_order);                                // nothing lost, reordered or duplicated
    CHECK(got.size() % 256 == next);                // every pushed byte was popped
    std::printf("transferred=%zu dropped=%u\n", got.size(), u.dropped);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

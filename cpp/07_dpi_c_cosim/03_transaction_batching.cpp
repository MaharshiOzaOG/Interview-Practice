/*
 * 07_dpi_c_cosim / Q03 : Why emulation is slow - batching host<->emulator calls   Hard
 * ---------------------------------------------------------------------------
 * In transaction-based acceleration (TBA) every call between the host
 * (C/UVM side) and the emulator (synthesizable transactor) costs a large FIXED
 * overhead (the emulator stops, syncs, transfers). Model it:
 *     cost(call) = OVERHEAD + words * PER_WORD
 *
 * (a) DirectChannel : one call per transaction
 * (b) Pipe          : SCE-MI-pipe-like buffer; send(word) only buffers, the call
 *                     happens when the buffer is full or on flush() (end of message)
 * Send 10,000 transactions of 4 words through both. Show:
 *   - the emulator side receives identical data (checksum)
 *   - call counts: 10,000 vs ceil(40,000 / 256) = 157
 *   - modelled speed-up (> 50x)
 *
 * Interview Q: "Your emulation runs at 20 kHz instead of 1 MHz. Why?"
 *   -> too many host<->emulator synchronisations (chatty/timed testbench,
 *      blocking per-cycle calls, $display in the DUT). Fix: untimed testbench,
 *      batching/streaming pipes, move logic into the transactor.
 */
#include <cstdint>
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

const uint64_t OVERHEAD = 1000, PER_WORD = 1;     // arbitrary time units

// emulator-side transactor: just accumulates what it receives
struct EmuSide {
    uint64_t checksum = 0, words = 0;
    void receive(const uint32_t* w, size_t n) {
        for (size_t i = 0; i < n; i++) { checksum = checksum * 31 + w[i]; words++; }
    }
};

// ======================= SOLUTION =======================
class DirectChannel {
    EmuSide& emu_;
public:
    uint64_t calls = 0, time = 0;
    explicit DirectChannel(EmuSide& e) : emu_(e) {}
    void send_txn(const uint32_t* w, size_t n) {          // every txn crosses the boundary
        calls++;
        time += OVERHEAD + n * PER_WORD;
        emu_.receive(w, n);
    }
};

class Pipe {
    EmuSide& emu_;
    std::vector<uint32_t> buf_;
    size_t depth_;
public:
    uint64_t calls = 0, time = 0;
    Pipe(EmuSide& e, size_t depth) : emu_(e), depth_(depth) { buf_.reserve(depth); }
    void send(uint32_t w) {
        buf_.push_back(w);
        if (buf_.size() == depth_) flush();                // crossing only when full
    }
    void flush() {                                         // end-of-message / end-of-test
        if (buf_.empty()) return;
        calls++;
        time += OVERHEAD + buf_.size() * PER_WORD;
        emu_.receive(buf_.data(), buf_.size());
        buf_.clear();
    }
};

// ======================= TESTS ==========================
int main() {
    const int TXNS = 10000, WORDS = 4;
    EmuSide e1, e2;
    DirectChannel direct(e1);
    Pipe pipe(e2, 256);

    for (int t = 0; t < TXNS; t++) {
        uint32_t w[WORDS] = {(uint32_t)t, 0xA0000000u + (uint32_t)t * 4, (uint32_t)t ^ 0x5A5A, 4};
        direct.send_txn(w, WORDS);
        for (int i = 0; i < WORDS; i++) pipe.send(w[i]);
    }
    pipe.flush();                                          // don't forget the tail!

    CHECK(e1.words == 40000 && e2.words == 40000);
    CHECK(e1.checksum == e2.checksum);                     // identical data, same order
    CHECK(direct.calls == 10000);
    CHECK(pipe.calls == 157);                              // ceil(40000/256)
    double speedup = (double)direct.time / (double)pipe.time;
    CHECK(speedup > 50.0);
    std::printf("direct: %llu calls, %llu units | pipe: %llu calls, %llu units | speed-up %.1fx\n",
                (unsigned long long)direct.calls, (unsigned long long)direct.time,
                (unsigned long long)pipe.calls, (unsigned long long)pipe.time, speedup);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

/*
 * 06_verification_modeling / Q11 : AXI burst address & strobe calculator   Medium
 * ---------------------------------------------------------------------------
 * (a) beat_addrs(start, size, len, burst) -> address of every beat
 *       FIXED : same address every beat
 *       INCR  : beat 0 = start, later beats = aligned(start) + i*size
 *       WRAP  : total = size*len; lower = (start / total) * total;
 *               address wraps back to `lower` at lower + total
 * (b) check_legal(...) -> "" or an error:
 *       WRAP: len in {2,4,8,16} and start aligned to size
 *       INCR: must not cross a 4KB boundary
 * (c) wstrb(addr, size, bus_bytes, first_beat) -> byte-lane strobe for one beat
 *       (narrow transfers and unaligned first beat)
 *
 * HW link: your AXI DMA driver/monitor and the slave memory model must compute
 *          exactly these addresses; mistakes here are classic DV bugs.
 */
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

enum class Burst { FIXED, INCR, WRAP };
using V = std::vector<uint64_t>;

// ======================= SOLUTION =======================
V beat_addrs(uint64_t start, uint32_t size, uint32_t len, Burst b) {
    V out;
    uint64_t aligned = start & ~(uint64_t)(size - 1);
    uint64_t total   = (uint64_t)size * len;
    uint64_t lower   = (start / total) * total;
    uint64_t addr    = start;
    for (uint32_t i = 0; i < len; i++) {
        out.push_back(addr);
        switch (b) {
            case Burst::FIXED: break;
            case Burst::INCR:  addr = aligned + (uint64_t)(i + 1) * size; break;
            case Burst::WRAP:
                addr = aligned + size;  aligned = addr;
                if (addr >= lower + total) { addr = lower; aligned = lower; }
                break;
        }
    }
    return out;
}

std::string check_legal(uint64_t start, uint32_t size, uint32_t len, Burst b) {
    if (b == Burst::WRAP) {
        if (!(len == 2 || len == 4 || len == 8 || len == 16)) return "WRAP len must be 2/4/8/16";
        if (start % size) return "WRAP start must be aligned";
    }
    if (b == Burst::INCR) {
        uint64_t last = (start & ~(uint64_t)(size - 1)) + (uint64_t)size * len - 1;
        if ((start >> 12) != (last >> 12)) return "INCR crosses 4KB";
    }
    return "";
}

uint32_t wstrb(uint64_t addr, uint32_t size, uint32_t bus_bytes) {
    uint32_t lo = (uint32_t)(addr & (bus_bytes - 1));                         // first lane
    uint64_t aligned = addr & ~(uint64_t)(size - 1);
    uint32_t hi = (uint32_t)((aligned + size - 1) & (bus_bytes - 1));         // last lane
    uint32_t s = 0;
    for (uint32_t i = lo; i <= hi; i++) s |= 1u << i;
    return s;
}

// ======================= TESTS ==========================
int main() {
    CHECK(beat_addrs(0x100, 4, 4, Burst::INCR)  == V({0x100, 0x104, 0x108, 0x10C}));
    CHECK(beat_addrs(0x100, 4, 3, Burst::FIXED) == V({0x100, 0x100, 0x100}));
    CHECK(beat_addrs(0x38, 4, 4, Burst::WRAP)   == V({0x38, 0x3C, 0x30, 0x34}));
    CHECK(beat_addrs(0x30, 4, 4, Burst::WRAP)   == V({0x30, 0x34, 0x38, 0x3C}));   // no wrap needed
    CHECK(beat_addrs(0x1002, 4, 3, Burst::INCR) == V({0x1002, 0x1004, 0x1008}));  // unaligned start

    CHECK(check_legal(0x38, 4, 4, Burst::WRAP) == "");
    CHECK(check_legal(0x38, 4, 3, Burst::WRAP) == "WRAP len must be 2/4/8/16");
    CHECK(check_legal(0x3A, 4, 4, Burst::WRAP) == "WRAP start must be aligned");
    CHECK(check_legal(0xFF8, 8, 2, Burst::INCR) == "INCR crosses 4KB");
    CHECK(check_legal(0xFF8, 8, 1, Burst::INCR) == "");

    CHECK(wstrb(0x0, 4, 4) == 0b1111);
    CHECK(wstrb(0x1, 4, 4) == 0b1110);          // unaligned first beat
    CHECK(wstrb(0x2, 1, 4) == 0b0100);          // narrow byte
    CHECK(wstrb(0x2, 2, 4) == 0b1100);          // narrow halfword, upper lanes
    CHECK(wstrb(0x14, 4, 8) == 0xF0);           // 32-bit beat on a 64-bit bus

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

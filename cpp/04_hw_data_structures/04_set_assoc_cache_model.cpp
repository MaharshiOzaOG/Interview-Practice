/*
 * 04_hw_data_structures / Q04 : Set-associative cache model      Difficulty: Hard
 * ---------------------------------------------------------------------------
 * Write a write-back / write-allocate cache model:
 *     Cache(size_bytes, line_bytes, ways)
 *     bool access(uint32_t addr, bool is_write)   // returns hit
 *     stats: hits, misses, writebacks
 * Address split:  | tag | index | offset |
 *     offset bits = log2(line), index bits = log2(sets), sets = size/(line*ways)
 * Replacement: true LRU per set (use a timestamp per line).
 * Dirty victim -> writeback++.
 *
 * HW link: this is the reference model for your WB/WA cache controller's
 *          scoreboard - predicts hit/miss/eviction for every request.
 */
#include <cstdint>
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class Cache {
    struct Line { bool valid = false, dirty = false; uint32_t tag = 0; uint64_t last_use = 0; };
    int sets_, ways_, off_bits_, idx_bits_;
    std::vector<std::vector<Line>> lines_;
    uint64_t now_ = 0;

    static int log2i(uint32_t x) { int r = 0; while (x >>= 1) r++; return r; }
public:
    uint64_t hits = 0, misses = 0, writebacks = 0;

    Cache(uint32_t size, uint32_t line, uint32_t ways)
        : sets_(size / (line * ways)), ways_(ways),
          off_bits_(log2i(line)), idx_bits_(log2i(size / (line * ways))),
          lines_(sets_, std::vector<Line>(ways)) {}

    uint32_t index(uint32_t a) const { return (a >> off_bits_) & ((1u << idx_bits_) - 1); }
    uint32_t tag(uint32_t a)   const { return a >> (off_bits_ + idx_bits_); }

    bool access(uint32_t addr, bool is_write) {
        now_++;
        auto& set = lines_[index(addr)];
        uint32_t t = tag(addr);

        for (auto& l : set)                                  // 1. lookup
            if (l.valid && l.tag == t) {
                hits++;
                l.last_use = now_;
                if (is_write) l.dirty = true;
                return true;
            }

        misses++;                                            // 2. miss: pick victim
        Line* victim = &set[0];
        for (auto& l : set) {
            if (!l.valid) { victim = &l; break; }            // prefer an empty way
            if (l.last_use < victim->last_use) victim = &l;  // else least recent
        }
        if (victim->valid && victim->dirty) writebacks++;    // 3. write back dirty line

        victim->valid = true;                                // 4. allocate (fill)
        victim->dirty = is_write;                            //    write-allocate
        victim->tag = t;
        victim->last_use = now_;
        return false;
    }
};

// ======================= TESTS ==========================
int main() {
    Cache c(128, 16, 2);          // 4 sets x 2 ways x 16B lines
    CHECK(c.index(0x040) == 0 && c.index(0x050) == 1 && c.tag(0x040) == 1);

    CHECK(!c.access(0x000, true));    // miss, line becomes dirty
    CHECK( c.access(0x004, false));   // same line -> hit
    CHECK(!c.access(0x040, false));   // set 0, second way
    CHECK( c.access(0x000, false));   // hit; now 0x040 is LRU
    CHECK(!c.access(0x080, false));   // evicts 0x040 (clean)
    CHECK(c.writebacks == 0);
    CHECK(!c.access(0x0C0, false));   // evicts 0x000 (dirty) -> writeback
    CHECK(c.writebacks == 1);
    CHECK(!c.access(0x000, false));   // it was evicted
    CHECK(c.hits == 2 && c.misses == 5);

    // streaming through 2x the cache size -> every access misses after warm-up
    Cache s(1024, 32, 4);
    for (int pass = 0; pass < 2; pass++)
        for (uint32_t a = 0; a < 2048; a += 32) s.access(a, false);
    CHECK(s.hits == 0);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

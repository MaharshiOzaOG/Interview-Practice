/*
 * 06_verification_modeling / Q01 : Transaction class (sequence-item style)   Easy
 * ---------------------------------------------------------------------------
 * struct AxiTxn { id, addr, len (beats 1..16), size (bytes/beat: 1,2,4,8),
 *                 burst (FIXED/INCR/WRAP), is_write, data (one word per beat) }
 * Implement:
 *   bool        compare(const AxiTxn& o, std::string& why) const  // first mismatch
 *   std::string to_string() const                                 // for logs
 *   void        randomize(std::mt19937& rng)                      // LEGAL values only:
 *                   addr aligned to size, data.size() == len,
 *                   WRAP -> len in {2,4,8,16}
 *   operator==
 * HW link: uvm_sequence_item's do_compare / convert2string / randomize.
 */
#include <cstdint>
#include <cstdio>
#include <random>
#include <sstream>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

enum class Burst { FIXED, INCR, WRAP };

// ======================= SOLUTION =======================
struct AxiTxn {
    int id = 0;
    uint64_t addr = 0;
    int len = 1, size = 4;
    Burst burst = Burst::INCR;
    bool is_write = true;
    std::vector<uint64_t> data;

    bool compare(const AxiTxn& o, std::string& why) const {
        std::ostringstream s;
        if (id != o.id)             s << "id "   << id   << "!=" << o.id;
        else if (addr != o.addr)    s << "addr 0x" << std::hex << addr << "!=0x" << o.addr;
        else if (len != o.len)      s << "len "  << len  << "!=" << o.len;
        else if (size != o.size)    s << "size " << size << "!=" << o.size;
        else if (burst != o.burst)  s << "burst differs";
        else if (is_write != o.is_write) s << "direction differs";
        else if (data.size() != o.data.size()) s << "beat count differs";
        else
            for (size_t i = 0; i < data.size(); i++)
                if (data[i] != o.data[i]) {
                    s << "beat " << i << " 0x" << std::hex << data[i] << "!=0x" << o.data[i];
                    break;
                }
        why = s.str();
        return why.empty();
    }
    bool operator==(const AxiTxn& o) const { std::string w; return compare(o, w); }

    std::string to_string() const {
        static const char* bn[] = {"FIXED", "INCR", "WRAP"};
        std::ostringstream s;
        s << (is_write ? "WR" : "RD") << " id=" << id << " addr=0x" << std::hex << addr
          << std::dec << " len=" << len << " size=" << size << " " << bn[(int)burst];
        return s.str();
    }

    void randomize(std::mt19937& rng) {
        auto pick = [&](int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng); };
        id       = pick(0, 15);
        is_write = pick(0, 1);
        burst    = (Burst)pick(0, 2);
        size     = 1 << pick(0, 3);
        if (burst == Burst::WRAP) len = 1 << pick(1, 4);   // 2,4,8,16
        else                      len = pick(1, 16);
        addr = (uint64_t)pick(0, 0xFFFFF) & ~(uint64_t)(size - 1);   // aligned
        data.assign(len, 0);
        for (auto& d : data) d = ((uint64_t)rng() << 32 | rng()) & (size == 8 ? ~0ull : ((1ull << (8 * size)) - 1));
    }
};

// ======================= TESTS ==========================
int main() {
    std::mt19937 rng(2026);
    for (int i = 0; i < 2000; i++) {
        AxiTxn t;
        t.randomize(rng);
        CHECK(t.addr % t.size == 0);
        CHECK((int)t.data.size() == t.len && t.len >= 1 && t.len <= 16);
        if (t.burst == Burst::WRAP) CHECK(t.len == 2 || t.len == 4 || t.len == 8 || t.len == 16);
        AxiTxn copy = t;
        CHECK(copy == t);
    }

    AxiTxn a;  a.addr = 0x100; a.len = 2; a.data = {1, 2};
    AxiTxn b = a;
    b.data[1] = 3;
    std::string why;
    CHECK(!a.compare(b, why));
    CHECK(why == "beat 1 0x2!=0x3");
    b = a; b.addr = 0x104;
    CHECK(!a.compare(b, why) && why == "addr 0x100!=0x104");
    CHECK(a.to_string() == "WR id=0 addr=0x100 len=2 size=4 INCR");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

/*
 * 06_verification_modeling / Q06 : Functional coverage model       Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Build a tiny coverage library:
 *   Coverpoint: named bins, each a [lo, hi] range; sample(v) -> bin index or -1
 *               (values in no bin are ignored); coverage() = % bins hit >= 1
 *   Cross     : of two coverpoints; tracks (binA, binB) pairs; coverage()
 * Then cover an AXI stream:
 *   cp_len   : {1}, {2:4}, {5:8}, {9:16}
 *   cp_burst : FIXED(0), INCR(1), WRAP(2)
 *   cross len x burst  (12 combinations)
 *
 * HW link: covergroup/coverpoint/cross in SV. In emulation, SV coverage is
 *          expensive, so teams often collect counters like this in C++.
 */
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <set>
#include <string>
#include <utility>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct Bin { std::string name; int64_t lo, hi; uint64_t hits = 0; };

class Coverpoint {
public:
    std::string name;
    std::vector<Bin> bins;
    Coverpoint(std::string n, std::vector<Bin> b) : name(std::move(n)), bins(std::move(b)) {}

    int sample(int64_t v) {
        for (size_t i = 0; i < bins.size(); i++)
            if (v >= bins[i].lo && v <= bins[i].hi) { bins[i].hits++; return (int)i; }
        return -1;
    }
    double coverage() const {
        int hit = 0;
        for (auto& b : bins) if (b.hits) hit++;
        return bins.empty() ? 0.0 : 100.0 * hit / bins.size();
    }
};

class Cross {
    const Coverpoint& a_; const Coverpoint& b_;
    std::set<std::pair<int, int>> hit_;
public:
    Cross(const Coverpoint& a, const Coverpoint& b) : a_(a), b_(b) {}
    void sample(int ia, int ib) { if (ia >= 0 && ib >= 0) hit_.insert({ia, ib}); }
    double coverage() const { return 100.0 * hit_.size() / (a_.bins.size() * b_.bins.size()); }
    std::vector<std::pair<int, int>> holes() const {        // what's still missing
        std::vector<std::pair<int, int>> h;
        for (int i = 0; i < (int)a_.bins.size(); i++)
            for (int j = 0; j < (int)b_.bins.size(); j++)
                if (!hit_.count({i, j})) h.push_back({i, j});
        return h;
    }
};

struct AxiCov {
    Coverpoint len{"cp_len", {{"one", 1, 1}, {"short", 2, 4}, {"mid", 5, 8}, {"long", 9, 16}}};
    Coverpoint burst{"cp_burst", {{"FIXED", 0, 0}, {"INCR", 1, 1}, {"WRAP", 2, 2}}};
    Cross x{len, burst};
    void sample(int l, int b) { x.sample(len.sample(l), burst.sample(b)); }
};

// ======================= TESTS ==========================
int main() {
    AxiCov cov;
    cov.sample(1, 1);
    cov.sample(3, 1);
    CHECK(cov.len.coverage() == 50.0);
    CHECK(cov.burst.coverage() > 33.0 && cov.burst.coverage() < 34.0);
    CHECK(cov.x.holes().size() == 10);

    cov.sample(99, 1);                          // len 99 is in no bin: ignored
    CHECK(cov.len.coverage() == 50.0);

    for (int l : {1, 3, 6, 12})
        for (int b : {0, 1, 2}) cov.sample(l, b);
    CHECK(cov.len.coverage() == 100.0);
    CHECK(cov.burst.coverage() == 100.0);
    CHECK(cov.x.coverage() == 100.0 && cov.x.holes().empty());
    CHECK(cov.len.bins[1].hits == 4);           // len=3 sampled 4 times

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

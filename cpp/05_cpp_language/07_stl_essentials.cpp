/*
 * 05_cpp_language / Q07 : STL essentials through DV tasks       Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * (a) sort transactions by (priority DESC, id ASC) with a lambda comparator
 * (b) count accesses per address with unordered_map; return top-K hottest
 *     addresses using a priority_queue (min-heap of size K)
 * (c) remove duplicate addresses from a vector (sort + unique + erase)
 * (d) ADDRESS DECODER: std::map<base, Region>; find which region owns an
 *     address with upper_bound (O(log n)) - the core of any SoC memory map
 * (e) count_if / accumulate / find_if one-liners
 */
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <numeric>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

struct Txn { int id; int prio; uint32_t addr; };

// ======================= SOLUTION =======================
void sort_txns(std::vector<Txn>& v) {
    std::sort(v.begin(), v.end(), [](const Txn& a, const Txn& b) {
        if (a.prio != b.prio) return a.prio > b.prio;   // higher priority first
        return a.id < b.id;                             // tie-break: lower id first
    });
}

std::vector<uint32_t> top_k_addrs(const std::vector<uint32_t>& accesses, int k) {
    std::unordered_map<uint32_t, int> freq;
    for (auto a : accesses) freq[a]++;
    using P = std::pair<int, uint32_t>;                                   // {count, addr}
    std::priority_queue<P, std::vector<P>, std::greater<P>> heap;          // min-heap
    for (auto& kv : freq) {
        heap.push({kv.second, kv.first});
        if ((int)heap.size() > k) heap.pop();                              // drop smallest
    }
    std::vector<uint32_t> out;
    while (!heap.empty()) { out.push_back(heap.top().second); heap.pop(); }
    std::reverse(out.begin(), out.end());                                  // hottest first
    return out;
}

void dedupe(std::vector<uint32_t>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

struct Region { uint32_t size; std::string name; };

std::string decode(const std::map<uint32_t, Region>& mmap, uint32_t addr) {
    auto it = mmap.upper_bound(addr);         // first region with base > addr
    if (it == mmap.begin()) return "DECERR";
    --it;                                     // candidate: last base <= addr
    if (addr - it->first < it->second.size) return it->second.name;
    return "DECERR";                          // in a hole between regions
}

// ======================= TESTS ==========================
int main() {
    std::vector<Txn> t = {{1, 0, 0}, {2, 3, 0}, {3, 1, 0}, {4, 3, 0}};
    sort_txns(t);
    CHECK(t[0].id == 2 && t[1].id == 4 && t[2].id == 3 && t[3].id == 1);

    std::vector<uint32_t> acc = {0x10, 0x20, 0x10, 0x30, 0x10, 0x20, 0x40};
    auto hot = top_k_addrs(acc, 2);
    CHECK(hot.size() == 2 && hot[0] == 0x10 && hot[1] == 0x20);

    std::vector<uint32_t> d = {5, 1, 5, 3, 1};
    dedupe(d);
    CHECK(d == std::vector<uint32_t>({1, 3, 5}));

    std::map<uint32_t, Region> mmap = {
        {0x00000000, {0x10000,  "ROM"}},
        {0x20000000, {0x40000,  "SRAM"}},
        {0x40000000, {0x1000,   "UART"}},
        {0x40001000, {0x1000,   "DMA"}},
    };
    CHECK(decode(mmap, 0x00000004) == "ROM");
    CHECK(decode(mmap, 0x2003FFFC) == "SRAM");
    CHECK(decode(mmap, 0x20040000) == "DECERR");      // just past SRAM
    CHECK(decode(mmap, 0x40001010) == "DMA");
    CHECK(decode(mmap, 0x80000000) == "DECERR");

    std::vector<int> lat = {12, 40, 7, 33, 90};
    CHECK(std::count_if(lat.begin(), lat.end(), [](int x) { return x > 30; }) == 3);
    CHECK(std::accumulate(lat.begin(), lat.end(), 0) == 182);
    CHECK(*std::find_if(lat.begin(), lat.end(), [](int x) { return x > 35; }) == 40);
    CHECK(*std::max_element(lat.begin(), lat.end()) == 90);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

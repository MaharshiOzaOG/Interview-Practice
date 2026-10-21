/*
 * 04_hw_data_structures / Q06 : Bitmap allocator & ID pool       Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) class IdPool(n <= 64): alloc() -> lowest free ID or -1, release(id).
 *     Use one uint64_t bitmap (1 = in use). alloc in O(1) with bit tricks.
 * (b) class BlockAlloc (64 blocks): alloc(n) -> start of n CONTIGUOUS free
 *     blocks (first fit) or -1; release(start, n).
 *
 * HW link: AXI masters keep a pool of free transaction IDs for outstanding
 *          requests; a DMA descriptor/buffer allocator works like (b).
 */
#include <cstdint>
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

static int ctz64(uint64_t x) { int n = 0; while (!(x & 1ull)) { x >>= 1; n++; } return n; }

// ======================= SOLUTION =======================
class IdPool {
    uint64_t used_ = 0, valid_;
public:
    explicit IdPool(int n) : valid_(n == 64 ? ~0ull : ((1ull << n) - 1)) {}
    int alloc() {
        uint64_t free_bits = ~used_ & valid_;
        if (!free_bits) return -1;
        uint64_t lowest = free_bits & (~free_bits + 1);   // isolate lowest free bit
        used_ |= lowest;
        return ctz64(lowest);
    }
    void release(int id) { used_ &= ~(1ull << id); }
    int  in_use() const  { int c = 0; uint64_t u = used_; while (u) { u &= u - 1; c++; } return c; }
};

class BlockAlloc {
    uint64_t used_ = 0;
    static uint64_t mask(int n) { return n == 64 ? ~0ull : ((1ull << n) - 1); }
public:
    int alloc(int n) {
        if (n <= 0 || n > 64) return -1;
        uint64_t m = mask(n);
        for (int s = 0; s + n <= 64; s++)
            if (((used_ >> s) & m) == 0) { used_ |= m << s; return s; }
        return -1;
    }
    void release(int start, int n) { used_ &= ~(mask(n) << start); }
};

// ======================= TESTS ==========================
int main() {
    IdPool p(4);
    CHECK(p.alloc() == 0); CHECK(p.alloc() == 1); CHECK(p.alloc() == 2); CHECK(p.alloc() == 3);
    CHECK(p.alloc() == -1);                 // exhausted
    p.release(1);
    CHECK(p.alloc() == 1);                  // lowest free reused
    CHECK(p.in_use() == 4);

    IdPool big(64);
    for (int i = 0; i < 64; i++) CHECK(big.alloc() == i);
    CHECK(big.alloc() == -1);

    BlockAlloc b;
    CHECK(b.alloc(10) == 0);
    CHECK(b.alloc(20) == 10);
    CHECK(b.alloc(30) == 30);
    CHECK(b.alloc(5)  == -1);               // only 4 left
    b.release(10, 20);
    CHECK(b.alloc(4) == 10);                // first fit into the hole
    CHECK(b.alloc(16) == 14);
    CHECK(b.alloc(4) == 60);
    CHECK(b.alloc(1) == -1);
    BlockAlloc whole;
    CHECK(whole.alloc(64) == 0);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

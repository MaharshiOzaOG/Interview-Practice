/*
 * 04_hw_data_structures / Q01 : Circular FIFO (template)        Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * template <typename T, unsigned DEPTH> class Fifo with
 *     push(const T&) -> bool, pop(T&) -> bool, peek(), size(), full(), empty()
 * DEPTH must be a power of two.
 *
 * Use the SAME trick as RTL FIFOs: read/write pointers have one EXTRA bit
 *   - empty : wr_ptr == rd_ptr
 *   - full  : wr_ptr - rd_ptr == DEPTH   (MSBs differ, lower bits equal)
 * so you don't waste a slot or need a separate counter.
 *
 * Follow-ups: what if DEPTH is not a power of two? (use modulo or a count).
 *             Which is the "classic bug"? (full vs empty ambiguity with N-bit ptrs).
 */
#include <cstdio>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
template <typename T, unsigned DEPTH>
class Fifo {
    static_assert(DEPTH && !(DEPTH & (DEPTH - 1)), "DEPTH must be a power of two");
    static constexpr unsigned PTR_MASK  = 2 * DEPTH - 1;   // log2(DEPTH)+1 bits
    static constexpr unsigned ADDR_MASK = DEPTH - 1;
    T mem_[DEPTH];
    unsigned wr_ = 0, rd_ = 0;
public:
    bool     empty() const { return wr_ == rd_; }
    unsigned size()  const { return (wr_ - rd_) & PTR_MASK; }
    bool     full()  const { return size() == DEPTH; }

    bool push(const T& v) {
        if (full()) return false;
        mem_[wr_ & ADDR_MASK] = v;
        wr_ = (wr_ + 1) & PTR_MASK;
        return true;
    }
    bool pop(T& v) {
        if (empty()) return false;
        v = mem_[rd_ & ADDR_MASK];
        rd_ = (rd_ + 1) & PTR_MASK;
        return true;
    }
    const T& peek() const { return mem_[rd_ & ADDR_MASK]; }   // caller checks !empty()
};

// ======================= TESTS ==========================
int main() {
    Fifo<int, 4> f;
    int v;
    CHECK(f.empty() && !f.full() && f.size() == 0);
    CHECK(!f.pop(v));                                   // underflow rejected
    for (int i = 0; i < 4; i++) CHECK(f.push(i));
    CHECK(f.full() && f.size() == 4);
    CHECK(!f.push(99));                                 // overflow rejected
    CHECK(f.peek() == 0);
    CHECK(f.pop(v) && v == 0);
    CHECK(f.push(4));

    // wrap many times: data must stay in order
    int expect = 1;
    for (int i = 5; i < 1000; i++) {
        CHECK(f.pop(v) && v == expect++);
        CHECK(f.push(i));
    }
    CHECK(f.size() == 4);

    Fifo<std::string, 2> s;
    s.push("AW"); s.push("W");
    std::string x;
    CHECK(s.pop(x) && x == "AW");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

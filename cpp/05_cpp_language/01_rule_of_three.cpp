/*
 * 05_cpp_language / Q01 : Rule of three (deep copy)          Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * class Buffer owns a heap array:  size_t n_; int* data_;
 * Write: constructor, destructor, copy constructor, copy assignment.
 *   - copies must be DEEP (independent data)
 *   - assignment must handle self-assignment and be exception-safe
 *     -> copy-and-swap idiom
 * A static counter tracks live allocations so the test can prove no leaks.
 *
 * Interview Q: "What happens with the compiler-generated copy?"
 *   -> shallow copy: two objects share one pointer -> double delete.
 * Rule of three: if you need one of {dtor, copy ctor, copy assign}, you need all.
 */
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <utility>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class Buffer {
    size_t n_;
    int* data_;
public:
    static int live;                                   // allocations alive

    explicit Buffer(size_t n = 0) : n_(n), data_(n ? new int[n]() : nullptr) { if (data_) live++; }

    ~Buffer() { if (data_) { delete[] data_; live--; } }

    Buffer(const Buffer& o) : n_(o.n_), data_(o.n_ ? new int[o.n_] : nullptr) {   // deep copy
        if (data_) { live++; std::copy(o.data_, o.data_ + n_, data_); }
    }

    Buffer& operator=(Buffer o) {      // by value = copy made here (copy-and-swap)
        swap(o);                       // our old data goes into o and dies with it
        return *this;
    }

    void swap(Buffer& o) noexcept { std::swap(n_, o.n_); std::swap(data_, o.data_); }

    size_t size() const { return n_; }
    int&       operator[](size_t i)       { return data_[i]; }
    const int& operator[](size_t i) const { return data_[i]; }
};
int Buffer::live = 0;

// ======================= TESTS ==========================
int main() {
    {
        Buffer a(4);
        for (int i = 0; i < 4; i++) a[i] = i * 10;
        Buffer b = a;                    // copy ctor
        b[0] = 99;
        CHECK(a[0] == 0 && b[0] == 99);  // independent
        CHECK(Buffer::live == 2);

        Buffer c(1);
        c = a;                           // copy assignment
        CHECK(c.size() == 4 && c[3] == 30);
        CHECK(Buffer::live == 3);

        c = c;                           // self-assignment safe
        CHECK(c[2] == 20);

        Buffer empty;
        c = empty;
        CHECK(c.size() == 0 && Buffer::live == 2);
    }
    CHECK(Buffer::live == 0);            // no leaks, no double frees

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

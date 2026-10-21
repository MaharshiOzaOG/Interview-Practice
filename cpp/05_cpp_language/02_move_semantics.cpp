/*
 * 05_cpp_language / Q02 : Move semantics & rule of five          Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Extend a resource-owning class with a move constructor and move assignment.
 * Count copies vs moves to SEE what the language does:
 *   (a) std::move(x) into a vector        -> 1 move, 0 copies
 *   (b) returning a local by value        -> 0 copies (RVO / move)
 *   (c) vector growth                      -> elements are MOVED only if the
 *       move constructor is noexcept (otherwise vector copies for safety)
 *   (d) moved-from object is valid but empty
 *
 * Interview Qs: lvalue vs rvalue; what std::move really does (just a cast to T&&);
 *               why noexcept on move matters.
 */
#include <cstdio>
#include <utility>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct Payload {
    static int copies, moves;
    int* data = nullptr;
    size_t n = 0;

    explicit Payload(size_t len = 0) : data(len ? new int[len]() : nullptr), n(len) {}
    ~Payload() { delete[] data; }

    Payload(const Payload& o) : data(o.n ? new int[o.n] : nullptr), n(o.n) {
        for (size_t i = 0; i < n; i++) data[i] = o.data[i];
        copies++;
    }
    Payload(Payload&& o) noexcept : data(o.data), n(o.n) {      // steal pointer
        o.data = nullptr; o.n = 0;                               // leave o empty
        moves++;
    }
    Payload& operator=(const Payload& o) {
        if (this != &o) { Payload tmp(o); std::swap(data, tmp.data); std::swap(n, tmp.n); }
        return *this;
    }
    Payload& operator=(Payload&& o) noexcept {
        if (this != &o) { delete[] data; data = o.data; n = o.n; o.data = nullptr; o.n = 0; moves++; }
        return *this;
    }
};
int Payload::copies = 0, Payload::moves = 0;

Payload make(size_t n) { Payload p(n); p.data[0] = 7; return p; }

static void reset() { Payload::copies = Payload::moves = 0; }

// ======================= TESTS ==========================
int main() {
    std::vector<Payload> v;
    v.reserve(4);

    reset();
    Payload a(100);
    v.push_back(std::move(a));                    // (a)
    CHECK(Payload::copies == 0 && Payload::moves == 1);
    CHECK(a.data == nullptr && a.n == 0);         // (d) moved-from is empty
    CHECK(v[0].n == 100);

    reset();
    Payload b = make(10);                         // (b)
    CHECK(Payload::copies == 0);
    CHECK(b.data[0] == 7);

    reset();
    std::vector<Payload> w;                       // (c) no reserve -> reallocations
    for (int i = 0; i < 10; i++) w.emplace_back(8);
    CHECK(Payload::copies == 0);                  // only because move ctor is noexcept
    CHECK(Payload::moves > 0);

    reset();
    Payload c(5), d(3);
    d = std::move(c);
    CHECK(d.n == 5 && c.n == 0 && Payload::moves == 1);
    d = d;                                        // copy self-assign safe
    CHECK(d.n == 5);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

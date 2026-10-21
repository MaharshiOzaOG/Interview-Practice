/*
 * 05_cpp_language / Q11 : Implement a simple shared_ptr          Difficulty: Hard
 * ---------------------------------------------------------------------------
 * template <typename T> class SharedPtr:
 *   - constructor from T*, copy ctor/assign (count++), move ctor/assign (steal)
 *   - destructor: count--, delete object + count when it reaches 0
 *   - get(), operator*, operator->, use_count(), reset(), explicit operator bool
 * Non-atomic count is fine for the interview (mention std::shared_ptr uses
 * atomic increments and a control block that also holds the weak count).
 *
 * Also asked as: "implement unique_ptr" -> same but copy deleted, move only.
 */
#include <cstdio>
#include <utility>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
template <typename T>
class SharedPtr {
    T*   ptr_   = nullptr;
    long* count_ = nullptr;

    void release() {
        if (count_ && --(*count_) == 0) { delete ptr_; delete count_; }
        ptr_ = nullptr; count_ = nullptr;
    }
public:
    SharedPtr() = default;
    explicit SharedPtr(T* p) : ptr_(p), count_(p ? new long(1) : nullptr) {}
    ~SharedPtr() { release(); }

    SharedPtr(const SharedPtr& o) : ptr_(o.ptr_), count_(o.count_) { if (count_) ++(*count_); }
    SharedPtr(SharedPtr&& o) noexcept : ptr_(o.ptr_), count_(o.count_) { o.ptr_ = nullptr; o.count_ = nullptr; }

    SharedPtr& operator=(const SharedPtr& o) {
        if (this != &o) { release(); ptr_ = o.ptr_; count_ = o.count_; if (count_) ++(*count_); }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& o) noexcept {
        if (this != &o) { release(); ptr_ = o.ptr_; count_ = o.count_; o.ptr_ = nullptr; o.count_ = nullptr; }
        return *this;
    }

    void reset(T* p = nullptr) { release(); if (p) { ptr_ = p; count_ = new long(1); } }
    T*   get() const        { return ptr_; }
    T&   operator*() const  { return *ptr_; }
    T*   operator->() const { return ptr_; }
    long use_count() const  { return count_ ? *count_ : 0; }
    explicit operator bool() const { return ptr_ != nullptr; }
};

// ======================= TESTS ==========================
static int alive = 0;
struct Obj { int v; explicit Obj(int x) : v(x) { alive++; } ~Obj() { alive--; } };

int main() {
    {
        SharedPtr<Obj> a(new Obj(5));
        CHECK(a.use_count() == 1 && a->v == 5 && (*a).v == 5);
        {
            SharedPtr<Obj> b = a;                    // copy
            CHECK(a.use_count() == 2 && b.get() == a.get());
            SharedPtr<Obj> c;
            c = b;                                   // copy assign
            CHECK(a.use_count() == 3);
        }
        CHECK(a.use_count() == 1 && alive == 1);

        SharedPtr<Obj> m = std::move(a);             // move: no count change
        CHECK(!a && m.use_count() == 1);

        SharedPtr<Obj> other(new Obj(9));
        other = m;                                   // old Obj(9) must be freed
        CHECK(alive == 1 && m.use_count() == 2);

        other = other;                               // self-assignment
        CHECK(other.use_count() == 2);

        m.reset(new Obj(1));
        CHECK(alive == 2 && other.use_count() == 1 && m.use_count() == 1);
    }
    CHECK(alive == 0);                               // everything freed exactly once

    SharedPtr<Obj> empty;
    CHECK(empty.use_count() == 0 && !empty);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

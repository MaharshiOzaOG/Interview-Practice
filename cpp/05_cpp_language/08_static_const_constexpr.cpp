/*
 * 05_cpp_language / Q08 : static, const, constexpr, singleton     Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) static data member: every Txn gets a unique, increasing id
 * (b) const member functions + `mutable` cache (logically const, lazily computed)
 * (c) constexpr function clog2(n) evaluated at COMPILE time (static_assert)
 *     -> like $clog2 for sizing pointer widths
 * (d) Meyers singleton: ConfigDb::instance() with a function-local static
 *     (thread-safe init since C++11) - think uvm_config_db
 * (e) static free function / variable = internal linkage (file-private)
 */
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class Txn {
    static int next_id_;                       // one copy shared by all objects
    int id_;
    uint32_t data_[4];
    mutable bool parity_valid_ = false;        // cache may change in const methods
    mutable int  parity_ = 0;
public:
    static int parity_computations;
    explicit Txn(uint32_t seed) : id_(next_id_++) { for (int i = 0; i < 4; i++) data_[i] = seed * (i + 1); }
    int id() const { return id_; }
    int parity() const {                       // const: callers see no change
        if (!parity_valid_) {
            uint32_t x = data_[0] ^ data_[1] ^ data_[2] ^ data_[3];
            int p = 0; while (x) { p ^= 1; x &= x - 1; }
            parity_ = p; parity_valid_ = true; parity_computations++;
        }
        return parity_;
    }
    static void reset_ids() { next_id_ = 0; }  // static method: no `this`
};
int Txn::next_id_ = 0;
int Txn::parity_computations = 0;

constexpr int clog2(uint64_t n) {             // ceil(log2(n)), clog2(1) = 0
    int r = 0;
    uint64_t v = 1;
    while (v < n) { v <<= 1; r++; }
    return r;
}
static_assert(clog2(1) == 0 && clog2(8) == 3 && clog2(9) == 4, "computed by the compiler");
constexpr int FIFO_DEPTH = 24;
constexpr int PTR_W = clog2(FIFO_DEPTH) + 1;  // usable as an array size / template arg
int ptr_storage[PTR_W];

class ConfigDb {
    std::map<std::string, int> kv_;
    ConfigDb() = default;                      // nobody else can construct
public:
    ConfigDb(const ConfigDb&) = delete;
    ConfigDb& operator=(const ConfigDb&) = delete;
    static ConfigDb& instance() { static ConfigDb db; return db; }
    void set(const std::string& k, int v) { kv_[k] = v; }
    bool get(const std::string& k, int& v) const {
        auto it = kv_.find(k);
        if (it == kv_.end()) return false;
        v = it->second; return true;
    }
};

// ======================= TESTS ==========================
int main() {
    Txn::reset_ids();
    Txn a(1), b(2), c(3);
    CHECK(a.id() == 0 && b.id() == 1 && c.id() == 2);

    const Txn& r = a;
    int p1 = r.parity(), p2 = r.parity();
    CHECK(p1 == p2 && Txn::parity_computations == 1);   // computed once, cached

    CHECK(PTR_W == 6);
    CHECK(sizeof(ptr_storage) / sizeof(int) == 6);

    ConfigDb::instance().set("num_masters", 2);
    int v = 0;
    CHECK(ConfigDb::instance().get("num_masters", v) && v == 2);
    CHECK(&ConfigDb::instance() == &ConfigDb::instance());   // same object every time
    CHECK(!ConfigDb::instance().get("missing", v));

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

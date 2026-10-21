/*
 * 05_cpp_language / Q10 : Callbacks & observer (analysis-port style)   Medium
 * ---------------------------------------------------------------------------
 * template <typename T> class AnalysisPort:
 *     int  connect(std::function<void(const T&)> cb)  -> returns a handle
 *     void disconnect(int handle)
 *     void write(const T& t)                           -> calls every subscriber
 * A monitor publishes transactions; a scoreboard (member function via lambda)
 * and a coverage collector (free function) subscribe.
 *
 * Concepts : std::function, lambdas capturing `this`, decoupling producer
 *            from consumers (the monitor doesn't know who listens).
 */
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

struct Txn { uint32_t addr; uint32_t data; bool write; };

// ======================= SOLUTION =======================
template <typename T>
class AnalysisPort {
    std::map<int, std::function<void(const T&)>> subs_;
    int next_ = 0;
public:
    int  connect(std::function<void(const T&)> cb) { subs_[next_] = std::move(cb); return next_++; }
    void disconnect(int h) { subs_.erase(h); }
    void write(const T& t) const { for (auto& kv : subs_) kv.second(t); }
    size_t size() const { return subs_.size(); }
};

class Monitor {
public:
    AnalysisPort<Txn> ap;
    void observe(uint32_t a, uint32_t d, bool w) { ap.write({a, d, w}); }  // saw bus activity
};

class Scoreboard {
public:
    std::vector<Txn> seen;
    void on_txn(const Txn& t) { seen.push_back(t); }
};

static int writes_covered = 0, reads_covered = 0;
void coverage_sample(const Txn& t) { (t.write ? writes_covered : reads_covered)++; }

// ======================= TESTS ==========================
int main() {
    Monitor mon;
    Scoreboard sb;
    int h_sb  = mon.ap.connect([&sb](const Txn& t) { sb.on_txn(t); });   // member via lambda
    int h_cov = mon.ap.connect(coverage_sample);                          // free function
    (void)h_cov;
    CHECK(mon.ap.size() == 2);

    mon.observe(0x100, 0xAA, true);
    mon.observe(0x104, 0xBB, false);
    CHECK(sb.seen.size() == 2 && sb.seen[1].addr == 0x104);
    CHECK(writes_covered == 1 && reads_covered == 1);

    mon.ap.disconnect(h_sb);
    mon.observe(0x108, 0xCC, true);
    CHECK(sb.seen.size() == 2);          // scoreboard no longer subscribed
    CHECK(writes_covered == 2);          // coverage still is

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

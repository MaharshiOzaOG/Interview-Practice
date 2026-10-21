/*
 * 06_verification_modeling / Q03 : In-order scoreboard            Difficulty: Medium
 * ---------------------------------------------------------------------------
 * class Scoreboard<T>:
 *   add_expected(T)   from the reference model / predictor
 *   add_actual(T)     from the output monitor; compares with the OLDEST expected
 *   report()          -> errors include: mismatches, unexpected actuals
 *                        (nothing expected), and leftover expected at end of test
 * Keep counters: matched, mismatched, unexpected, missing.
 *
 * Interview Q: "Your test passes but the DUT dropped the last 3 packets. How?"
 *   -> the scoreboard never checked for leftover expected items at end-of-test
 *      (check_phase). Always do the leftover check.
 */
#include <cstdint>
#include <cstdio>
#include <deque>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
template <typename T>
class Scoreboard {
    std::deque<T> exp_;
public:
    int matched = 0, mismatched = 0, unexpected = 0, missing = 0;

    void add_expected(const T& t) { exp_.push_back(t); }

    void add_actual(const T& t) {
        if (exp_.empty()) { unexpected++; return; }      // DUT produced extra output
        T e = exp_.front();
        exp_.pop_front();
        if (e == t) matched++;
        else        mismatched++;
    }

    bool report() {                                      // end-of-test check
        missing = (int)exp_.size();
        return mismatched == 0 && unexpected == 0 && missing == 0;
    }
};

// ======================= TESTS ==========================
int main() {
    Scoreboard<uint32_t> good;
    for (uint32_t i = 0; i < 100; i++) good.add_expected(i * 3);
    for (uint32_t i = 0; i < 100; i++) good.add_actual(i * 3);
    CHECK(good.report() && good.matched == 100);

    Scoreboard<uint32_t> bad_data;
    bad_data.add_expected(1); bad_data.add_expected(2);
    bad_data.add_actual(1);   bad_data.add_actual(7);
    CHECK(!bad_data.report() && bad_data.mismatched == 1);

    Scoreboard<uint32_t> dropped;
    for (uint32_t i = 0; i < 5; i++) dropped.add_expected(i);
    for (uint32_t i = 0; i < 2; i++) dropped.add_actual(i);
    CHECK(!dropped.report() && dropped.missing == 3);

    Scoreboard<uint32_t> extra;
    extra.add_actual(42);
    CHECK(!extra.report() && extra.unexpected == 1);

    Scoreboard<std::string> s;                            // works for any comparable type
    s.add_expected("pkt0"); s.add_actual("pkt0");
    CHECK(s.report());

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

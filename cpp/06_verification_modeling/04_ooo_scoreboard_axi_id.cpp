/*
 * 06_verification_modeling / Q04 : Out-of-order scoreboard by AXI ID   Difficulty: Hard
 * ---------------------------------------------------------------------------
 * AXI rule: responses with the SAME ID must return in order; DIFFERENT IDs may
 * return in any order. Build a scoreboard that enforces exactly that:
 *   add_expected(Resp)  -> queued per ID (unordered_map<int, deque<Resp>>)
 *   add_actual(Resp)    -> compare with the head of THAT ID's queue
 * Classify every error:
 *   UNEXPECTED   : no outstanding expectation for that ID
 *   ORDER        : matches a LATER entry of the same ID (same-ID reordering bug)
 *   DATA         : matches nothing -> data corruption (head consumed)
 *   MISSING      : leftover expectations at end of test
 *
 * HW link: exactly the scoreboard for an AXI interconnect / your DMA subsystem
 *          with multiple outstanding IDs.
 */
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

struct Resp {
    int id; uint32_t addr; uint32_t data;
    bool operator==(const Resp& o) const { return id == o.id && addr == o.addr && data == o.data; }
};

// ======================= SOLUTION =======================
class OooScoreboard {
    std::unordered_map<int, std::deque<Resp>> q_;
public:
    int matched = 0, unexpected = 0, order_err = 0, data_err = 0, missing = 0;
    std::vector<std::string> log;

    void add_expected(const Resp& r) { q_[r.id].push_back(r); }

    void add_actual(const Resp& r) {
        auto it = q_.find(r.id);
        if (it == q_.end() || it->second.empty()) {
            unexpected++; log.push_back("UNEXPECTED id=" + std::to_string(r.id)); return;
        }
        auto& dq = it->second;
        if (dq.front() == r) { dq.pop_front(); matched++; return; }

        for (size_t i = 1; i < dq.size(); i++)          // did it jump the queue?
            if (dq[i] == r) {
                order_err++;
                log.push_back("ORDER id=" + std::to_string(r.id));
                dq.erase(dq.begin() + (long)i);
                return;
            }
        data_err++;                                     // matches nothing
        log.push_back("DATA id=" + std::to_string(r.id));
        dq.pop_front();
    }

    bool report() {
        missing = 0;
        for (auto& kv : q_) missing += (int)kv.second.size();
        return unexpected == 0 && order_err == 0 && data_err == 0 && missing == 0;
    }
};

// ======================= TESTS ==========================
int main() {
    // issue order: A0(id0), B0(id1), A1(id0), C0(id2), B1(id1)
    Resp A0{0, 0x00, 1}, A1{0, 0x04, 2}, B0{1, 0x10, 3}, B1{1, 0x14, 4}, C0{2, 0x20, 5};

    OooScoreboard ok;
    for (auto r : {A0, B0, A1, C0, B1}) ok.add_expected(r);
    for (auto r : {C0, B0, A0, B1, A1}) ok.add_actual(r);      // legal: per-ID order kept
    CHECK(ok.report() && ok.matched == 5);

    OooScoreboard ord;
    for (auto r : {A0, A1}) ord.add_expected(r);
    ord.add_actual(A1);                                        // same ID returned early
    ord.add_actual(A0);
    CHECK(!ord.report() && ord.order_err == 1);

    OooScoreboard dat;
    dat.add_expected(B0);
    dat.add_actual(Resp{1, 0x10, 0xBAD});
    CHECK(!dat.report() && dat.data_err == 1 && dat.log[0] == "DATA id=1");

    OooScoreboard unx;
    unx.add_expected(A0);
    unx.add_actual(C0);                                        // nobody expected id 2
    CHECK(!unx.report() && unx.unexpected == 1 && unx.missing == 1);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

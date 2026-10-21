/*
 * 06_verification_modeling / Q10 : Cycle-based model of a valid/ready pipeline   Hard
 * ---------------------------------------------------------------------------
 * Model N pipeline registers between a producer and a consumer with valid/ready
 * back-pressure, simulated cycle by cycle in TWO PHASES (like RTL):
 *   1. combinational: compute ready from the back:  rdy[i] = !v[i] || rdy[i+1]
 *   2. sequential   : compute ALL next-state values from current values,
 *                     then commit them together (= non-blocking assignments)
 * Check:
 *   - every item arrives exactly once and in order, under random consumer stalls
 *   - with the consumer always ready, throughput = 1 item/cycle and
 *     total cycles = items + N (pipeline latency)
 *
 * Why: emulation/verification engineers often write C++ cycle models; the classic
 * bug is updating registers in place (blocking) so data "races" through stages.
 */
#include <cstdio>
#include <initializer_list>
#include <random>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct Pipeline {
    int n;
    std::vector<bool> v;
    std::vector<int>  d;
    explicit Pipeline(int stages) : n(stages), v(stages, false), d(stages, 0) {}

    // returns cycles taken; `got` collects consumer output
    int run(int items, double ready_prob, unsigned seed, std::vector<int>& got) {
        std::mt19937 rng(seed);
        std::bernoulli_distribution rdy_dist(ready_prob);
        int next = 0, cycle = 0;
        while ((int)got.size() < items && cycle < 100000) {
            bool cons_ready = rdy_dist(rng);

            // ---- phase 1: combinational ready chain (back to front) ----
            std::vector<bool> rdy(n + 1);
            rdy[n] = cons_ready;
            for (int i = n - 1; i >= 0; i--) rdy[i] = !v[i] || rdy[i + 1];

            bool prod_valid = next < items;
            if (v[n - 1] && cons_ready) got.push_back(d[n - 1]);     // consumer handshake

            // ---- phase 2: compute next state from CURRENT values ----
            std::vector<bool> nv = v;
            std::vector<int>  nd = d;
            for (int i = 0; i < n; i++) {
                if (rdy[i]) {                                         // stage i loads
                    bool in_v = (i == 0) ? prod_valid : v[i - 1];
                    int  in_d = (i == 0) ? next       : d[i - 1];
                    nv[i] = in_v;
                    if (in_v) nd[i] = in_d;
                }
            }
            if (prod_valid && rdy[0]) next++;                         // producer handshake
            v = nv;  d = nd;                                          // commit (clock edge)
            cycle++;
        }
        return cycle;
    }
};

// ======================= TESTS ==========================
int main() {
    for (int stages : {1, 2, 4}) {
        Pipeline p(stages);
        std::vector<int> got;
        int cycles = p.run(100, 1.0, 1, got);
        CHECK(cycles == 100 + stages);              // full throughput + latency
        bool order = true;
        for (int i = 0; i < 100; i++) if (got[i] != i) order = false;
        CHECK(order);
    }

    for (unsigned seed = 0; seed < 20; seed++) {    // random back-pressure
        Pipeline p(3);
        std::vector<int> got;
        p.run(500, 0.4, seed, got);
        bool ok = got.size() == 500;
        for (int i = 0; ok && i < 500; i++) if (got[i] != i) ok = false;
        CHECK(ok);                                  // no drops, no duplicates
    }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

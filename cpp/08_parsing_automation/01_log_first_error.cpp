/*
 * 08_parsing_automation / Q01 : Parse a UVM simulation/emulation log   Difficulty: Medium
 * ---------------------------------------------------------------------------
 * Given log text like
 *     UVM_INFO  tb.sv(42) @ 0: reporter [TEST] starting
 *     UVM_ERROR sb.sv(88) @ 1250: uvm_test_top.env.sb [SB] data mismatch exp=0x10 act=0x11
 * write  LogSummary parse_log(const std::string& text)  returning:
 *   - count per severity (INFO, WARNING, ERROR, FATAL)
 *   - time and message of the FIRST error or fatal (debug always starts there)
 *   - the set of reporting IDs that produced errors ([SB], [AXI] ...)
 *   - pass = no ERROR and no FATAL
 * Tools: std::istringstream + getline, string::find / substr / stoull.
 * (Same task is usually asked in Python - see the Python section later.)
 */
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <sstream>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct LogSummary {
    std::map<std::string, int> counts;
    bool has_error = false;
    uint64_t first_err_time = 0;
    std::string first_err_msg;
    std::set<std::string> err_ids;
    bool pass = true;
};

LogSummary parse_log(const std::string& text) {
    LogSummary s;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (line.compare(0, 4, "UVM_") != 0) continue;            // not a report line
        std::string sev = line.substr(4, line.find(' ') - 4);     // INFO / ERROR / ...
        s.counts[sev]++;
        if (sev != "ERROR" && sev != "FATAL") continue;

        s.pass = false;
        size_t at  = line.find(" @ ");
        size_t col = line.find(':', at);
        size_t lb  = line.find('[', col), rb = line.find(']', lb);
        if (lb != std::string::npos && rb != std::string::npos)
            s.err_ids.insert(line.substr(lb + 1, rb - lb - 1));
        if (!s.has_error && at != std::string::npos && col != std::string::npos) {
            s.has_error = true;
            s.first_err_time = std::stoull(line.substr(at + 3, col - at - 3));
            s.first_err_msg  = (rb != std::string::npos) ? line.substr(rb + 2) : "";
        }
    }
    return s;
}

// ======================= TESTS ==========================
int main() {
    const std::string log = R"(# compile done
UVM_INFO tb.sv(42) @ 0: reporter [TEST] starting test axi_dma_basic
UVM_WARNING drv.sv(10) @ 500: uvm_test_top.env.agt.drv [DRV] slow ready
UVM_ERROR sb.sv(88) @ 1250: uvm_test_top.env.sb [SB] data mismatch exp=0x10 act=0x11
UVM_INFO mon.sv(77) @ 1300: uvm_test_top.env.agt.mon [MON] beat 3
UVM_ERROR axi_chk.sv(5) @ 2000: uvm_test_top.env.chk [AXI] WLAST missing
UVM_FATAL tb.sv(99) @ 9000: reporter [TIMEOUT] watchdog expired
)";
    LogSummary s = parse_log(log);
    CHECK(s.counts["INFO"] == 2 && s.counts["WARNING"] == 1);
    CHECK(s.counts["ERROR"] == 2 && s.counts["FATAL"] == 1);
    CHECK(s.has_error && s.first_err_time == 1250);
    CHECK(s.first_err_msg == "data mismatch exp=0x10 act=0x11");
    CHECK(s.err_ids == std::set<std::string>({"SB", "AXI", "TIMEOUT"}));
    CHECK(!s.pass);

    LogSummary clean = parse_log("UVM_INFO a.sv(1) @ 0: r [T] hi\nUVM_INFO a.sv(2) @ 5: r [T] bye\n");
    CHECK(clean.pass && !clean.has_error && clean.counts["INFO"] == 2);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

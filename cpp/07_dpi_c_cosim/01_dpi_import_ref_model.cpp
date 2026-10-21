/*
 * 07_dpi_c_cosim / Q01 : DPI-C import - C reference model called from SV   Medium
 * ---------------------------------------------------------------------------
 * Write the C side of these SystemVerilog imports:
 *
 *   import "DPI-C" function int unsigned c_mac(input int unsigned acc,
 *                                              input shortint a, input shortint b);
 *   import "DPI-C" function void c_checksum(input byte unsigned data[16],   // fixed array
 *                                           output int unsigned sum,
 *                                           output bit odd_parity);
 *   import "DPI-C" context function int c_log(input string msg);
 *
 * DPI type mapping (memorise this table):
 *   SV int            <-> int            SV int unsigned     <-> unsigned int
 *   SV shortint       <-> short          SV longint          <-> long long
 *   SV byte unsigned  <-> unsigned char  SV bit (scalar)     <-> svBit (unsigned char)
 *   SV string         <-> const char*    SV bit[N:0] packed  <-> svBitVecVal* (uint32 words)
 *   output/inout args <-> POINTERS       fixed-size array    <-> pointer to first element
 *   open array []     <-> svOpenArrayHandle (needs svdpi.h)
 * Rules: functions must be extern "C" (no C++ name mangling); pure functions
 *        must not call back into SV; `context` lets C call exported SV tasks.
 *
 * svdpi.h is not available on online compilers, so its two types are
 * typedef'd below. main() plays the role of the SV testbench.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// In a real flow:  #include "svdpi.h"
typedef unsigned char svBit;
typedef uint32_t      svBitVecVal;

// ======================= SOLUTION =======================
extern "C" {

// saturating multiply-accumulate: acc + a*b, clamped to 32-bit unsigned range
unsigned int c_mac(unsigned int acc, short a, short b) {
    long long r = (long long)acc + (long long)a * b;
    if (r < 0)          return 0;
    if (r > 0xFFFFFFFF) return 0xFFFFFFFFu;
    return (unsigned int)r;
}

void c_checksum(const unsigned char* data, unsigned int* sum, svBit* odd_parity) {
    unsigned int s = 0, x = 0;
    for (int i = 0; i < 16; i++) { s += data[i]; x ^= data[i]; }
    int p = 0;
    while (x) { p ^= 1; x &= x - 1; }
    *sum = s;                          // outputs are written through pointers
    *odd_parity = (svBit)p;
}

static std::vector<std::string> g_log;   // what C saw (for the test)
int c_log(const char* msg) {           // SV string arrives as const char*
    g_log.push_back(msg);
    return (int)std::strlen(msg);
}

} // extern "C"

// ======================= TESTS (main = the SV side) ==========================
int main() {
    CHECK(c_mac(10, 3, 4) == 22);
    CHECK(c_mac(0, -3, 4) == 0);                    // clamps at 0
    CHECK(c_mac(0xFFFFFFF0u, 100, 100) == 0xFFFFFFFFu);
    CHECK(c_mac(100, -5, 10) == 50);

    unsigned char pkt[16];
    for (int i = 0; i < 16; i++) pkt[i] = (unsigned char)(i + 1);
    unsigned int sum = 0; svBit odd = 9;
    c_checksum(pkt, &sum, &odd);
    CHECK(sum == 136);                              // 1+2+...+16
    CHECK(odd == 1);                                // XOR(1..16) = 16 = 0b10000 -> one 1-bit

    CHECK(c_log("UVM_INFO dpi up") == 15);
    CHECK(g_log.size() == 1 && g_log[0] == "UVM_INFO dpi up");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

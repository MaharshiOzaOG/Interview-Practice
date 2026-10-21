/*
 * 05_cpp_language / Q06 : Operator overloading - a Verilog-like bit vector   Medium
 * ---------------------------------------------------------------------------
 * class BitVec(width <= 64, value):
 *   & | ^ ~           bitwise, result keeps the width
 *   << >>             shifts, bits falling off the top are lost
 *   + (wraps)         like a W-bit adder
 *   == !=
 *   operator[](i)     read bit i
 *   slice(msb, lsb)   like v[msb:lsb]
 *   concat(a, b)      like {a, b}
 *   operator<<(ostream) prints Verilog style: 8'ha5
 */
#include <cstdint>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class BitVec {
    int w_;
    uint64_t v_;
    static uint64_t mask(int w) { return w >= 64 ? ~0ull : ((1ull << w) - 1); }
public:
    BitVec(int width, uint64_t value) : w_(width), v_(value & mask(width)) {}
    int width() const { return w_; }
    uint64_t value() const { return v_; }

    BitVec operator&(const BitVec& o) const { return BitVec(w_, v_ & o.v_); }
    BitVec operator|(const BitVec& o) const { return BitVec(w_, v_ | o.v_); }
    BitVec operator^(const BitVec& o) const { return BitVec(w_, v_ ^ o.v_); }
    BitVec operator~() const                { return BitVec(w_, ~v_); }
    BitVec operator<<(int n) const          { return BitVec(w_, n >= 64 ? 0 : v_ << n); }
    BitVec operator>>(int n) const          { return BitVec(w_, n >= 64 ? 0 : v_ >> n); }
    BitVec operator+(const BitVec& o) const { return BitVec(w_, v_ + o.v_); }
    bool operator==(const BitVec& o) const  { return w_ == o.w_ && v_ == o.v_; }
    bool operator!=(const BitVec& o) const  { return !(*this == o); }
    bool operator[](int i) const            { return (v_ >> i) & 1u; }

    BitVec slice(int msb, int lsb) const { return BitVec(msb - lsb + 1, v_ >> lsb); }

    friend BitVec concat(const BitVec& a, const BitVec& b) {
        return BitVec(a.w_ + b.w_, (a.v_ << b.w_) | b.v_);
    }
    friend std::ostream& operator<<(std::ostream& os, const BitVec& b) {
        std::ostringstream hex;
        hex << std::hex << b.v_;
        return os << b.w_ << "'h" << hex.str();
    }
};

static std::string str(const BitVec& b) { std::ostringstream s; s << b; return s.str(); }

// ======================= TESTS ==========================
int main() {
    BitVec a(8, 0xA5), b(8, 0x0F);
    CHECK((a & b) == BitVec(8, 0x05));
    CHECK((a | b) == BitVec(8, 0xAF));
    CHECK((a ^ b) == BitVec(8, 0xAA));
    CHECK(~a == BitVec(8, 0x5A));                 // stays 8 bits
    CHECK((a << 4) == BitVec(8, 0x50));           // top bits drop off
    CHECK((a >> 4) == BitVec(8, 0x0A));
    CHECK((BitVec(8, 0xFF) + BitVec(8, 1)) == BitVec(8, 0));
    CHECK(a[0] && !a[1] && a[7]);
    CHECK(a.slice(7, 4) == BitVec(4, 0xA));
    CHECK(concat(BitVec(4, 0x3), BitVec(8, 0xC4)) == BitVec(12, 0x3C4));
    CHECK(BitVec(4, 1) != BitVec(8, 1));          // width matters
    CHECK(str(a) == "8'ha5");
    CHECK(str(BitVec(1, 1)) == "1'h1");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

/*
 * 05_cpp_language / Q04 : Templates                               Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) Function template  max_of(a, b)  and a variadic  sum_all(args...)
 * (b) Class template with a NON-TYPE parameter:
 *        template <int W> class UInt   // W-bit unsigned that wraps like Verilog
 *     + - & | operators keep the result masked to W bits; static_assert(W in 1..64)
 * (c) Full specialisation: describe<T>() returns "generic", but "bool" for bool.
 * (d) if constexpr (C++17): to_hex<T>() picks the digit count from sizeof(T).
 *
 * HW link: UInt<W> is how C++ models (SystemC sc_uint, Verilator, HLS ap_uint)
 *          represent arbitrary-width signals.
 */
#include <cstdint>
#include <cstdio>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
template <typename T>
T max_of(T a, T b) { return a < b ? b : a; }

template <typename... Ts>
auto sum_all(Ts... xs) { return (xs + ... + 0); }            // C++17 fold expression

template <int W>
class UInt {
    static_assert(W >= 1 && W <= 64, "width must be 1..64");
    uint64_t v_;
public:
    static constexpr uint64_t MASK = (W == 64) ? ~0ull : ((1ull << W) - 1);
    constexpr UInt(uint64_t v = 0) : v_(v & MASK) {}
    constexpr uint64_t value() const { return v_; }
    constexpr UInt operator+(UInt o) const { return UInt(v_ + o.v_); }   // wraps at 2^W
    constexpr UInt operator-(UInt o) const { return UInt(v_ - o.v_); }
    constexpr UInt operator&(UInt o) const { return UInt(v_ & o.v_); }
    constexpr UInt operator|(UInt o) const { return UInt(v_ | o.v_); }
    constexpr bool operator==(UInt o) const { return v_ == o.v_; }
    constexpr bool bit(int i) const { return (v_ >> i) & 1u; }
};

template <typename T> std::string describe()       { return "generic"; }
template <>           std::string describe<bool>() { return "bool"; }

template <typename T>
std::string to_hex(T v) {
    char buf[32];
    if constexpr (sizeof(T) <= 4) std::snprintf(buf, sizeof buf, "0x%08X", (unsigned)v);
    else                          std::snprintf(buf, sizeof buf, "0x%016llX", (unsigned long long)v);
    return buf;
}

// ======================= TESTS ==========================
int main() {
    CHECK(max_of(3, 9) == 9);
    CHECK(max_of(std::string("ab"), std::string("b")) == "b");
    CHECK(sum_all(1, 2, 3, 4) == 10);
    CHECK(sum_all() == 0);

    UInt<4> a(9), b(10);
    CHECK((a + b).value() == 3);        // 19 mod 16
    CHECK((UInt<4>(0) - UInt<4>(1)).value() == 15);
    CHECK(UInt<4>(0x1F).value() == 0xF);
    CHECK((a & b).value() == 8 && (a | b).value() == 11);
    CHECK(a.bit(3) && !a.bit(1));
    static_assert(UInt<12>::MASK == 0xFFF, "compile-time check works");
    CHECK(UInt<64>(~0ull).value() == ~0ull);

    CHECK(describe<int>() == "generic");
    CHECK(describe<bool>() == "bool");
    CHECK(to_hex<uint32_t>(0xBEEF) == "0x0000BEEF");
    CHECK(to_hex<uint64_t>(0x1ull) == "0x0000000000000001");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

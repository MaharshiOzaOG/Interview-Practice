/*
 * 06_verification_modeling / Q09 : Hamming SECDED ECC (13,8)      Difficulty: Hard
 * ---------------------------------------------------------------------------
 * Encode 8 data bits into 13 bits:
 *   positions 1..12 : Hamming(12,8) - parity at 1,2,4,8; data at 3,5,6,7,9,10,11,12
 *   position 0      : overall parity of bits 1..12  (adds Double-Error Detect)
 * decode(code) -> {data, status} where status is
 *   OK            syndrome == 0, overall parity OK
 *   CORRECTED     overall parity wrong -> single error at position `syndrome`
 *                 (syndrome 0 means the overall parity bit itself flipped)
 *   DOUBLE_ERROR  syndrome != 0 but overall parity OK -> uncorrectable
 * Syndrome = XOR of the POSITIONS of all bits that are 1.
 * Test exhaustively: all 256 values x every single flip x every double flip.
 *
 * HW link: memory ECC in SoCs (DDR, SRAM), common Qualcomm/NVIDIA question.
 */
#include <cstdint>
#include <cstdio>
#include <initializer_list>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
enum class Ecc { OK, CORRECTED, DOUBLE_ERROR };
static const int DATA_POS[8] = {3, 5, 6, 7, 9, 10, 11, 12};

static int syndrome_of(uint16_t code) {           // XOR of positions holding a 1
    int s = 0;
    for (int p = 1; p <= 12; p++) if ((code >> p) & 1) s ^= p;
    return s;
}
static int parity13(uint16_t code) { int p = 0; for (int i = 0; i < 13; i++) p ^= (code >> i) & 1; return p; }

uint16_t encode(uint8_t d) {
    uint16_t c = 0;
    for (int i = 0; i < 8; i++) if ((d >> i) & 1) c |= (uint16_t)(1u << DATA_POS[i]);
    int s = syndrome_of(c);                        // set parity bits so syndrome becomes 0
    for (int p : {1, 2, 4, 8}) if (s & p) c |= (uint16_t)(1u << p);
    if (parity13(c)) c |= 1;                       // overall even parity
    return c;
}

uint8_t extract(uint16_t c) {
    uint8_t d = 0;
    for (int i = 0; i < 8; i++) if ((c >> DATA_POS[i]) & 1) d |= (uint8_t)(1u << i);
    return d;
}

Ecc decode(uint16_t c, uint8_t& data) {
    int s = syndrome_of(c);
    int p = parity13(c);
    Ecc st = Ecc::OK;
    if (p) {                                       // odd number of flips -> assume 1
        c ^= (uint16_t)(1u << s);                  // s == 0 flips the overall parity bit
        st = Ecc::CORRECTED;
    } else if (s) {
        st = Ecc::DOUBLE_ERROR;
    }
    data = extract(c);
    return st;
}

// ======================= TESTS ==========================
int main() {
    int single_ok = 0, double_ok = 0;
    for (int d = 0; d < 256; d++) {
        uint16_t c = encode((uint8_t)d);
        uint8_t out;
        CHECK(syndrome_of(c) == 0 && parity13(c) == 0);
        CHECK(decode(c, out) == Ecc::OK && out == d);

        for (int i = 0; i < 13; i++) {             // every single-bit error corrected
            uint16_t e = c ^ (uint16_t)(1u << i);
            if (decode(e, out) == Ecc::CORRECTED && out == d) single_ok++;
        }
        for (int i = 0; i < 13; i++)               // every double-bit error detected
            for (int j = i + 1; j < 13; j++) {
                uint16_t e = c ^ (uint16_t)(1u << i) ^ (uint16_t)(1u << j);
                if (decode(e, out) == Ecc::DOUBLE_ERROR) double_ok++;
            }
    }
    CHECK(single_ok == 256 * 13);
    CHECK(double_ok == 256 * 78);                  // C(13,2) = 78
    CHECK(encode(0) == 0);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

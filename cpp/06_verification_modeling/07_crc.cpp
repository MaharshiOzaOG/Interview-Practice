/*
 * 06_verification_modeling / Q07 : CRC-32 and CRC-16             Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) crc32_bitwise(data, len) : IEEE 802.3 CRC-32 (reflected, poly 0xEDB88320,
 *                                init 0xFFFFFFFF, final XOR 0xFFFFFFFF)
 * (b) crc32_table             : same result, 256-entry lookup table (8x faster)
 * (c) crc16_ccitt(data, len)  : NON-reflected, poly 0x1021, init 0xFFFF
 * Standard check: CRC of ASCII "123456789"
 *      CRC-32 = 0xCBF43926,   CRC-16/CCITT-FALSE = 0x29B1
 *
 * HW link: the RTL CRC is an LFSR of XOR gates; the C model is the golden
 *          reference for Ethernet/PCIe/USB packet checkers.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint32_t crc32_bitwise(const uint8_t* p, size_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    while (n--) {
        crc ^= *p++;                              // reflected: data enters at LSB
        for (int k = 0; k < 8; k++)
            crc = (crc & 1u) ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
    }
    return ~crc;
}

static uint32_t table[256];
void crc32_init_table() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) c = (c & 1u) ? (c >> 1) ^ 0xEDB88320u : c >> 1;
        table[i] = c;                             // CRC contribution of one byte
    }
}
uint32_t crc32_table(const uint8_t* p, size_t n) {
    uint32_t crc = 0xFFFFFFFFu;
    while (n--) crc = (crc >> 8) ^ table[(crc ^ *p++) & 0xFFu];
    return ~crc;
}

uint16_t crc16_ccitt(const uint8_t* p, size_t n) {
    uint16_t crc = 0xFFFF;
    while (n--) {
        crc ^= (uint16_t)(*p++ << 8);             // non-reflected: data enters at MSB
        for (int k = 0; k < 8; k++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

// ======================= TESTS ==========================
int main() {
    crc32_init_table();
    const uint8_t* s = (const uint8_t*)"123456789";
    CHECK(crc32_bitwise(s, 9) == 0xCBF43926u);
    CHECK(crc32_table(s, 9) == 0xCBF43926u);
    CHECK(crc16_ccitt(s, 9) == 0x29B1);
    CHECK(crc32_bitwise(s, 0) == 0);

    uint8_t buf[1000];
    for (int i = 0; i < 1000; i++) buf[i] = (uint8_t)(i * 31 + 7);
    CHECK(crc32_bitwise(buf, 1000) == crc32_table(buf, 1000));

    uint32_t good = crc32_table(buf, 1000);       // a single flipped bit is always caught
    buf[500] ^= 0x10;
    CHECK(crc32_table(buf, 1000) != good);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

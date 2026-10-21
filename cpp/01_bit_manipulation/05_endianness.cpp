/*
 * 01_bit_manipulation / Q05 : Endianness                          Difficulty: Easy
 * ---------------------------------------------------------------------------
 * (a) bswap16 / bswap32 / bswap64 using shifts and masks
 * (b) is_little_endian() : detect host byte order at run time
 * (c) read_be32 / read_le32 : build a uint32 from a byte array in each order
 *
 * Interview Q: "Write 0x11223344 to address A. What byte is at A on a
 *               little-endian CPU?"  -> 0x44 (least significant byte first).
 * HW link     : CPU (LE) talking to network/PCIe config/big-endian IP; memory
 *               dumps in emulation are often byte-swapped.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
uint16_t bswap16(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }

uint32_t bswap32(uint32_t x) {
    return  (x >> 24) |
           ((x >>  8) & 0x0000FF00u) |
           ((x <<  8) & 0x00FF0000u) |
            (x << 24);
}

uint64_t bswap64(uint64_t x) {
    return ((uint64_t)bswap32((uint32_t)x) << 32) | bswap32((uint32_t)(x >> 32));
}

bool is_little_endian() {
    uint32_t v = 1;
    uint8_t first;
    std::memcpy(&first, &v, 1);      // memcpy = well-defined way to look at the bytes
    return first == 1;
}

uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}
uint32_t read_le32(const uint8_t* p) {
    return ((uint32_t)p[3] << 24) | ((uint32_t)p[2] << 16) | ((uint32_t)p[1] << 8) | p[0];
}

// ======================= TESTS ==========================
int main() {
    CHECK(bswap16(0xABCD) == 0xCDAB);
    CHECK(bswap32(0x11223344u) == 0x44332211u);
    CHECK(bswap64(0x0102030405060708ull) == 0x0807060504030201ull);
    CHECK(bswap32(bswap32(0xDEADBEEFu)) == 0xDEADBEEFu);

    uint8_t bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
    CHECK(read_be32(bytes) == 0xDEADBEEFu);
    CHECK(read_le32(bytes) == 0xEFBEADDEu);

    // Host order check: copy a uint32 into bytes and compare with the reader
    uint32_t v = 0x11223344u;
    uint8_t mem[4];
    std::memcpy(mem, &v, 4);
    if (is_little_endian()) CHECK(mem[0] == 0x44 && read_le32(mem) == v);
    else                    CHECK(mem[0] == 0x11 && read_be32(mem) == v);
    std::printf("Host is %s-endian\n", is_little_endian() ? "little" : "big");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

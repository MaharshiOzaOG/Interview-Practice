/*
 * 04_hw_data_structures / Q07 : Sparse memory model with byte strobes   Medium
 * ---------------------------------------------------------------------------
 * A 64-bit address space is far too big for an array. Build:
 *     class SparseMem
 *       write32(addr, data, strobe)   // strobe bit i -> write byte lane i (little-endian)
 *       read32(addr)                  // untouched bytes read as `fill` (default 0)
 *       load(addr, bytes)             // backdoor preload (like $readmemh)
 *       pages()                       // number of 4KB pages actually allocated
 * Allocate memory in 4KB pages on first write (unordered_map<page, array>).
 *
 * HW link: the memory slave model behind your AXI DMA testbench; emulators
 *          also back-door load DDR/SRAM images this way.
 */
#include <array>
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class SparseMem {
    static const uint64_t PAGE = 4096;
    std::unordered_map<uint64_t, std::array<uint8_t, PAGE>> pages_;
    uint8_t fill_;
public:
    explicit SparseMem(uint8_t fill = 0) : fill_(fill) {}

    void write8(uint64_t a, uint8_t v) {
        auto it = pages_.find(a / PAGE);
        if (it == pages_.end()) {                       // allocate page on first touch
            std::array<uint8_t, PAGE> blank;
            blank.fill(fill_);
            it = pages_.emplace(a / PAGE, blank).first;
        }
        it->second[a % PAGE] = v;
    }
    uint8_t read8(uint64_t a) const {
        auto it = pages_.find(a / PAGE);
        return it == pages_.end() ? fill_ : it->second[a % PAGE];
    }

    void write32(uint64_t a, uint32_t data, uint8_t strobe = 0xF) {
        for (int i = 0; i < 4; i++)
            if ((strobe >> i) & 1u) write8(a + i, (uint8_t)(data >> (8 * i)));
    }
    uint32_t read32(uint64_t a) const {
        uint32_t v = 0;
        for (int i = 0; i < 4; i++) v |= (uint32_t)read8(a + i) << (8 * i);
        return v;
    }
    void load(uint64_t a, const std::vector<uint8_t>& bytes) {
        for (size_t i = 0; i < bytes.size(); i++) write8(a + i, bytes[i]);
    }
    size_t pages() const { return pages_.size(); }
};

// ======================= TESTS ==========================
int main() {
    SparseMem m;
    CHECK(m.read32(0x1234) == 0 && m.pages() == 0);           // reads don't allocate

    m.write32(0x0, 0xAABBCCDDu, 0b0101);                      // lanes 0 and 2 only
    CHECK(m.read32(0x0) == 0x00BB00DDu);
    m.write32(0x0, 0x11223344u, 0b1010);                      // lanes 1 and 3
    CHECK(m.read32(0x0) == 0x11BB33DDu);

    m.write32(0xFFE, 0xCAFEBABEu);                            // crosses a page boundary
    CHECK(m.read32(0xFFE) == 0xCAFEBABEu);
    CHECK(m.pages() == 2);

    m.write32(0x8000000000ull, 0x5555AAAAu);                  // huge address, 1 more page
    CHECK(m.read32(0x8000000000ull) == 0x5555AAAAu && m.pages() == 3);

    m.load(0x2000, {0xEF, 0xBE, 0xAD, 0xDE});
    CHECK(m.read32(0x2000) == 0xDEADBEEFu);

    SparseMem x(0xFF);                                        // uninit pattern
    CHECK(x.read32(0x40) == 0xFFFFFFFFu);
    x.write32(0x40, 0x0, 0b0001);
    CHECK(x.read32(0x40) == 0xFFFFFF00u);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

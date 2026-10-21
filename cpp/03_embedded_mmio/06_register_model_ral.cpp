/*
 * 03_embedded_mmio / Q06 : Mini register model (UVM-RAL style) in C++     Hard
 * ---------------------------------------------------------------------------
 * Build classes:
 *   Field    { name, lsb, width, access, reset }
 *   Register { name, offset, fields, value }   write(data), read(), reset(),
 *                                              hw_set(field, val), field(name)
 *   RegBlock { map offset -> Register }        write(addr, data) / read(addr)
 *                                              -> false on unmapped address
 * Access policies to support:
 *   RW  : normal read/write
 *   RO  : writes ignored
 *   W1C : writing 1 clears the bit, writing 0 has no effect (status bits)
 *   WO  : write-only, reads return 0
 *
 * HW link: this is what uvm_reg does: a mirror of DUT registers used to predict
 *          values in the scoreboard. In emulation, C tests use a header of the
 *          same register map.
 */
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
enum class Access { RW, RO, W1C, WO };

struct Field {
    std::string name;
    int lsb, width;
    Access access;
    uint32_t reset;
    uint32_t mask() const { return (width == 32 ? 0xFFFFFFFFu : ((1u << width) - 1u)) << lsb; }
};

class Register {
public:
    std::string name;
    uint32_t offset;
    std::vector<Field> fields;
    uint32_t value = 0;

    Register(std::string n, uint32_t off, std::vector<Field> f)
        : name(std::move(n)), offset(off), fields(std::move(f)) { reset(); }

    void reset() {
        value = 0;
        for (auto& f : fields) value |= (f.reset << f.lsb) & f.mask();
    }

    void write(uint32_t data) {                 // software (bus) write
        for (auto& f : fields) {
            uint32_t m = f.mask();
            switch (f.access) {
                case Access::RW:
                case Access::WO:  value = (value & ~m) | (data & m); break;
                case Access::RO:  break;
                case Access::W1C: value &= ~(data & m);              break;
            }
        }
    }

    uint32_t read() const {                     // software (bus) read
        uint32_t r = value;
        for (auto& f : fields) if (f.access == Access::WO) r &= ~f.mask();
        return r;
    }

    void hw_set(const std::string& fname, uint32_t v) {   // hardware updates (e.g. status)
        for (auto& f : fields)
            if (f.name == fname) value = (value & ~f.mask()) | ((v << f.lsb) & f.mask());
    }

    uint32_t field(const std::string& fname) const {      // mirror value of one field
        for (auto& f : fields)
            if (f.name == fname) return (value & f.mask()) >> f.lsb;
        return 0;
    }
};

class RegBlock {
    std::map<uint32_t, Register> regs_;
public:
    void add(const Register& r) { regs_.emplace(r.offset, r); }
    Register* get(uint32_t addr) { auto it = regs_.find(addr); return it == regs_.end() ? nullptr : &it->second; }
    bool write(uint32_t addr, uint32_t data) { auto* r = get(addr); if (!r) return false; r->write(data); return true; }
    bool read(uint32_t addr, uint32_t& data) { auto* r = get(addr); if (!r) return false; data = r->read(); return true; }
    void reset() { for (auto& kv : regs_) kv.second.reset(); }
};

// ======================= TESTS ==========================
int main() {
    RegBlock blk;
    blk.add(Register("CTRL", 0x00, {
        {"en",   0, 1, Access::RW, 0},
        {"mode", 1, 3, Access::RW, 2},
        {"ver",  24, 8, Access::RO, 0x11},
    }));
    blk.add(Register("STATUS", 0x04, {
        {"done", 0, 1, Access::W1C, 0},
        {"err",  1, 1, Access::W1C, 0},
    }));
    blk.add(Register("KEY", 0x08, { {"key", 0, 32, Access::WO, 0} }));

    uint32_t d = 0;
    CHECK(blk.read(0x00, d) && d == 0x11000004u);        // reset: ver=0x11, mode=2
    blk.write(0x00, 0xFFFFFFFFu);
    CHECK(blk.read(0x00, d) && d == 0x1100000Fu);        // RO ver unchanged

    Register* st = blk.get(0x04);
    st->hw_set("done", 1);  st->hw_set("err", 1);
    CHECK(blk.read(0x04, d) && d == 0x3);
    blk.write(0x04, 0x1);                                 // W1C clears only done
    CHECK(blk.read(0x04, d) && d == 0x2);
    blk.write(0x04, 0x0);                                 // writing 0 does nothing
    CHECK(st->field("err") == 1);

    blk.write(0x08, 0xCAFEF00Du);
    CHECK(blk.read(0x08, d) && d == 0);                   // WO reads as 0
    CHECK(blk.get(0x08)->field("key") == 0xCAFEF00Du);    // but mirror holds it

    CHECK(!blk.write(0x40, 1));                           // unmapped
    CHECK(!blk.read(0x40, d));

    blk.reset();
    CHECK(blk.get(0x00)->field("mode") == 2);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

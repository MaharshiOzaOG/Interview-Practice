/*
 * 05_cpp_language / Q03 : Virtual functions, polymorphism, slicing   Difficulty: Easy
 * ---------------------------------------------------------------------------
 * (a) Abstract base `BusTxn` with pure virtual name() and virtual bytes().
 *     Derived AxiTxn and ApbTxn override them. Store in vector<unique_ptr<BusTxn>>.
 * (b) Show WHY the base destructor must be virtual (count destructor calls).
 * (c) Show OBJECT SLICING: passing a derived object BY VALUE to a function taking
 *     the base type calls the base version.
 *
 * Interview Qs:
 *  - How do virtual calls work? Each polymorphic object has a hidden vptr to a
 *    per-class vtable of function pointers; the call is an indirect jump.
 *  - `override` catches signature typos; `final` stops further overriding.
 *  - UVM link: uvm_sequence_item / virtual methods in SV classes work the same way.
 */
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
static int derived_dtors = 0;

class BusTxn {
public:
    virtual ~BusTxn() = default;                    // (b) virtual destructor
    virtual std::string name() const = 0;           // pure virtual -> abstract class
    virtual int bytes() const { return 4; }
};

class AxiTxn : public BusTxn {
    int len_, size_;
public:
    AxiTxn(int len, int size) : len_(len), size_(size) {}
    ~AxiTxn() override { derived_dtors++; }
    std::string name() const override { return "AXI"; }
    int bytes() const override { return len_ * size_; }
};

class ApbTxn final : public BusTxn {                // nobody can derive from ApbTxn
public:
    ~ApbTxn() override { derived_dtors++; }
    std::string name() const override { return "APB"; }
};

// (c) slicing demo with a NON-abstract base
struct Packet           { virtual std::string kind() const { return "Packet"; } virtual ~Packet() = default; };
struct CrcPacket : Packet { std::string kind() const override { return "CrcPacket"; } };

std::string by_value(Packet p)       { return p.kind(); }   // slices!
std::string by_ref(const Packet& p)  { return p.kind(); }   // polymorphic

// ======================= TESTS ==========================
int main() {
    {
        std::vector<std::unique_ptr<BusTxn>> q;
        q.push_back(std::make_unique<AxiTxn>(4, 8));
        q.push_back(std::make_unique<ApbTxn>());
        CHECK(q[0]->name() == "AXI" && q[0]->bytes() == 32);
        CHECK(q[1]->name() == "APB" && q[1]->bytes() == 4);   // inherited default
        int total = 0;
        for (auto& t : q) total += t->bytes();                 // dynamic dispatch
        CHECK(total == 36);
    }
    CHECK(derived_dtors == 2);          // derived dtors ran via base pointer

    CrcPacket c;
    CHECK(by_ref(c) == "CrcPacket");
    CHECK(by_value(c) == "Packet");     // the CrcPacket part was sliced off

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

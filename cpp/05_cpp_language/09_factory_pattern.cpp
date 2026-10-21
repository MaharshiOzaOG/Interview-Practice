/*
 * 05_cpp_language / Q09 : Factory with type overrides (UVM-factory style)   Medium
 * ---------------------------------------------------------------------------
 * Build a Factory that:
 *   - registers creators by type name:  reg("base_seq", []{ return make_unique<BaseSeq>(); })
 *   - creates by name:                  create("base_seq") -> unique_ptr<Sequence>
 *   - supports overrides:               set_override("base_seq", "err_seq")
 *     after which create("base_seq") returns an ErrSeq  (follows override chains)
 * Use a static registration helper so each class registers itself before main().
 *
 * Why interviewers ask: tests swap drivers/sequences without editing the env -
 * exactly how uvm_factory::set_type_override_by_type works.
 */
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
struct Sequence {
    virtual ~Sequence() = default;
    virtual std::string body() const = 0;
};

class Factory {
    std::map<std::string, std::function<std::unique_ptr<Sequence>()>> creators_;
    std::map<std::string, std::string> overrides_;
public:
    static Factory& get() { static Factory f; return f; }

    void reg(const std::string& name, std::function<std::unique_ptr<Sequence>()> fn) {
        creators_[name] = std::move(fn);
    }
    void set_override(const std::string& orig, const std::string& repl) { overrides_[orig] = repl; }
    void clear_overrides() { overrides_.clear(); }

    std::unique_ptr<Sequence> create(std::string name) const {
        for (int hops = 0; hops < 16; hops++) {            // follow chain, guard loops
            auto o = overrides_.find(name);
            if (o == overrides_.end()) break;
            name = o->second;
        }
        auto it = creators_.find(name);
        return it == creators_.end() ? nullptr : it->second();
    }
};

template <typename T>
struct Registrar {                                           // runs at static-init time
    explicit Registrar(const std::string& n) { Factory::get().reg(n, [] { return std::make_unique<T>(); }); }
};

struct BaseSeq  : Sequence { std::string body() const override { return "normal traffic"; } };
struct ErrSeq   : BaseSeq  { std::string body() const override { return "inject SLVERR"; } };
struct BurstSeq : BaseSeq  { std::string body() const override { return "max-length bursts"; } };

static Registrar<BaseSeq>  r1("base_seq");
static Registrar<ErrSeq>   r2("err_seq");
static Registrar<BurstSeq> r3("burst_seq");

// ======================= TESTS ==========================
int main() {
    Factory& f = Factory::get();
    CHECK(f.create("base_seq")->body() == "normal traffic");
    CHECK(f.create("nope") == nullptr);

    f.set_override("base_seq", "err_seq");                   // test-level override
    CHECK(f.create("base_seq")->body() == "inject SLVERR");

    f.set_override("err_seq", "burst_seq");                  // chain: base->err->burst
    CHECK(f.create("base_seq")->body() == "max-length bursts");

    f.clear_overrides();
    CHECK(f.create("base_seq")->body() == "normal traffic");

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

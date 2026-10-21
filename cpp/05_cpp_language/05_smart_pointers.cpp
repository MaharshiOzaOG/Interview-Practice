/*
 * 05_cpp_language / Q05 : Smart pointers & RAII                  Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) unique_ptr : sole ownership, move-only. Transfer it into a function.
 * (b) shared_ptr : shared ownership, check use_count().
 * (c) weak_ptr   : break a reference CYCLE. A Component holds shared_ptr to its
 *                  children; the child's parent link must be weak_ptr or neither
 *                  is ever freed (UVM-like component tree).
 * (d) Custom deleter: wrap a C resource (FILE*-like handle) so it closes itself.
 * (e) RAII: resource released in the destructor, even on early return/exception.
 */
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

static int alive = 0;

// ======================= SOLUTION =======================
struct Txn {
    int id;
    explicit Txn(int i) : id(i) { alive++; }
    ~Txn() { alive--; }
};

int consume(std::unique_ptr<Txn> t) { return t->id; }   // takes ownership; freed on return

struct Component {
    std::string name;
    std::weak_ptr<Component> parent;                     // weak: does NOT own
    std::vector<std::shared_ptr<Component>> children;    // strong: owns
    explicit Component(std::string n) : name(std::move(n)) { alive++; }
    ~Component() { alive--; }
    std::string parent_name() const { auto p = parent.lock(); return p ? p->name : "<none>"; }
};

std::shared_ptr<Component> add_child(const std::shared_ptr<Component>& p, const std::string& n) {
    auto c = std::make_shared<Component>(n);
    c->parent = p;
    p->children.push_back(c);
    return c;
}

// C-style handle + custom deleter
struct Handle { int fd; };
static int open_handles = 0;
Handle* h_open()           { open_handles++; return new Handle{3}; }
void    h_close(Handle* h) { open_handles--; delete h; }
using HandlePtr = std::unique_ptr<Handle, decltype(&h_close)>;

bool use_handle(bool fail_early) {
    HandlePtr h(h_open(), &h_close);     // RAII: closes on every exit path
    if (fail_early) return false;
    return h->fd == 3;
}

// ======================= TESTS ==========================
int main() {
    auto u = std::make_unique<Txn>(42);
    CHECK(alive == 1);
    CHECK(consume(std::move(u)) == 42);
    CHECK(u == nullptr && alive == 0);           // ownership moved and freed

    {
        auto s1 = std::make_shared<Txn>(1);
        auto s2 = s1;
        CHECK(s1.use_count() == 2);
        s2.reset();
        CHECK(s1.use_count() == 1 && alive == 1);
    }
    CHECK(alive == 0);

    {
        auto env = std::make_shared<Component>("env");
        auto agt = add_child(env, "agent");
        add_child(agt, "driver");
        CHECK(agt->parent_name() == "env");
        CHECK(env->parent_name() == "<none>");
        CHECK(alive == 3);
    }
    CHECK(alive == 0);                           // no cycle leak thanks to weak_ptr

    CHECK(use_handle(false));
    CHECK(!use_handle(true));
    CHECK(open_handles == 0);                    // closed on both paths

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

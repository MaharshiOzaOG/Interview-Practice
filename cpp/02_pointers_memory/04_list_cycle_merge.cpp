/*
 * 02_pointers_memory / Q04 : Cycle detection & merge sorted lists   Difficulty: Medium
 * ---------------------------------------------------------------------------
 * (a) has_cycle(head)     : Floyd's tortoise and hare
 * (b) cycle_start(head)   : node where the cycle begins (nullptr if none)
 *     Why it works: after meeting, reset one pointer to head; move both by 1;
 *     they meet at the start (distance head->start == distance meet->start mod L).
 * (c) cycle_length(head)
 * (d) merge_sorted(a, b)  : merge two sorted lists in place using a dummy node
 *
 * HW link: same idea as detecting a loop in a descriptor chain (DMA linked list)
 *          that never reaches the "last" flag -> hang in emulation.
 */
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

struct Node { int val; Node* next; };

std::vector<Node*> make_nodes(int n) {      // separate allocation so we can free easily
    std::vector<Node*> v;
    for (int i = 0; i < n; i++) v.push_back(new Node{i, nullptr});
    for (int i = 0; i + 1 < n; i++) v[i]->next = v[i + 1];
    return v;
}

// ======================= SOLUTION =======================
bool has_cycle(Node* head) {
    Node *s = head, *f = head;
    while (f && f->next) {
        s = s->next; f = f->next->next;
        if (s == f) return true;
    }
    return false;
}

Node* cycle_start(Node* head) {
    Node *s = head, *f = head;
    while (f && f->next) {
        s = s->next; f = f->next->next;
        if (s == f) {
            s = head;
            while (s != f) { s = s->next; f = f->next; }
            return s;
        }
    }
    return nullptr;
}

int cycle_length(Node* head) {
    Node* start = cycle_start(head);
    if (!start) return 0;
    int len = 1;
    for (Node* p = start->next; p != start; p = p->next) len++;
    return len;
}

Node* merge_sorted(Node* a, Node* b) {
    Node dummy{0, nullptr};
    Node* tail = &dummy;
    while (a && b) {
        if (a->val <= b->val) { tail->next = a; a = a->next; }
        else                  { tail->next = b; b = b->next; }
        tail = tail->next;
    }
    tail->next = a ? a : b;
    return dummy.next;
}

// ======================= TESTS ==========================
int main() {
    auto v = make_nodes(6);                   // 0->1->2->3->4->5
    CHECK(!has_cycle(v[0]));
    CHECK(cycle_start(v[0]) == nullptr);
    v[5]->next = v[2];                        // 5 -> 2 makes a cycle of length 4
    CHECK(has_cycle(v[0]));
    CHECK(cycle_start(v[0]) == v[2]);
    CHECK(cycle_length(v[0]) == 4);
    for (Node* n : v) delete n;

    // merge
    Node* a = new Node{1, new Node{4, new Node{7, nullptr}}};
    Node* b = new Node{2, new Node{3, new Node{8, new Node{9, nullptr}}}};
    Node* m = merge_sorted(a, b);
    std::vector<int> got;
    for (Node* p = m; p; p = p->next) got.push_back(p->val);
    CHECK(got == std::vector<int>({1, 2, 3, 4, 7, 8, 9}));
    while (m) { Node* n = m->next; delete m; m = n; }

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

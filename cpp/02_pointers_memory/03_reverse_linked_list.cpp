/*
 * 02_pointers_memory / Q03 : Linked list basics                  Difficulty: Easy
 * ---------------------------------------------------------------------------
 * Using   struct Node { int val; Node* next; };
 * (a) reverse_iter(head)   : iterative reversal, O(1) extra space
 * (b) reverse_rec(head)    : recursive reversal
 * (c) middle(head)         : slow/fast pointer, returns 2nd middle for even length
 * (d) remove_value(head, v): delete all nodes with value v (pointer-to-pointer trick)
 *
 * Tip: draw 3 boxes and 3 pointers (prev, cur, next) - interviewers love seeing it.
 */
#include <cstdio>
#include <vector>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

struct Node { int val; Node* next; };

// helpers
Node* build(const std::vector<int>& v) {
    Node* head = nullptr;
    for (auto it = v.rbegin(); it != v.rend(); ++it) head = new Node{*it, head};
    return head;
}
std::vector<int> to_vec(Node* h) { std::vector<int> v; for (; h; h = h->next) v.push_back(h->val); return v; }
void free_list(Node* h) { while (h) { Node* n = h->next; delete h; h = n; } }

// ======================= SOLUTION =======================
Node* reverse_iter(Node* head) {
    Node* prev = nullptr;
    while (head) {
        Node* next = head->next;  // save
        head->next = prev;        // flip
        prev = head;              // advance
        head = next;
    }
    return prev;
}

Node* reverse_rec(Node* head) {
    if (!head || !head->next) return head;
    Node* new_head = reverse_rec(head->next);
    head->next->next = head;      // the node after me now points back to me
    head->next = nullptr;
    return new_head;
}

Node* middle(Node* head) {
    Node* slow = head; Node* fast = head;
    while (fast && fast->next) { slow = slow->next; fast = fast->next->next; }
    return slow;
}

Node* remove_value(Node* head, int v) {
    Node** pp = &head;            // points at the link we may rewrite
    while (*pp) {
        if ((*pp)->val == v) { Node* dead = *pp; *pp = dead->next; delete dead; }
        else pp = &(*pp)->next;
    }
    return head;
}

// ======================= TESTS ==========================
int main() {
    Node* h = build({1, 2, 3, 4, 5});
    h = reverse_iter(h);  CHECK(to_vec(h) == std::vector<int>({5, 4, 3, 2, 1}));
    h = reverse_rec(h);   CHECK(to_vec(h) == std::vector<int>({1, 2, 3, 4, 5}));
    CHECK(middle(h)->val == 3);
    free_list(h);

    h = build({1, 2, 3, 4});
    CHECK(middle(h)->val == 3);
    free_list(h);

    CHECK(reverse_iter(nullptr) == nullptr);

    h = build({7, 1, 7, 7, 2, 7});
    h = remove_value(h, 7);
    CHECK(to_vec(h) == std::vector<int>({1, 2}));
    free_list(h);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

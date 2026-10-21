/*
 * 01_bit_manipulation / Q09 : XOR tricks                         Difficulty: Easy/Medium
 * ---------------------------------------------------------------------------
 * (a) single_number : every value appears twice except one. Find it. O(n), O(1) space.
 * (b) missing_number: array holds n distinct values from 0..n, one missing.
 * (c) two_singles   : every value appears twice except TWO. Find both. (Medium)
 * (d) xor_swap      : swap two ints without a temp (and why a == b alias breaks it)
 *
 * Key facts: a ^ a = 0, a ^ 0 = a, XOR is commutative/associative.
 */
#include <cstdint>
#include <cstdio>
#include <vector>
#include <utility>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
int single_number(const std::vector<int>& v) {
    int x = 0;
    for (int a : v) x ^= a;       // pairs cancel
    return x;
}

int missing_number(const std::vector<int>& v) {
    int x = (int)v.size();        // include n itself
    for (int i = 0; i < (int)v.size(); i++) x ^= i ^ v[i];
    return x;
}

std::pair<int, int> two_singles(const std::vector<int>& v) {
    uint32_t all = 0;
    for (int a : v) all ^= (uint32_t)a;           // = p ^ q  (non-zero since p != q)
    uint32_t diff = all & (~all + 1u);            // a bit where p and q differ
    int p = 0, q = 0;
    for (int a : v) ((uint32_t)a & diff) ? (p ^= a) : (q ^= a);   // split into two groups
    return p < q ? std::make_pair(p, q) : std::make_pair(q, p);
}

void xor_swap(int* a, int* b) {
    if (a == b) return;           // same address: a^=a would zero it!
    *a ^= *b;  *b ^= *a;  *a ^= *b;
}

// ======================= TESTS ==========================
int main() {
    CHECK(single_number({4, 1, 2, 1, 2}) == 4);
    CHECK(single_number({7}) == 7);
    CHECK(missing_number({3, 0, 1}) == 2);
    CHECK(missing_number({0, 1}) == 2);
    CHECK(missing_number({1}) == 0);
    CHECK(two_singles({1, 2, 1, 3, 2, 5}) == std::make_pair(3, 5));
    CHECK(two_singles({-1, 0}) == std::make_pair(-1, 0));

    int a = 5, b = 9;
    xor_swap(&a, &b);  CHECK(a == 9 && b == 5);
    xor_swap(&a, &a);  CHECK(a == 9);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

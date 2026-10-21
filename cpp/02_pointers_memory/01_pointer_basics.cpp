/*
 * 02_pointers_memory / Q01 : Pointer basics                       Difficulty: Easy
 * ---------------------------------------------------------------------------
 * (a) swap_ptr(int*, int*) and swap_ref(int&, int&)
 * (b) sum(const int* begin, const int* end)   -- pointer-range iteration
 * (c) find_max(int* a, int n) -> pointer to the max element (nullptr if n == 0)
 * (d) reverse_array(int* a, int n) with two pointers
 * (e) Know the const puzzles:
 *        const int* p   -> pointer to const int   (can't do *p = 1, can do p++)
 *        int* const p   -> const pointer to int   (can do *p = 1, can't do p++)
 *        const int* const p -> neither
 *     Read right-to-left: "p is a const pointer to int".
 */
#include <cstdio>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
void swap_ptr(int* a, int* b) { int t = *a; *a = *b; *b = t; }
void swap_ref(int& a, int& b) { int t = a;  a = b;  b = t; }

int sum(const int* begin, const int* end) {
    int s = 0;
    for (const int* p = begin; p != end; ++p) s += *p;
    return s;
}

int* find_max(int* a, int n) {
    if (n <= 0) return nullptr;
    int* best = a;
    for (int* p = a + 1; p < a + n; ++p)
        if (*p > *best) best = p;
    return best;
}

void reverse_array(int* a, int n) {
    int* l = a;
    int* r = a + n - 1;
    while (l < r) swap_ptr(l++, r--);
}

// ======================= TESTS ==========================
int main() {
    int x = 1, y = 2;
    swap_ptr(&x, &y);  CHECK(x == 2 && y == 1);
    swap_ref(x, y);    CHECK(x == 1 && y == 2);

    int arr[] = {3, 9, -2, 9, 4};
    CHECK(sum(arr, arr + 5) == 23);
    CHECK(sum(arr, arr) == 0);

    int* m = find_max(arr, 5);
    CHECK(m == &arr[1]);              // first max wins
    CHECK(m - arr == 1);              // pointer difference = index
    CHECK(find_max(arr, 0) == nullptr);

    reverse_array(arr, 5);
    int exp[] = {4, 9, -2, 9, 3};
    for (int i = 0; i < 5; i++) CHECK(arr[i] == exp[i]);

    // pointer arithmetic scales by element size
    int* p = arr;
    CHECK((char*)(p + 1) - (char*)p == (long)sizeof(int));

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

/*
 * 02_pointers_memory / Q02 : Implement memcpy / memmove / memset / memcmp   Medium
 * ---------------------------------------------------------------------------
 * Write your own versions using only byte pointers.
 *   my_memcpy  : regions must NOT overlap
 *   my_memmove : regions MAY overlap -> copy backwards if dst is after src
 *   my_memset, my_memcmp
 *
 * Interview Q: "What goes wrong if you use memcpy on overlapping buffers?"
 *   -> forward copy overwrites source bytes before they are read.
 * HW link    : DMA engines have the same overlap problem (your AXI DMA!).
 */
#include <cstddef>
#include <cstdio>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
void* my_memcpy(void* dst, const void* src, size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dst);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    while (n--) *d++ = *s++;
    return dst;
}

void* my_memmove(void* dst, const void* src, size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dst);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    if (d == s || n == 0) return dst;
    if (d < s) {                       // dst before src: forward copy is safe
        while (n--) *d++ = *s++;
    } else {                           // dst after src: copy from the end
        d += n;  s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

void* my_memset(void* dst, int value, size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dst);
    while (n--) *d++ = (unsigned char)value;
    return dst;
}

int my_memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* p = static_cast<const unsigned char*>(a);
    const unsigned char* q = static_cast<const unsigned char*>(b);
    for (; n; n--, p++, q++)
        if (*p != *q) return *p < *q ? -1 : 1;
    return 0;
}

// ======================= TESTS ==========================
int main() {
    char a[16] = "hello world";
    char b[16] = {0};
    my_memcpy(b, a, 12);
    CHECK(std::strcmp(b, "hello world") == 0);

    char c[16] = "abcdefgh";
    my_memmove(c + 2, c, 6);           // overlap, dst after src
    CHECK(std::memcmp(c, "ababcdef", 8) == 0);

    char d[16] = "abcdefgh";
    my_memmove(d, d + 2, 6);           // overlap, dst before src
    CHECK(std::memcmp(d, "cdefghgh", 8) == 0);

    unsigned char e[8];
    my_memset(e, 0xA5, 8);
    for (int i = 0; i < 8; i++) CHECK(e[i] == 0xA5);

    CHECK(my_memcmp("abc", "abc", 3) == 0);
    CHECK(my_memcmp("abc", "abd", 3) < 0);
    CHECK(my_memcmp("\xFF", "\x01", 1) > 0);   // compares as UNSIGNED bytes

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

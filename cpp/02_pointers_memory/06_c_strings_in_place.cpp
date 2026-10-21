/*
 * 02_pointers_memory / Q06 : C-string handling in place          Difficulty: Medium
 * ---------------------------------------------------------------------------
 * No std::string allowed - char* only.
 * (a) my_strlen, my_strcpy
 * (b) reverse_str(char* s)                       : in place
 * (c) reverse_words(char* s)  "the sky is blue" -> "blue is sky the"
 *        trick: reverse whole string, then reverse each word
 * (d) is_palindrome(const char* s)               : ignore non-alphanumerics and case
 * (e) my_atoi(const char* s)                      : skip spaces, sign, digits, clamp to int
 */
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
size_t my_strlen(const char* s) { const char* p = s; while (*p) p++; return (size_t)(p - s); }

char* my_strcpy(char* dst, const char* src) { char* d = dst; while ((*d++ = *src++)) {} return dst; }

static void reverse_range(char* l, char* r) {      // inclusive range
    while (l < r) { char t = *l; *l++ = *r; *r-- = t; }
}

void reverse_str(char* s) {
    size_t n = my_strlen(s);
    if (n) reverse_range(s, s + n - 1);
}

void reverse_words(char* s) {
    reverse_str(s);
    char* p = s;
    while (*p) {
        while (*p == ' ') p++;                     // skip spaces
        char* start = p;
        while (*p && *p != ' ') p++;               // find word end
        if (p > start) reverse_range(start, p - 1);
    }
}

bool is_palindrome(const char* s) {
    const char* l = s;
    const char* r = s + my_strlen(s);              // one past end
    while (l < r) {
        if (!std::isalnum((unsigned char)*l)) { l++; continue; }
        if (!std::isalnum((unsigned char)r[-1])) { r--; continue; }
        if (std::tolower((unsigned char)*l) != std::tolower((unsigned char)r[-1])) return false;
        l++; r--;
    }
    return true;
}

int my_atoi(const char* s) {
    while (*s == ' ') s++;
    int sign = 1;
    if (*s == '+' || *s == '-') { if (*s == '-') sign = -1; s++; }
    long long v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        if (v * sign > INT_MAX) return INT_MAX;    // clamp before overflow grows
        if (v * sign < INT_MIN) return INT_MIN;
        s++;
    }
    return (int)(v * sign);
}

// ======================= TESTS ==========================
int main() {
    CHECK(my_strlen("") == 0 && my_strlen("abc") == 3);
    char buf[32];
    my_strcpy(buf, "copy me");
    CHECK(std::strcmp(buf, "copy me") == 0);

    char s1[] = "hello";       reverse_str(s1);    CHECK(std::strcmp(s1, "olleh") == 0);
    char s2[] = "";            reverse_str(s2);    CHECK(s2[0] == '\0');
    char s3[] = "the sky is blue"; reverse_words(s3); CHECK(std::strcmp(s3, "blue is sky the") == 0);

    CHECK(is_palindrome("A man, a plan, a canal: Panama"));
    CHECK(!is_palindrome("race a car"));
    CHECK(is_palindrome(""));

    CHECK(my_atoi("42") == 42);
    CHECK(my_atoi("   -17abc") == -17);
    CHECK(my_atoi("99999999999") == INT_MAX);
    CHECK(my_atoi("-99999999999") == INT_MIN);
    CHECK(my_atoi("abc") == 0);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

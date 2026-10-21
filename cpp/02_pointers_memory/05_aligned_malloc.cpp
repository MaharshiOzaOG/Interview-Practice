/*
 * 02_pointers_memory / Q05 : aligned_malloc / aligned_free          Difficulty: Hard
 * ---------------------------------------------------------------------------
 * Using only malloc/free, write:
 *     void* aligned_malloc(size_t size, size_t align);   // align = power of two
 *     void  aligned_free(void* p);
 * The returned pointer must be a multiple of `align`, and aligned_free must be
 * able to find the ORIGINAL malloc pointer.
 *
 * Idea: over-allocate (size + align - 1 + sizeof(void*)), round the address up,
 *       store the raw pointer just BEFORE the aligned block.
 *
 *   raw                     aligned
 *   |  padding  | raw ptr |  user data (size bytes) ... |
 *
 * HW link: DMA buffers and cache-line-aligned descriptors must be aligned.
 * Classic Qualcomm/NVIDIA embedded question.
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
void* aligned_malloc(size_t size, size_t align) {
    if (align == 0 || (align & (align - 1))) return nullptr;      // must be power of 2
    size_t extra = align - 1 + sizeof(void*);
    void* raw = std::malloc(size + extra);
    if (!raw) return nullptr;
    uintptr_t start   = (uintptr_t)raw + sizeof(void*);           // leave room for raw ptr
    uintptr_t aligned = (start + align - 1) & ~(uintptr_t)(align - 1);
    ((void**)aligned)[-1] = raw;                                   // stash raw pointer
    return (void*)aligned;
}

void aligned_free(void* p) {
    if (p) std::free(((void**)p)[-1]);
}

// ======================= TESTS ==========================
int main() {
    size_t aligns[] = {1, 2, 8, 16, 64, 256, 4096};
    size_t sizes[]  = {1, 7, 100, 5000};
    for (size_t a : aligns) {
        for (size_t sz : sizes) {
            unsigned char* p = (unsigned char*)aligned_malloc(sz, a);
            CHECK(p != nullptr);
            CHECK(((uintptr_t)p & (a - 1)) == 0);
            std::memset(p, 0x5A, sz);                 // whole block writable
            CHECK(p[0] == 0x5A && p[sz - 1] == 0x5A);
            aligned_free(p);
        }
    }
    CHECK(aligned_malloc(16, 3) == nullptr);           // not a power of two
    aligned_free(nullptr);                             // must be safe

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

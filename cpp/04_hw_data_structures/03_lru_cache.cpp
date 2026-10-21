/*
 * 04_hw_data_structures / Q03 : LRU cache (LeetCode 146)          Difficulty: Medium
 * ---------------------------------------------------------------------------
 * class LRUCache(capacity): get(key) -> value or -1, put(key, value). Both O(1).
 *
 * Design: std::list keeps recency order (front = most recent),
 *         unordered_map<key, list iterator> gives O(1) lookup.
 *         list::splice moves a node to the front without reallocating.
 *
 * HW link: the replacement policy in your cache controller. Real caches use
 *          pseudo-LRU (tree PLRU) because true LRU state grows as ways!.
 */
#include <cstdio>
#include <list>
#include <unordered_map>
#include <utility>

static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)

// ======================= SOLUTION =======================
class LRUCache {
    int cap_;
    std::list<std::pair<int, int>> order_;                                  // {key, value}
    std::unordered_map<int, std::list<std::pair<int, int>>::iterator> pos_;
public:
    explicit LRUCache(int capacity) : cap_(capacity) {}

    int get(int key) {
        auto it = pos_.find(key);
        if (it == pos_.end()) return -1;
        order_.splice(order_.begin(), order_, it->second);   // mark most recent
        return it->second->second;
    }

    void put(int key, int value) {
        auto it = pos_.find(key);
        if (it != pos_.end()) {
            it->second->second = value;
            order_.splice(order_.begin(), order_, it->second);
            return;
        }
        if ((int)order_.size() == cap_) {                     // evict LRU (back)
            pos_.erase(order_.back().first);
            order_.pop_back();
        }
        order_.emplace_front(key, value);
        pos_[key] = order_.begin();
    }
};

// ======================= TESTS ==========================
int main() {
    LRUCache c(2);
    c.put(1, 1);
    c.put(2, 2);
    CHECK(c.get(1) == 1);        // order: 1, 2
    c.put(3, 3);                 // evicts 2
    CHECK(c.get(2) == -1);
    c.put(4, 4);                 // evicts 1
    CHECK(c.get(1) == -1);
    CHECK(c.get(3) == 3);
    CHECK(c.get(4) == 4);

    LRUCache d(2);
    d.put(1, 10);  d.put(2, 20);
    d.put(1, 11);                // update makes 1 most recent
    d.put(3, 30);                // evicts 2
    CHECK(d.get(1) == 11 && d.get(2) == -1 && d.get(3) == 30);

    if (fails) std::printf("%d check(s) FAILED\n", fails);
    else       std::printf("All tests passed\n");
    return fails != 0;
}

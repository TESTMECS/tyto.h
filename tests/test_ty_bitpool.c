#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ty_bitpool.h>

// define our pool
BIT_POOL_DEFINE(pool, TY_POOL_SIZE);

// define out backing storage
static unsigned char memory[TY_POOL_SIZE][TY_BLOCK_SIZE];
static pool          mypool = {0};

void*
pool_alloc(void)
{
    int  index;
    bool ok;
    BIT_POOL_ALLOC(TY_POOL_SIZE, &mypool, index, ok);
    if (!ok)
        return NULL;
    return memory[index];
}

void
pool_free(void* ptr)
{
    size_t index = ((unsigned char (*)[TY_BLOCK_SIZE])ptr - memory);
    BIT_POOL_SET_0(TY_POOL_SIZE, &mypool, index);
}

// ---- Example 1: Batch allocation ----
// Demonstrates the speed advantage - we can find all free slots
// in O(number of free slots) instead of O(total pool size)
int
pool_alloc_batch(int count, void** out_ptrs)
{
    int allocated = 0;
    for (int i = 0; i < count; i++) {
        void* ptr = pool_alloc();
        if (!ptr)
            break;
        out_ptrs[allocated++] = ptr;
    }
    return allocated;
}

// ---- Example 2: Fast pool statistics ----
// No need to iterate entire pool - just count the set bits!
// Uses popcount which is also a single CPU instruction
static inline int
popcount64(uint64_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#else
    int n = 0;
    while (x) {
        n += x & 1;
        x >>= 1;
    }
    return n;
#endif
}

void
pool_stats(int* out_free, int* out_used)
{
    int used_count = 0;

    for (size_t i = 0; i < BIT_POOL_L0_COUNT(TY_POOL_SIZE); i++) {
        uint64_t l0      = mypool.l0[i];
        int      l0_used = popcount64(l0);

        if (i == BIT_POOL_L0_COUNT(TY_POOL_SIZE) - 1 &&
            TY_POOL_SIZE % 64 != 0) {
            int last_bucket_size = TY_POOL_SIZE % 64;
            l0_used = popcount64(l0 & ((1ULL << last_bucket_size) - 1));
        }
        used_count += l0_used;
    }

    *out_used = used_count;
    *out_free = TY_POOL_SIZE - used_count;
}

// ---- Example 3: Find first contiguous N free slots ----
// Useful for allocating variable-sized blocks
// The bitpool makes this efficient - we can scan L1 for partially-free buckets
int
pool_find_contiguous(int count)
{
    if (count > TY_POOL_SIZE)
        return -1;

    int streak = 0;
    int start  = -1;

    for (int i = 0; i < TY_POOL_SIZE; i++) {
        bool used = BIT_POOL_CHECK_1(TY_POOL_SIZE, &mypool, i);
        if (!used) {
            if (streak == 0)
                start = i;
            streak++;
            if (streak >= count)
                return start;
        } else {
            streak = 0;
            start  = -1;
        }
    }
    return -1;
}

// ---- Example 4: Try to allocate at specific index ----
// Useful for alignment requirements or realloc-like behavior
void*
pool_alloc_at(size_t index)
{
    if (index >= TY_POOL_SIZE)
        return NULL;
    if (BIT_POOL_CHECK_1(TY_POOL_SIZE, &mypool, index))
        return NULL;
    BIT_POOL_SET_1(TY_POOL_SIZE, &mypool, index);
    return memory[index];
}

// ---- Example 5: Check if specific range is all free ----
// O(1) for entire range using bit operations!
bool
pool_is_range_free(size_t start, size_t count)
{
    if (start + count > TY_POOL_SIZE)
        return false;

    uint64_t start_bucket = start / 64;
    uint64_t end_bucket   = (start + count - 1) / 64;

    if (start_bucket == end_bucket) {
        uint64_t mask = ((1ULL << count) - 1) << (start % 64);
        return (mypool.l0[start_bucket] & mask) == 0;
    }

    // First partial bucket
    uint64_t first_mask = ~0ULL << (start % 64);
    if ((mypool.l0[start_bucket] & first_mask) != 0)
        return false;

    // Middle full buckets
    for (uint64_t b = start_bucket + 1; b < end_bucket; b++) {
        if (mypool.l0[b] != 0)
            return false;
    }

    // Last partial bucket
    uint64_t last_bits = (start + count) % 64;
    uint64_t last_mask = last_bits == 0 ? ~0ULL : ((1ULL << last_bits) - 1);
    if ((mypool.l0[end_bucket] & last_mask) != 0)
        return false;

    return true;
}

// ---- Example 6: Defragmentation candidate detection ----
// Find the largest contiguous free block - useful for deciding
// when to compact or if allocation will fail
int
pool_largest_contiguous(void)
{
    int max_streak = 0;
    int streak     = 0;

    for (int i = 0; i < TY_POOL_SIZE; i++) {
        bool used = BIT_POOL_CHECK_1(TY_POOL_SIZE, &mypool, i);
        if (!used) {
            streak++;
            if (streak > max_streak)
                max_streak = streak;
        } else {
            streak = 0;
        }
    }
    return max_streak;
}

void
print_separator(const char* title)
{
    printf("\n=== %s ===\n", title);
}

int
main(int argc, char** argv)
{
    void* a = pool_alloc();
    void* b = pool_alloc();

    printf("a = %p, b = %p\n", a, b);

    strcpy(a, "hello");
    printf("a = %s\n", (char*)a);

    printf("a = %p, b = %p\n", a, b);

    pool_free(a);
    pool_free(b);

    // ---- Example 1: Batch allocation demo ----
    print_separator("Batch Allocation");
    void* batch[10];
    int   batch_count = pool_alloc_batch(10, batch);
    printf("Allocated %d slots in batch\n", batch_count);
    for (int i = 0; i < batch_count; i++) {
        printf("  batch[%d] = %p\n", i, batch[i]);
    }
    // for (int i = 0; i < batch_count; i++) {
    //     pool_free(batch[i]);
    // }

    // ---- Example 2: Statistics demo ----
    print_separator("Pool Statistics");
    int free_slots, used_slots;
    pool_stats(&free_slots, &used_slots);
    printf("Pool size: %d\n", TY_POOL_SIZE);
    printf("Used slots: %d\n", used_slots);
    printf("Free slots: %d\n", free_slots);

    for (int i = 0; i < batch_count; i++) {
        pool_free(batch[i]);
    }

    // ---- Example 3: Contiguous allocation demo ----
    print_separator("Find Contiguous");
    pool_alloc_batch(5, batch);  // Allocate 5 to create holes
    pool_alloc();                // More fragmentation
    pool_alloc();
    int contiguous = pool_find_contiguous(3);
    printf("Largest contiguous free region: %d slots\n", contiguous);

    // ---- Example 4: Allocate at specific index ----
    print_separator("Allocate at Index");
    void* at_100       = pool_alloc_at(100);
    void* at_100_again = pool_alloc_at(100);  // Should fail
    printf("Allocated at index 100: %p\n", at_100);
    printf("Allocated at index 100 again (should be NULL): %p\n", at_100_again);
    pool_free(at_100);

    // ---- Example 5: Range check demo ----
    print_separator("Range Check");
    pool_alloc_at(50);
    pool_alloc_at(51);
    bool range_free    = pool_is_range_free(0, 50);
    bool range_free_50 = pool_is_range_free(50, 5);
    printf("Range [0, 50) is free: %s\n", range_free ? "yes" : "no");
    printf("Range [50, 55) is free: %s\n", range_free_50 ? "yes" : "no");

    // ---- Example 6: Largest contiguous demo ----
    print_separator("Largest Contiguous Free");
    int largest = pool_largest_contiguous();
    printf("Largest contiguous free block: %d slots\n", largest);

    return 0;
}

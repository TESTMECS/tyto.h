#ifndef TY_BITPOOL_H_
#define TY_BITPOOL_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef TY_POOL_SIZE
#define TY_POOL_SIZE 1024
#endif  // TY_POOL_SIZE

#ifndef TY_BLOCK_SIZE
#define TY_BLOCK_SIZE 32
#endif  // TY_BLOCK_SIZE

#define BIT_POOL_L0_COUNT(N) ((N) / 64)
#define BIT_POOL_L1_COUNT(N) (((N) + 4095) / 4096)

#define BIT_POOL_DEFINE(name, N)                                               \
    typedef struct                                                             \
    {                                                                          \
        uint64_t l1[BIT_POOL_L1_COUNT(N)];                                     \
        uint64_t l0[BIT_POOL_L0_COUNT(N)];                                     \
    } name

// === helpers ===

static inline int
ctz64(uint64_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#else
    // fallback (slow)
    int n = 0;
    while ((x & 1) == 0) {
        x >>= 1;
        n++;
    }
    return n;
#endif
}

// === API ===

#define BIT_POOL_FIND_0(N, bp, out_index, out_ok)                              \
    do {                                                                       \
        int l0_index = -1;                                                     \
                                                                               \
        if ((N) > 64) {                                                        \
            for (size_t i = 0; i < BIT_POOL_L1_COUNT(N); ++i) {                \
                uint64_t used = (bp)->l1[i];                                   \
                uint64_t inv  = ~used;                                         \
                int      slot = ctz64(inv);                                    \
                if (slot != 64) {                                              \
                    l0_index = (int)(64 * i + slot);                           \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        } else {                                                               \
            l0_index = 0;                                                      \
        }                                                                      \
                                                                               \
        if (l0_index == -1 || l0_index >= (int)BIT_POOL_L0_COUNT(N)) {         \
            (out_ok)    = false;                                               \
            (out_index) = -1;                                                  \
            break;                                                             \
        }                                                                      \
                                                                               \
        uint64_t inv  = ~((bp)->l0[l0_index]);                                 \
        int      slot = ctz64(inv);                                            \
        if (slot != 64) {                                                      \
            (out_index) = l0_index * 64 + slot;                                \
            (out_ok)    = true;                                                \
        } else {                                                               \
            (out_index) = -1;                                                  \
            (out_ok)    = false;                                               \
        }                                                                      \
    } while (0)

#define BIT_POOL_SET_1(N, bp, index)                                           \
    do {                                                                       \
        assert((index) < (uint64_t)(N));                                       \
                                                                               \
        uint64_t l0_index = (index) / 64;                                      \
        uint64_t l0_slot  = (index) % 64;                                      \
        uint64_t l1_index = l0_index / 64;                                     \
        uint64_t l1_slot  = l0_index % 64;                                     \
                                                                               \
        uint64_t bucket = (bp)->l0[l0_index];                                  \
        bucket |= (1ULL << l0_slot);                                           \
                                                                               \
        if (bucket == UINT64_MAX)                                              \
            (bp)->l1[l1_index] |= (1ULL << l1_slot);                           \
                                                                               \
        (bp)->l0[l0_index] = bucket;                                           \
    } while (0)

#define BIT_POOL_SET_0(N, bp, index)                                           \
    do {                                                                       \
        assert((index) < (uint64_t)(N));                                       \
                                                                               \
        uint64_t l0_index = (index) / 64;                                      \
        uint64_t l0_slot  = (index) % 64;                                      \
        uint64_t l1_index = l0_index / 64;                                     \
        uint64_t l1_slot  = l0_index % 64;                                     \
                                                                               \
        (bp)->l1[l1_index] &= ~(1ULL << l1_slot);                              \
        (bp)->l0[l0_index] &= ~(1ULL << l0_slot);                              \
    } while (0)

#define BIT_POOL_CHECK_1(N, bp, index)                                         \
    (((bp)->l0[(index) / 64] & (1ULL << ((index) % 64))) != 0)

#define BIT_POOL_ALLOC(N, bp, out_index, out_ok)                               \
    do {                                                                       \
        BIT_POOL_FIND_0(N, bp, out_index, out_ok);                             \
        if (out_ok)                                                            \
            BIT_POOL_SET_1(N, bp, (uint64_t)(out_index));                      \
    } while (0)

#endif /* TY_BITPOOL_H_ */

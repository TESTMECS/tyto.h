#ifndef TY_ALLOC_H_
#define TY_ALLOC_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALLOCATOR_MAX_ALIGNMENT 16

#define panic(message)                                                         \
    {                                                                          \
        fprintf(                                                               \
            stderr, "%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, message); \
        exit(EXIT_FAILURE);                                                    \
    }

//! @xmalloc!(count, type)
//! malloc [count] bytes and return [type*]
//! Wraps [zmalloc]
#define xmalloc(count, type) ((type*)zmalloc(sizeof(type) * (count)))
//! malloc [size] bytes and panic if cannot.
static inline void*
zmalloc(size_t size)
{
    void* result = malloc(size);
    if (result == NULL)
        panic("Cannot allocate memory.");
    return result;
}

//! @xcalloc!(count, type)
//!	allocate [count] bytes with [size] bytes each, setting them to 0.
//! Wraps [zcalloc]
#define xcalloc(count, type) ((type*)zcalloc((count), sizeof(type)))

//! zcalloc [count] bytes of [size] and panic if cannot.
static inline void*
zcalloc(size_t count, size_t size)
{
    void* result = calloc(count, size);
    if (result == NULL)
        panic("Cannot allocate memory.");
    return result;
}

//!	@realloc!(ptr, count, type)
//! 	reallocate [ptr] to [count] bytes with the new block [size] wide.
//! Wraps [xrealloc]
#define xrealloc(ptr, count, type)                                             \
    ((type*)zrealloc((ptr), (count), sizeof(type)))

//! reallocate [ptr] checking for [count] > (SIZE_MAX / size) overflow.
static inline void*
zrealloc(void* ptr, size_t count, size_t size)
{
    if (count > SIZE_MAX / size)
        panic("Allocation overflow.");
    void* result = realloc(ptr, count * size);
    if (result == NULL)
        panic("Cannot reallocate memory.");
    return result;
}

typedef struct Allocator
{
    void* (*alloc)(void* ctx, size_t size);
    void* (*remap)(void* ctx, void* buf, size_t new_size);
    void (*free)(void* ctx, void* buf);
} Allocator;

//! Track the alignment lost during allocations.
//! Internal usage for arena implementations.
static inline size_t
_alignment_loss(size_t bytes_allocated, size_t alignment)
{
    size_t offset = bytes_allocated & (alignment - 1);
    if (offset == 0)
        return 0;
    return alignment - offset;
}

//! @Region
//! 	An allocator with a fixed size and capacity.
typedef struct Region Region;
struct Region
{
    void* (*alloc)(Region* region, size_t size);
    void* (*remap)(Region* region, void* buf, size_t new_size);
    void (*free)(Region* region, void* buf);
    size_t size;
    size_t capacity;
    void*  data;
};

//! Casts the [Region] into the allocator interface.
static inline Allocator*
Region_new(Region* region);

//! Initialize the Region with the implementations.
static inline Region
Region_init(void* buf, size_t len);

//! Implementation
void*
Region_alloc(Region* region, size_t size);
void*
Region_remap(Region* region, void* buf, size_t new_size);
void
Region_free(Region* region, void* buf);

//! Check if the fixed buffer is owned.
static inline bool
Region_is_owned(Region* region, void* buf)
{
    return (uint8_t*)region->data <= (uint8_t*)buf &&
           (uint8_t*)buf < (uint8_t*)region->data + region->capacity;
}

//! Size of the Current buffer.
static inline size_t
Region_buffer_size(void* buf)
{
    return *((size_t*)buf - 1);
}

static inline bool
_internal_is_last_allocated(Region* region, void* buf)
{
    size_t   buffer_size = Region_buffer_size(buf);
    uint8_t* prev        = (uint8_t*)region->data + region->size - buffer_size;
    return prev == (uint8_t*)buf;
}

//! @Annex
//! 	An allocator with a fixed region and fallback allocator.
typedef struct Annex Annex;
struct Annex
{
    void* (*alloc)(Annex* annex, size_t size);
    void* (*remap)(Annex* annex, void* buf, size_t new_size);
    void (*free)(Annex* annex, void* buf);
    Region     allocator;
    Allocator* fallback;
};

static inline Allocator*
Annex_new(Annex* annex)
{
    return (Allocator*)annex;
}

static inline Annex
Annex_init(void* buf, size_t len, Allocator* fallback);

//! Implementation
static void*
Annex_alloc(Annex* annex, size_t size);

static void*
Annex_remap(Annex* annex, void* buf, size_t new_size);

static void
Annex_free(Annex* annex, void* buf);

//! @Arena
//! 	An allocator with a linked list of blocks.
typedef struct Arena Arena;

typedef struct ArenaBlock
{
    size_t             size;
    size_t             capacity;
    void*              data;
    struct ArenaBlock* next;
} ArenaBlock;

struct Arena
{
    void* (*alloc)(Arena* arena, size_t size);
    void* (*remap)(Arena* arena, void* buf, size_t new_size);
    void (*free)(Arena* arena, void* buf);
    Allocator*  allocator;
    size_t      first_block_size;
    ArenaBlock* blocks;
    ArenaBlock* current_block;
    ArenaBlock* last_block;
};

static inline Allocator*
Arena_new(Arena* arena)
{
    return (Allocator*)arena;
}

static inline size_t
_block_bytes_left(ArenaBlock* block)
{
    size_t inc = _alignment_loss(block->size, ALLOCATOR_MAX_ALIGNMENT);
    return block->capacity - (block->size + inc);
}

static bool
_block_alloc(Arena* arena, size_t requested_size)
{
    size_t allocated_size;
    if (!arena->blocks) {
        allocated_size = arena->first_block_size;
    } else {
        allocated_size = arena->last_block->capacity;
    }
    if (allocated_size < 1) {
        allocated_size = 1;
    }
    while (allocated_size < requested_size) {
        allocated_size *= 2;
    }
    if (allocated_size > UINT32_MAX) {
        allocated_size = UINT32_MAX;
    }
    Allocator*  allocator = arena->allocator;
    ArenaBlock* new_block =
        (ArenaBlock*)allocator->alloc(allocator, sizeof(ArenaBlock));
    if (!new_block)
        return false;
    new_block->data = allocator->alloc(allocator, allocated_size);
    if (!new_block->data) {
        allocator->free(allocator, new_block);
        return false;
    }
    new_block->size     = 0;
    new_block->capacity = allocated_size;
    new_block->next     = NULL;
    if (!arena->blocks) {
        arena->blocks = new_block;
    } else {
        arena->last_block->next = new_block;
    }
    arena->last_block    = new_block;
    arena->current_block = new_block;
    return true;
}

static inline bool
_is_last_allocated_arena(Arena* arena, void* buf)
{
    size_t buffer_size = Region_buffer_size(buf);  // Same implementation here.
    ArenaBlock* block  = arena->current_block;
    uint8_t*    prev   = (uint8_t*)block->data + block->size - buffer_size;
    return prev == (uint8_t*)buf;
}

//! Implementation
static void*
Arena_alloc(Arena* arena, size_t size);

static void*
Arena_remap(Arena* arena, void* buf, size_t new_size);

static void
Arena_free(Arena* arena, void* buf);

static void
Arena_deinit(Arena* arena);

static void
Arena_reset(Arena* arena);

//! @Vector
//! An Allocator backed vector.
#define Vector(T)                                                              \
    struct                                                                     \
    {                                                                          \
        size_t     size;                                                       \
        T*         data;                                                       \
        size_t     capacity;                                                   \
        Allocator* allocator;                                                  \
    }

typedef Vector(void*) AnyVector;

#define Vector_init(_allocator) {.allocator = (_allocator)}

#define Vector_push(_v, _val)                                                  \
    (Vector_ensure((AnyVector*)(_v), sizeof(*(_v)->data), 1) &&                \
     ((_v)->data[(_v)->size++] = (_val), true))

#define Vector_pushN(_v, _arr, _N)                                             \
    (Vector_ensure((AnyVector*)(_v), sizeof(*(v)->data), (_N)) &&              \
     (memcpy(&(_v)->data[(_v)->size], (_arr), (_N) * sizeof(*(_arr))),         \
      (_v)->size += (_N),                                                      \
      true))

#define Vector_pop(_v)                                                         \
    ((_v)->size > 0 ? ((void)(_v)->data[--(_v)->size], true) : false)

#define Vector_clear(_v) (_v)->size = 0;

#define Vector_deinit(_v)                                                      \
    do {                                                                       \
        (_v)->size     = 0;                                                    \
        (_v)->capacity = 0;                                                    \
        (_v)->allocator->free((_v)->allocator, (_v)->data);                    \
        (_v)->data = NULL;                                                     \
    } while (0)

static bool
Vector_ensure(AnyVector* v, size_t element_size, size_t add_count);

#endif  // TY_ALLOC_H_

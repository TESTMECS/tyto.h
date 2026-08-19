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

//! malloc [size] bytes and panic if cannot.
static inline void*
xmalloc(size_t size)
{
    void* result = malloc(size);
    if (result == NULL)
        panic("Cannot allocate memory.");
    return result;
}

//! zcalloc [count] bytes of [size] and panic if cannot.
static inline void*
xcalloc(size_t count, size_t size)
{
    void* result = calloc(count, size);
    if (result == NULL)
        panic("Cannot allocate memory.");
    return result;
}

//! reallocate [ptr] checking for [count] > (SIZE_MAX / size) overflow.
static inline void*
xrealloc(void* ptr, size_t count, size_t size)
{
    if (count > SIZE_MAX / size)
        panic("Allocation overflow.");
    void* result = realloc(ptr, count * size);
    if (result == NULL)
        panic("Cannot reallocate memory.");
    return result;
}

typedef struct allocator
{
    void* (*alloc)(void* ctx, size_t size);
    void* (*remap)(void* ctx, void* buf, size_t new_size);
    void (*free)(void* ctx, void* buf);
} allocator_t;

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

//! @region
//! 	An allocator with a fixed size and capacity.
typedef struct region region_t;
struct region
{
    void* (*alloc)(region_t* region, size_t size);
    void* (*remap)(region_t* region, void* buf, size_t new_size);
    void (*free)(region_t* region, void* buf);
    size_t size;
    size_t capacity;
    void*  data;
};

//! Casts the [region_t] into the allocator interface.
static inline allocator_t*
region_new(region_t* region);

//! Initialize the region_t with the implementations.
static inline region_t
region_init(void* buf, size_t len);

//! Implementation
void*
region_alloc(region_t* region, size_t size);

void*
region_remap(region_t* region, void* buf, size_t new_size);

void
region_free(region_t* region, void* buf);

//! Check if the fixed buffer is owned.
static inline bool
region_is_owned(region_t* region, void* buf)
{
    return (uint8_t*)region->data <= (uint8_t*)buf &&
           (uint8_t*)buf < (uint8_t*)region->data + region->capacity;
}

//! Size of the Current buffer.
static inline size_t
region_buffer_size(void* buf)
{
    return *((size_t*)buf - 1);
}

static inline bool
_internal_is_last_allocated(region_t* region, void* buf)
{
    size_t   buffer_size = region_buffer_size(buf);
    uint8_t* prev        = (uint8_t*)region->data + region->size - buffer_size;
    return prev == (uint8_t*)buf;
}

//! @annex_t
//! 	An allocator with a fixed region and fallback allocator.
typedef struct annex annex_t;
struct annex
{
    void* (*alloc)(annex_t* annex, size_t size);
    void* (*remap)(annex_t* annex, void* buf, size_t new_size);
    void (*free)(annex_t* annex, void* buf);
    region_t     allocator;
    allocator_t* fallback;
};

static inline allocator_t*
annex_new(annex_t* annex)
{
    return (allocator_t*)annex;
}

static inline annex_t
annex_init(void* buf, size_t len, allocator_t* fallback);

//! Implementation
static void*
annex_alloc(annex_t* annex, size_t size);

static void*
annex_remap(annex_t* annex, void* buf, size_t new_size);

static void
annex_free(annex_t* annex, void* buf);

//! @arena_t
//! 	An allocator with a linked list of blocks.
typedef struct arena arena_t;
typedef struct arena_block
{
    size_t              size;
    size_t              capacity;
    void*               data;
    struct arena_block* next;
} arena_block_t;

struct arena
{
    void* (*alloc)(arena_t* arena, size_t size);
    void* (*remap)(arena_t* arena, void* buf, size_t new_size);
    void (*free)(arena_t* arena, void* buf);
    allocator_t*   allocator;
    size_t         first_block_size;
    arena_block_t* blocks;
    arena_block_t* current_block;
    arena_block_t* last_block;
};

static inline allocator_t*
arena_t_new(arena_t* arena)
{
    return (allocator_t*)arena;
}

static inline size_t
_block_bytes_left(arena_block_t* block)
{
    size_t inc = _alignment_loss(block->size, ALLOCATOR_MAX_ALIGNMENT);
    return block->capacity - (block->size + inc);
}

static bool
_block_alloc(arena_t* arena, size_t requested_size)
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
    allocator_t*   allocator = arena->allocator;
    arena_block_t* new_block =
        (arena_block_t*)allocator->alloc(allocator, sizeof(arena_block_t));
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
_is_last_allocated_arena(arena_t* arena, void* buf)
{
    size_t buffer_size = region_buffer_size(buf);  // Same implementation here.
    arena_block_t* block = arena->current_block;
    uint8_t*       prev  = (uint8_t*)block->data + block->size - buffer_size;
    return prev == (uint8_t*)buf;
}

//! Implementation
static void*
arena_alloc(arena_t* arena, size_t size);

static void*
arena_remap(arena_t* arena, void* buf, size_t new_size);

static void
arena_free(arena_t* arena, void* buf);

static void
arena_deinit(arena_t* arena);

static void
arena_reset(arena_t* arena);

//! @Vector
//! An allocator_t backed vector.
#define Vector(T)                                                              \
    struct                                                                     \
    {                                                                          \
        size_t       size;                                                     \
        T*           data;                                                     \
        size_t       capacity;                                                 \
        allocator_t* allocator;                                                \
    }

typedef Vector(void*) AnyVector;

#define Vector_init(_allocator) {.allocator = (_allocator)}

#define Vector_push(_v, _val)                                                  \
    (Vector_ensure((AnyVector*)(_v), sizeof(*(_v)->data), 1) &&                \
     ((_v)->data[(_v)->size++] = (_val), true))

#define Vector_pushN(_v, _arr, _N)                                             \
    (Vector_ensure((AnyVector*)(_v), sizeof(*(_v)->data), (_N)) &&             \
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

#ifdef TY_ALLOC_IMPL
//! @region_t
static inline allocator_t*
region_new(region_t* region)
{
    return (allocator_t*)region;
}

static inline region_t
region_init(void* buf, size_t len)
{
    return (region_t){
        .alloc    = region_alloc,
        .remap    = region_remap,
        .free     = region_free,
        .size     = 0,
        .capacity = len,
        .data     = buf,
    };
}

void*
region_alloc(region_t* region, size_t size)
{
    if (size == 0)
        return NULL;
    size_t requested_size = size + sizeof(size_t);
    size_t inc = _alignment_loss(region->size, ALLOCATOR_MAX_ALIGNMENT);
    if (region->capacity - region->size < requested_size + inc)
        return NULL;
    void* buffer = (uint8_t*)region->data + region->size + inc;
    region->size += requested_size + inc;
    *((size_t*)buffer) = size;
    buffer             = (size_t*)buffer + 1;
    return buffer;
}
void*
region_remap(region_t* region, void* buf, size_t new_size)
{
    if (!buf)
        return region_alloc(region, new_size);
    size_t old_size = region_buffer_size(buf);
    if (old_size >= new_size)
        return buf;
    uint32_t bytes_left = region->capacity - region->size + old_size;
    void*    new_array;
    if (_internal_is_last_allocated(region, buf) && bytes_left >= new_size) {
        region_free(region, buf);
        new_array = region_alloc(region, new_size);
    } else {
        new_array = region_alloc(region, new_size);
        if (new_array)
            memcpy(new_array, buf, old_size);
    }
    return new_array;
}
void
region_t_free(region_t* region, void* buf)
{
    if (_internal_is_last_allocated(region, buf)) {
        size_t buffer_size = region_buffer_size(buf);
        region->size -= buffer_size + sizeof(size_t);
    }
}

//! @annex_t

static inline annex_t
annex_init(void* buf, size_t len, allocator_t* fallback)
{
    return (annex_t){
        .alloc     = annex_alloc,
        .remap     = annex_remap,
        .free      = annex_free,
        .allocator = region_init(buf, len),
        .fallback  = fallback,
    };
}

static void*
annex_alloc(annex_t* annex, size_t size)
{
    void* buf = annex->allocator.alloc(&annex->allocator, size);
    if (!buf) {
        buf = annex->fallback->alloc(annex->fallback, size);
    }
    return buf;
}

static void*
annex_remap(annex_t* annex, void* buf, size_t new_size)
{
    if (!buf)
        annex_alloc(annex, new_size);
    if (region_is_owned(&annex->allocator, buf)) {
        void* new_buffer =
            annex->allocator.remap(&annex->allocator, buf, new_size);
        if (!new_buffer) {
            new_buffer = annex->fallback->alloc(annex->fallback, new_size);
            if (new_buffer) {
                size_t old_size = region_buffer_size(buf);
                memcpy(new_buffer, buf, old_size);
            }
        }
        return new_buffer;
    }
    return annex->fallback->remap(annex->fallback, buf, new_size);
}

static void
annex_free(annex_t* annex, void* buf)
{
    if (region_is_owned(&annex->allocator, buf)) {
        annex->allocator.free(&annex->allocator, buf);
    } else {
        annex->fallback->free(annex->fallback, buf);
    }
}

//! @arena_t

static inline arena_t
arena_init(size_t block_size, allocator_t* allocator)
{
    return (arena_t){
        .alloc            = arena_alloc,
        .remap            = arena_remap,
        .free             = arena_free,
        .allocator        = (allocator),
        .first_block_size = (block_size),
    };
}

static void*
arena_t_alloc(arena_t* arena, size_t size)
{
    if (size == 0)
        return NULL;
    size_t requested_size = size + sizeof(size_t);
    if (!arena->blocks && !_block_alloc(arena, requested_size)) {
        return NULL;
    }
    while (_block_bytes_left(arena->current_block) < requested_size) {
        arena->current_block = arena->current_block->next;
        if (!arena->current_block) {
            if (!_block_alloc(arena, requested_size)) {
                return NULL;
            }
        }
        break;
    }
    arena_block_t* block = arena->current_block;
    size_t         inc = _alignment_loss(block->size, ALLOCATOR_MAX_ALIGNMENT);
    void*          buf = (uint8_t*)block->data + block->size + inc;
    block->size += requested_size + inc;
    *((size_t*)buf) = size;
    buf             = (size_t*)buf + 1;
    return buf;
}

static void*
arena_t_remap(arena_t* arena, void* buf, size_t new_size)
{
    if (!buf || !arena->blocks) {
        return arena_t_alloc(arena, new_size);
    }
    size_t old_size = region_buffer_size(buf);
    if (old_size >= new_size) {
        return buf;
    }
    arena_block_t* block      = arena->current_block;
    uint32_t       bytes_left = block->capacity - block->size + old_size;
    void*          new_array;
    if (_is_last_allocated_arena(arena, buf) && bytes_left >= new_size) {
        arena_free(arena, buf);
        new_array = arena_t_alloc(arena, new_size);
    } else {
        new_array = arena_t_alloc(arena, new_size);
        if (new_array) {
            memcpy(new_array, buf, old_size);
        }
    }
    return new_array;
}

static void
arena_free(arena_t* arena, void* buf)
{
    if (_is_last_allocated_arena(arena, buf)) {
        arena_block_t* block       = arena->current_block;
        size_t         buffer_size = region_buffer_size(buf);
        block->size -= buffer_size + sizeof(size_t);
    }
}

static void
arena_deinit(arena_t* arena)
{
    allocator_t*   allocator = arena->allocator;
    arena_block_t* current   = arena->blocks;
    while (current) {
        arena_block_t* temp = current;
        current             = current->next;
        allocator->free(allocator, temp->data);
        allocator->free(allocator, temp);
    }
    arena->blocks        = NULL;
    arena->current_block = NULL;
    arena->last_block    = NULL;
}

static void
arena_reset(arena_t* arena)
{
    arena_block_t* current = arena->blocks;
    while (current) {
        current->size = 0;
        current       = current->next;
    }
    arena->current_block = arena->blocks;
}

//! @Vector
static bool
Vector_ensure(AnyVector* v, size_t element_size, size_t add_count)
{
    size_t needed = v->size + add_count;
    if (needed <= v->capacity)
        return true;

    size_t new_capacity = (v->capacity == 0) ? 16 : v->capacity;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }
    void* new_buffer =
        v->allocator->remap(v->allocator, v->data, new_capacity * element_size);
    if (new_buffer == NULL)
        return false;
    v->data     = new_buffer;
    v->capacity = new_capacity;
    return true;
}
#endif

#endif  // TY_ALLOC_H_

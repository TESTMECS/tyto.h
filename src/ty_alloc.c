#include <ty_alloc.h>

//! @Region
static inline Allocator*
Region_new(Region* region)
{
    return (Allocator*)region;
}

static inline Region
Region_init(void* buf, size_t len)
{
    return (Region){
        .alloc    = Region_alloc,
        .remap    = Region_remap,
        .free     = Region_free,
        .size     = 0,
        .capacity = len,
        .data     = buf,
    };
}

void*
Region_alloc(Region* region, size_t size)
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
Region_remap(Region* region, void* buf, size_t new_size)
{
    if (!buf)
        return Region_alloc(region, new_size);
    size_t old_size = Region_buffer_size(buf);
    if (old_size >= new_size)
        return buf;
    uint32_t bytes_left = region->capacity - region->size + old_size;
    void*    new_array;
    if (_internal_is_last_allocated(region, buf) && bytes_left >= new_size) {
        Region_free(region, buf);
        new_array = Region_alloc(region, new_size);
    } else {
        new_array = Region_alloc(region, new_size);
        if (new_array)
            memcpy(new_array, buf, old_size);
    }
    return new_array;
}
void
Region_free(Region* region, void* buf)
{
    if (_internal_is_last_allocated(region, buf)) {
        size_t buffer_size = Region_buffer_size(buf);
        region->size -= buffer_size + sizeof(size_t);
    }
}

//! @Annex

static inline Annex
Annex_init(void* buf, size_t len, Allocator* fallback)
{
    return (Annex){
        .alloc     = Annex_alloc,
        .remap     = Annex_remap,
        .free      = Annex_free,
        .allocator = Region_init(buf, len),
        .fallback  = fallback,
    };
}

static void*
Annex_alloc(Annex* annex, size_t size)
{
    void* buf = annex->allocator.alloc(&annex->allocator, size);
    if (!buf) {
        buf = annex->fallback->alloc(annex->fallback, size);
    }
    return buf;
}

static void*
Annex_remap(Annex* annex, void* buf, size_t new_size)
{
    if (!buf)
        Annex_alloc(annex, new_size);
    if (Region_is_owned(&annex->allocator, buf)) {
        void* new_buffer =
            annex->allocator.remap(&annex->allocator, buf, new_size);
        if (!new_buffer) {
            new_buffer = annex->fallback->alloc(annex->fallback, new_size);
            if (new_buffer) {
                size_t old_size = Region_buffer_size(buf);
                memcpy(new_buffer, buf, old_size);
            }
        }
        return new_buffer;
    }
    return annex->fallback->remap(annex->fallback, buf, new_size);
}

static void
Annex_free(Annex* annex, void* buf)
{
    if (Region_is_owned(&annex->allocator, buf)) {
        annex->allocator.free(&annex->allocator, buf);
    } else {
        annex->fallback->free(annex->fallback, buf);
    }
}

//! @Arena

static inline Arena
Arena_init(size_t block_size, Allocator* allocator)
{
    return (Arena){
        .alloc            = Arena_alloc,
        .remap            = Arena_remap,
        .free             = Arena_free,
        .allocator        = (allocator),
        .first_block_size = (block_size),
    };
}

static void*
Arena_alloc(Arena* arena, size_t size)
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
    ArenaBlock* block = arena->current_block;
    size_t      inc   = _alignment_loss(block->size, ALLOCATOR_MAX_ALIGNMENT);
    void*       buf   = (uint8_t*)block->data + block->size + inc;
    block->size += requested_size + inc;
    *((size_t*)buf) = size;
    buf             = (size_t*)buf + 1;
    return buf;
}

static void*
Arena_remap(Arena* arena, void* buf, size_t new_size)
{
    if (!buf || !arena->blocks) {
        return Arena_alloc(arena, new_size);
    }
    size_t old_size = Region_buffer_size(buf);
    if (old_size >= new_size) {
        return buf;
    }
    ArenaBlock* block      = arena->current_block;
    uint32_t    bytes_left = block->capacity - block->size + old_size;
    void*       new_array;
    if (_is_last_allocated_arena(arena, buf) && bytes_left >= new_size) {
        Arena_free(arena, buf);
        new_array = Arena_alloc(arena, new_size);
    } else {
        new_array = Arena_alloc(arena, new_size);
        if (new_array) {
            memcpy(new_array, buf, old_size);
        }
    }
    return new_array;
}

static void
Arena_free(Arena* arena, void* buf)
{
    if (_is_last_allocated_arena(arena, buf)) {
        ArenaBlock* block       = arena->current_block;
        size_t      buffer_size = Region_buffer_size(buf);
        block->size -= buffer_size + sizeof(size_t);
    }
}

static void
Arena_deinit(Arena* arena)
{
    Allocator*  allocator = arena->allocator;
    ArenaBlock* current   = arena->blocks;
    while (current) {
        ArenaBlock* temp = current;
        current          = current->next;
        allocator->free(allocator, temp->data);
        allocator->free(allocator, temp);
    }
    arena->blocks        = NULL;
    arena->current_block = NULL;
    arena->last_block    = NULL;
}

static void
Arena_reset(Arena* arena)
{
    ArenaBlock* current = arena->blocks;
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

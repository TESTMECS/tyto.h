#include <ty_alloc.h>

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

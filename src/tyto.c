//! @file tyto.c
//! Implementation for the tyto library.
#include "tyto.proto.h"

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

//! @Logging
static void
tyto_log(enum TytoLogKind kind, const char* fmt, ...)
{
    fprintf(stderr, "%s%s:\x1b[0m ", level_color[kind], level_name[kind]);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

//! @Slice
static inline Slice
slice_empty()
{
    return (Slice){.buffer = "", .size = 0};
}

static inline Slice
slice_make(const char* buffer, size_t size)
{
    return (Slice){.buffer = buffer, .size = size};
}

Slice
slice_range(const char* start, const char* end)
{
    const char* s = "";
    const char* e = "";
    if (start > end) {
        s = end;
        e = start;
    } else {
        s = start;
        e = end;
    }
    return slice_make(s, (size_t)(e - s));
}

Slice
slice_cstring(const char* cstring)
{
    return slice_make(cstring, strlen(cstring));
}

static inline size_t
slice_size(Slice s)
{
    return s.size;
}

static inline const char*
slice_raw(Slice s)
{
    return s.buffer;
}

static inline const char*
slice_ref(Slice s, size_t idx)
{
    return &s.buffer[idx];
}

static inline char
slice_at(Slice s, size_t idx)
{
    return s.buffer[idx];
}

int
slice_cmp(Slice s1, Slice s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size > s2.size ? s2.size : s1.size);
}

bool
slice_eq(Slice s1, Slice s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

bool
slice_split(Slice s, char delim, Slice* pre, Slice* post)
{
    bool   ret = false;
    size_t i   = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == delim) {
            ret = true;
            break;
        }
    }
    if (pre)
        *pre = slice_make(&s.buffer[0], i);
    if (post)
        *post = ret ? slice_make(&s.buffer[i + 1], s.size - i - 1)
                    : slice_make(&s.buffer[s.size], 0);
    return ret;
}

bool
slice_split_n(Slice s, char delim, size_t n, Slice* pre, Slice* post)
{
    bool   ret = false;
    size_t i   = 0;
    for (i = 0; i < (n < s.size ? n : s.size); i++) {
        if (s.buffer[i] == delim) {
            ret = true;
            break;
        }
    }
    if (pre)
        *pre = ret ? slice_make(s.buffer, i) : s;
    if (post)
        *post = ret ? slice_make(&s.buffer[i + 1], s.size - i - 1)
                    : slice_make(&s.buffer[s.size], 0);
    return ret;
}

bool
slice_is_empty(Slice s)
{
    return s.size == 0;
}

bool
slice_has(Slice s, char c)
{
    size_t i = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            return true;
    }
    return false;
}

bool
slice_find(Slice s, char c, size_t* idx)
{
    size_t i = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c) {
            if (idx)
                *idx = i;
            return true;
        }
    }
    return false;
}

bool
slice_rfind(Slice s, char c, size_t* idx)
{
    size_t i = 0;
    for (i = s.size; i > 0; i--) {
        if (s.buffer[i - 1] == c) {
            if (idx)
                *idx = i - 1;
            return true;
        }
    }
    return false;
}

size_t
slice_count(Slice s, char c)
{
    size_t i     = 0;
    size_t count = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            count++;
    }
    return count;
}

bool
slice_starts_with(Slice s, Slice prefix)
{
    size_t i = 0;
    if (prefix.size > s.size)
        return false;

    for (i = 0; i < prefix.size; i++) {
        if (prefix.buffer[i] != s.buffer[i])
            return false;
    }
    return true;
}

bool
slice_ends_with(Slice s, Slice postfix)
{
    size_t i = 0;
    size_t j = 0;
    if (postfix.size > s.size)
        return false;

    for (i = s.size, j = postfix.size; j > 0; i--, j--) {
        if (s.buffer[i - 1] != postfix.buffer[j - 1])
            return false;
    }
    return true;
}
//! ## String ##
static void
string_grow(String* s);

static void
string_grow_to(String* s, size_t cap);

static String
string_null();

String
string_empty()
{
    return (String){.size = 0, .capacity = 0, .buffer = NULL};
}

String
string_cstring(const char* buf)
{
    size_t len = strlen(buf);
    String s   = (String){.size     = len,
                          .capacity = len,
                          .buffer   = zmalloc(s.capacity * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    memmove(s.buffer, buf, s.size);
    return s;
}

String
string_make(size_t cap)
{
    String s = (String){
        .size = 0, .capacity = cap, .buffer = zmalloc(cap * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    return s;
}

String
string_slice(Slice sl)
{
    String s = (String){
        .size     = sl.size,
        .capacity = sl.size,
        .buffer   = zmalloc(sl.size * sizeof(char)),
    };
    if (s.buffer == NULL)
        return string_null();

    memmove(s.buffer, sl.buffer, s.size);

    return s;
}

String
string_copy(String s)
{
    String str = (String){
        .size     = s.size,
        .capacity = s.capacity,
        .buffer   = zmalloc(s.capacity * sizeof(char)),
    };
    if (str.buffer == NULL)
        return string_null();
    memmove(str.buffer, s.buffer, str.size);
    return str;
}

String
string_move(String* s)
{
    String str = {0};
    str        = *s;
    *s         = string_empty();
    return str;
}

Slice
string_toslice(String s)
{
    return slice_make(s.buffer, s.size);
}

void
string_free(String* s)
{
    free(s->buffer);
    *s = string_empty();
}

size_t
string_size(String s)
{
    return s.size;
}

size_t
string_cap(String s)
{
    return s.capacity;
}

bool
string_is_empty(String s)
{
    return s.buffer == NULL && s.size == 0;
}

bool
string_is_null(String s)
{
    return s.buffer == NULL && s.size == SIZE_MAX && s.capacity == SIZE_MAX;
}

int
string_cmp(String s1, String s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size < s2.size ? s1.size : s2.size);
}

bool
string_eq(String s1, String s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

char
string_at(String s, size_t idx);

char*
string_ref(String s, size_t idx);

void
string_insert(String* s, size_t idx, char c)
{
    if (idx >= s->size) {
        *s = string_null();
        return;
    }
    if (s->size == s->capacity) {
        string_grow(s);
        if (string_is_null(*s))
            return;
    }
    memmove(&s->buffer[idx + 1], &s->buffer[idx], s->size - idx);
    s->buffer[idx] = c;
    s->size++;
}

void
string_insert_slice(String* s, size_t idx, Slice v)
{
    if (idx >= s->size) {
        *s = string_null();
        return;
    }
    if (s->size + v.size > s->capacity) {
        string_grow_to(s, s->size + v.size);
        if (string_is_null(*s))
            return;
    }
    memmove(&s->buffer[idx + v.size], &s->buffer[idx], s->size - idx);
    memmove(&s->buffer[idx], v.buffer, v.size);
    s->size += v.size;
}

void
string_push(String* s, char c)
{
    if (s->size == s->capacity) {
        string_grow(s);
        if (string_is_null(*s))
            return;
    }
    s->buffer[s->size++] = c;
}

void
string_push_slice(String* s, Slice sl)
{
    if (s->size + sl.size > s->capacity) {
        string_grow_to(s, s->size + sl.size);
        if (string_is_null(*s))
            return;
    }
    memmove(&s->buffer[s->size], sl.buffer, sl.size);
    s->size += sl.size;
}

char
string_pop(String* s)
{
    return s->buffer[--s->size];
}

char
string_remove(String* s, size_t idx)
{
    char c = s->buffer[idx];

    memmove(&s->buffer[idx], &s->buffer[idx + 1], s->size - idx - 1);
    s->size--;
    return c;
}

static void
string_grow(String* s)
{
    if (s->capacity == 0) {
        s->capacity = STRING_INITIAL_CAPACITY;
    } else {
        s->capacity *= STRING_GROWTH_FACTOR;
    }
    s->buffer = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static void
string_grow_to(String* s, size_t cap)
{
    s->capacity = cap;
    s->buffer   = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static String
string_null()
{
    return (String){.size = SIZE_MAX, .capacity = SIZE_MAX, .buffer = NULL};
}

String
read_file_tostring(char* name)
{
    ensure(name);

    FILE* f = fopen(name, "r");
    panic_if(f == NULL, "Cannot open %s", name);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* s           = xmalloc(size + 1, char);
    long  size_t_read = fread(s, 1, size, f);

    panic_if(
        size_t_read < size && feof(f) == 0, "Cannot read %s to end.\n", name);

    s[size_t_read] = '\0';

    fclose(f);
    return string_slice((Slice){.buffer = s, .size = size_t_read});
}

void
write_file_wstring(char* name, String data)
{
    ensure(name);

    FILE* f = fopen(name, "w");
    panic_if(f == NULL, "Cannot open %s", name);

    size_t bytes_written = fwrite(data.buffer, 1, data.size, f);
    fclose(f);
    panic_if(bytes_written != data.size, "Cannot write data to file %s.", name);
}

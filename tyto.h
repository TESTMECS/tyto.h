/***************************************

  /$$$$$$$$          /$$
 |__  $$__/         | $$
    | $$ /$$   /$$ /$$$$$$    /$$$$$$
    | $$| $$  | $$|_  $$_/   /$$__  $$
    | $$| $$  | $$  | $$    | $$  \ $$
    | $$| $$  | $$  | $$ /$$| $$  | $$
    | $$|  $$$$$$$  |  $$$$/|  $$$$$$/
    |__/ \____  $$   \___/   \______/
         /$$  | $$
        |  $$$$$$/
         \______/
***************************************/
#ifndef TYTO_PROTO_H_
#define TYTO_PROTO_H_

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! General macro magic.
#define TYTO_CAT__(a, b) a##b
#define TYTO_CAT_(a, b)  TYTO_CAT__(a, b)
#define TYTO_CAT(a, b)   TYTO_CAT_(a, b)

//! @puts!
#define putsln   printf("%s:%d\n", __func__, __LINE__)
#define putsi(i) printf("%s:%d: %d\n", __func__, __LINE__, i)
#define putss(s) printf("%s:%d: %s\n", __func__, __LINE__, s)
#define putsf(...)                                                             \
    {                                                                          \
        fprintf(stderr, "%s:%d: ", __func__, __LINE__);                        \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    }

//! Creates a unique identifier with the line number.
#define UNIQUE_ID(name) TYTO_CAT(name, __LINE__)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//! @TYTO_STATIC_ASSERT!
//! 	C99 static assert, creates a negative sized type if the condition is
//! false.
#define TYTO_STATIC_ASSERT(e)                                                  \
    typedef char TYTO_CAT(compile_time_assertion, __LINE__)[(e) ? 1 : -1]

//! @ARRAY_SIZE!
//! 	Calculates the length of an array.
#define ARRAY_SIZE(...) (sizeof(__VA_ARGS__) / sizeof(*(__VA_ARGS__)))

//! @ensure!(arg)
//!		Checks if [arg] is null and exits if so.
#define ensure(argument)                                                       \
    if (argument == NULL) {                                                    \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: %s's precondition \"not null\" (" #argument          \
            ") violated\n",                                                    \
            __FILE__,                                                          \
            __LINE__,                                                          \
            __func__);                                                         \
        exit(EXIT_FAILURE);                                                    \
    }

//! @require!(description, condition)
//! 	require a specific description to be true.
#define require(description, condition)                                        \
    if (!(condition)) {                                                        \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: %s's precondition \"%s\" (%s) violated\n",           \
            __FILE__,                                                          \
            __LINE__,                                                          \
            __func__,                                                          \
            description,                                                       \
            #condition);                                                       \
        exit(EXIT_FAILURE);                                                    \
    }

//! @xassert!(description, condition)
//! 	assert macro with a description.
#define xassert(description, condition)                                        \
    if (!(condition)) {                                                        \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: assertion \"%s\" (%s) violated\n",                   \
            __FILE__,                                                          \
            __LINE__,                                                          \
            description,                                                       \
            #condition);                                                       \
        exit(EXIT_FAILURE);                                                    \
    }

//! @alive!(pointer)
//! 	Checks if a pointer is not null.
#define alive(pointer)                                                         \
    if (pointer == NULL) {                                                     \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: assertion \"not null\" (" #pointer ") violated\n",   \
            __FILE__,                                                          \
            __LINE__);                                                         \
        exit(EXIT_FAILURE);                                                    \
    }

//! @panic!(message)
//! 	Prints error message and exits the program.
#define panic(message)                                                         \
    {                                                                          \
        fprintf(                                                               \
            stderr, "%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, message); \
        exit(EXIT_FAILURE);                                                    \
    }

//! @panicf!(message)
//! 	Prints error message and exits the program with format args.
#define panicf(...)                                                            \
    {                                                                          \
        fprintf(stderr, "%s:%d, %s: ", __FILE__, __LINE__, __func__);          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }

//! @panic_if!(condition, ...)
//! 	If the condition is true exits the program after printing the format
//! args.
#define panic_if(condition, ...)                                               \
    if (condition) {                                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
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

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

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

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

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
    if (count > SIZE_MAX / size) {
        panic("Allocation overflow.");
    }

    void* result = realloc(ptr, count * size);

    if (result == NULL) {
        panic("Cannot reallocate memory.");
    }

    return result;
}

//! @Allocator Interface
//! 	General interface allocator.

#define ALLOCATOR_MAX_ALIGNMENT 16

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
//! An Arena backed vector.
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

//! @Logger
enum TytoLogKind
{
    LOG_DEBUG,
    LOG_NOTE,
    LOG_WARNING,
    LOG_ERROR,
};

static const char* level_name[] = {
    [LOG_DEBUG]   = "debug",
    [LOG_NOTE]    = "note",
    [LOG_WARNING] = "warning",
    [LOG_ERROR]   = "error",
};

static const char* level_color[] = {
    [LOG_DEBUG]   = "\x1b[1m",
    [LOG_NOTE]    = "\x1b[1;96m",
    [LOG_WARNING] = "\x1b[1;95m",
    [LOG_ERROR]   = "\x1b[1;91m",
};

//! Debug Macro.
#define dbg(...) tyto_log(LOG_DEBUG, __VA_ARGS__)

//! Logging function.
static void
tyto_log(enum TytoLogKind kind, const char* fmt, ...);

//!
//! Slice
//! Just a pointer + size.
//! Stack allocated.
typedef struct
{
    const char* buffer;
    size_t      size;
} Slice;

//! Create a [size] 0 slice.
static inline Slice
slice_empty();

//! Create a slice from [cstring] and [size]
static inline Slice
slice_make(const char* buffer, size_t size);

//! Create a slice from [start] to [end] of cstring.
Slice
slice_range(const char* start, const char* end);

//! Create a slice from [cstring]
//! 	@panic_if [cstring] is NULL.
Slice
slice_cstring(const char* cstring);

//! Return size of the slice.
static inline size_t
slice_size(Slice s);

//! Return pointer to underlying buffer.
static inline const char*
slice_raw(Slice s);

//! Return pointer to char of underlying buffer.
static inline const char*
slice_ref(Slice s, size_t idx);

//! Get char at [idx]-th char in [s].
//! @panic_if idx <= [s.size]
static inline char
slice_at(Slice s, size_t idx);

//! Compare slices.
//! 1  ? [s1] > [s2] :
//! 0  ? [s1] == [s2] :
//! -1 ? [s1] < [s2] ;
int
slice_cmp(Slice s1, Slice s2);

//! Return true if [s1] == [s2]
bool
slice_eq(Slice s1, Slice s2);

//!  Splits [s] into part before [delim] and part after [delim]
//!  If [delim] not found, [pre] gets whole [s] string and [post]
//!  gets assigned the empty string starting at the first character
//!  after the end of s.
bool
slice_split(Slice s, char delim, Slice* pre, Slice* post);

//! Same as [slice_split] however checks at most [n] characters.
bool
slice_split_n(Slice s, char delim, size_t n, Slice* pre, Slice* post);

//! Predicate on [s.size] and [s.buffer]
bool
slice_is_empty(Slice s);

//! Predicate on [s.buffer] for [c] char.
//! Complexity: TODO
bool
slice_has(Slice s, char c);

//! Find [c] char in [s] setting [idx] to where it was found.
//! Complexity: TODO
bool
slice_find(Slice s, char c, size_t* idx);

//! Find [c] char in [s] setting [idx], but from the end.
//! Complexity: TODO
bool
slice_rfind(Slice s, char c, size_t* idx);

//! Count [c] chars in [s].
size_t
slice_count(Slice s, char c);

//! Predicate on [s.buffer] starting with [prefix.buffer]
bool
slice_starts_with(Slice s, Slice prefix);

//! Predicate on [s.buffer] ending with [postfix.buffer]
bool
slice_ends_with(Slice s, Slice postfix);

//! @String
//! A Heap allocated String.
//! Ptr + size + capacity
const size_t STRING_INITIAL_CAPACITY = 8;
const size_t STRING_GROWTH_FACTOR    = 2;

typedef struct
{
    char*  buffer;
    size_t size;
    size_t capacity;
} String;

//! Creates an empty String.
String
string_empty();

//! Create String from cstring.
String
string_cstring(const char* buf);

//! Create String from [cap].
String
string_make(size_t cap);

//! Create String from [slice]
String
string_slice(Slice slice);

//! Copy [src] string and return the copy.
String
string_copy(String src);

//! Move [s] to the returned string.
String
string_move(String* s);

//! Create a Slice from [s].
Slice
string_toslice(String s);

//! Frees memory allocated for [s].
void
string_free(String* s);

//! Get [s.size].
size_t
string_size(String s);

//! Get [s.cap].
size_t
string_cap(String s);

//! Predicate on [s.size].
bool
string_is_empty(String s);

//! Predicate on [s.buffer] if null.
bool
string_is_null(String s);

//! Compare strings.
int
string_cmp(String s1, String s2);

//! Check if string is eq.
bool
string_eq(String s1, String s2);

//! Returns [idx]-th char in [s].
char
string_at(String s, size_t idx);

//! Returns pointer to [idx]-th char in [s].
char*
string_ref(String s, size_t idx);

//! Insert [c] into the [idx]-th position of [s].
void
string_insert(String* s, size_t idx, char c);

//! Insert copy of [slice] into [idx]-th position of [s].
void
string_insert_slice(String* s, size_t idx, Slice slice);

//! Push [c] to the end of [s].
void
string_push(String* s, char c);

//! Push copy of [v] to end of [s].
void
string_push_slice(String* s, Slice sl);

//! Pops and returns the last character in [s].
char
string_pop(String* s);

//! Removes and returns the [idx]-th char in [s].
char
string_remove(String* s, size_t idx);

//! @StringFile
String
read_file_tostring(char* name);

void
write_file_wstring(char* name, String data);

#ifdef TYTO_IMPL
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

#endif  // TYTO_IMPL
#endif  // TYTO_PROTO_H_

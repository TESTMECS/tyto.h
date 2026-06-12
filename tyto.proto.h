//! @file tyto.proto.h
//! 	Prototype header.
//! General outline:
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

#endif  // TYTO_PROTO_H_

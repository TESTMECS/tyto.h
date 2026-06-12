//! @file tyto.proto.h
//! 	Prototype header.
//! General outline:
#ifndef TYTO_PROTO_H_
#define TYTO_PROTO_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYTO_CAT__(a, b) a##b
#define TYTO_CAT_(a, b)  TYTO_CAT__(a, b)
#define TYTO_CAT(a, b)   TYTO_CAT_(a, b)

/* @puts! */
#define putsln   printf("%s:%d\n", __func__, __LINE__)
#define putsi(i) printf("%s:%d: %d\n", __func__, __LINE__, i)
#define putss(s) printf("%s:%d: %s\n", __func__, __LINE__, s)
#define putsf(...)                                                             \
    {                                                                          \
        fprintf(stderr, "%s:%d: ", __func__, __LINE__);                        \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    }

/* Creates a unique identifier with the line number. */
#define UNIQUE_ID(name) TYTO_CAT(name, __LINE__)

/**
 * @TYTO_STATIC_ASSERT!
 * C99 static assert, creates a negative sized type if the condition is false.
 **/
#define TYTO_STATIC_ASSERT(e)                                                  \
    typedef char TYTO_CAT(compile_time_assertion, __LINE__)[(e) ? 1 : -1]

/**
 * @ARRAY_SIZE!
 * Calculates the length of an array.
 **/
#define ARRAY_SIZE(...) (sizeof(__VA_ARGS__) / sizeof(*(__VA_ARGS__)))

/**
 * @ensure!(arg)
 * 	Checks if [arg] is null and exits if so.
 **/
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

/**
 * @require!(description, condition)
 * 	require a specific description to be true.
 **/
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

/**
 * @xassert!(description, condition)
 * 	assert macro with a description.
 **/
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

/**
 * @alive!(pointer)
 * Checks if a pointer is not null.
 **/
#define alive(pointer)                                                         \
    if (pointer == NULL) {                                                     \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: assertion \"not null\" (" #pointer ") violated\n",   \
            __FILE__,                                                          \
            __LINE__);                                                         \
        exit(EXIT_FAILURE);                                                    \
    }

/**
 * @panic!(message)
 * Prints error message and exits the program.
 **/
#define panic(message)                                                         \
    {                                                                          \
        fprintf(                                                               \
            stderr, "%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, message); \
        exit(EXIT_FAILURE);                                                    \
    }

/**
 * @panicf!(message)
 * Prints error message and exits the program with format args.
 **/
#define panicf(...)                                                            \
    {                                                                          \
        fprintf(stderr, "%s:%d, %s: ", __FILE__, __LINE__, __func__);          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }

/**
 * @panic_if!(condition, ...)
 * If the condition is true exits the program after printing the format args.
 **/
#define panic_if(condition, ...)                                               \
    if (condition) {                                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }

/**
 * @xmalloc!(count, type)
 *	malloc [count] bytes and return [type*]
 *  Wraps [zmalloc]
 **/
#define xmalloc(count, type) ((type*)zmalloc(sizeof(type) * (count)))

/* malloc [size] bytes and panic if cannot. */
static inline void*
zmalloc(size_t size)
{
    void* result = malloc(size);

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

    return result;
}

/**
 * @xcalloc!(count, type)
 * 	allocate [count] bytes with [size] bytes each, setting them to 0.
 * Wraps [zcalloc]
 **/
#define xcalloc(count, type) ((type*)zcalloc((count), sizeof(type)))

/* zcalloc [count] bytes of [size] and panic if cannot. */
static inline void*
zcalloc(size_t count, size_t size)
{
    void* result = calloc(count, size);

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

    return result;
}

/**
 * @realloc!(ptr, count, type)
 * reallocate [ptr] to [count] bytes with the new block [size] wide.
 * Wraps [xrealloc]
 **/
#define xrealloc(ptr, count, type)                                             \
    ((type*)zrealloc((ptr), (count), sizeof(type)))

/* reallocate [ptr] checking for [count] > (SIZE_MAX / size) overflow. */
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

/**
 * ## Typedefs ##
 * Type definitions for <inttypes.h> similar to rust.
 **/
typedef int8_t    i8;
typedef uint8_t   u8;
typedef uint8_t   byte;
typedef int16_t   i16;
typedef uint16_t  u16;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef int64_t   i64;
typedef uint64_t  u64;
typedef double    f64;
typedef float     f32;
typedef intptr_t  iptr;
typedef uintptr_t uptr;
typedef size_t    sz;

/**
 * ## Arena ##
 * Simple Arena for EZ allocations, Linked list allocator.
 **/

#define ARENA_PAGE_SIZE (size_t)4096

/* @Arena(struct) */
typedef struct Arena
{
    u8*           region; /* pointer to allocated memory */
    sz            total_sz;
    sz            current_sz;
    struct Arena* next; /* Pointer to the next arena in linked list */
} Arena;

/* Create a new arena. */
Arena*
arena_new();

/* Allocate memory in the areana */
void*
arena_alloc(Arena* a, sz size);

/* free the arena. */
void
arena_free(Arena* a);

/**
 * ## Slice ##
 * Just a pointer + size.
 * Stack allocated.
 **/

typedef struct
{
    const char* buffer;
    sz          size;
} Slice;

/* Create a [size] 0 slice. */
Slice
slice_empty();

/* Create a slice from [cstring] and [size] */
Slice
slice_make(const char* buffer, sz size);

/* Create a slice from [start] to [end] of cstring. */
Slice
slice_range(const char* start, const char* end);

/**
 * Create a slice from [cstring]
 * @panic_if [cstring] is NULL.
 **/
Slice
slice_cstring(const char* cstring);

/* Return size of the slice. */
sz
slice_size(Slice s);

/* Return pointer to underlying buffer. */
const char*
slice_raw(Slice s);

/* Return pointer to char of underlying buffer. */
const char*
slice_ref(Slice s, sz idx);

/**
 * Get char at [idx]-th char in [s].
 * @panic_if idx <= [s.size]
 **/
char
slice_at(Slice s, sz idx);

/**
 * Compare slices.
 * 1  ? [s1] > [s2] :
 * 0  ? [s1] == [s2] :
 * -1 ? [s1] < [s2] ;
 **/
int
slice_cmp(Slice s1, Slice s2);

/* Return true if [s1] == [s2] */
bool
slice_eq(Slice s1, Slice s2);

/**
 * Splits [s] into part before [delim] and part after [delim]
 * If [delim] not found, [pre] gets whole [s] string and [post]
 * gets assigned the empty string starting at the first character
 * after the end of s.
 **/
bool
slice_split(Slice s, char delim, Slice* pre, Slice* post);

/* Same as [slice_split] however checks at most [n] characters. */
bool
slice_split_n(Slice s, char delim, sz n, Slice* pre, Slice* post);

/* Predicate on [s.size] and [s.buffer] */
bool
slice_is_empty(Slice s);

/**
 * Predicate on [s.buffer] for [c] char.
 * Complexity: TODO
 **/
bool
slice_has(Slice s, char c);

/**
 * Find [c] char in [s] setting [idx] to where it was found.
 * Complexity: TODO
 **/
bool
slice_find(Slice s, char c, sz* idx);

/**
 * Find [c] char in [s] setting [idx], but from the end.
 * Complexity: TODO
 **/
bool
slice_rfind(Slice s, char c, sz* idx);

/* Count [c] chars in [s]. */
sz
slice_count(Slice s, char c);

/* Predicate on [s.buffer] starting with [prefix.buffer] */
bool
slice_starts_with(Slice s, Slice prefix);

/* Predicate on [s.buffer] ending with [postfix.buffer] */
bool
slice_ends_with(Slice s, Slice postfix);

/**
 * ## String ##
 * A Heap allocated String.
 * Ptr + size + capacity
 **/

const sz INITIAL_CAPACITY = 8;
const sz GROWTH_FACTOR    = 2;

typedef struct
{
    char* buffer;
    sz    size;
    sz    capacity;
} String;

/* Creates an empty String. */
String
string_empty();

/* Create String from cstring. */
String
string_cstring(const char* buf);

/* Create String from [cap]. */
String
string_make(sz cap);

/* Create String from [slice] */
String
string_slice(Slice slice);

/* Copy [src] string and return the copy. */
String
string_copy(String src);

/* Move [s] to the returned string. */
String
string_move(String* s);

/* Create a Slice from [s]. */
Slice
string_toslice(String s);

/* Frees memory allocated for [s]. */
void
string_free(String* s);

/* Get [s.size]. */
sz
string_size(String s);

/* Get [s.cap]. */
sz
string_cap(String s);

/* Predicate on [s.size]. */
bool
string_is_empty(String s);

/* Predicate on [s.buffer] if null. */
bool
string_is_null(String s);

/* Compare strings. */
int
string_cmp(String s1, String s2);

/* Check if string is eq. */
bool
string_eq(String s1, String s2);

/* Returns [idx]-th char in [s]. */
char
string_at(String s, size_t idx);

/* Returns pointer to [idx]-th char in [s]. */
char*
string_ref(String s, sz idx);

/* Insert [c] into the [idx]-th position of [s]. */
void
string_insert(String* s, sz idx, char c);

/* Insert copy of [slice] into [idx]-th position of [s]. */
void
string_insert_slice(String* s, sz idx, Slice slice);

/* Push [c] to the end of [s]. */
void
string_push(String* s, char c);

/* Push copy of [v] to end of [s]. */
void
string_push_slice(String* s, Slice sl);

/* Pops and returns the last character in [s]. */
char
string_pop(String* s);

/* Removes and returns the [idx]-th char in [s]. */
char
string_remove(String* s, sz idx);

/* ## String file api ## */
String
read_file_tostring(char* name);

void
write_file_wstring(char* name, String data);

#endif  // TYTO_PROTO_H_

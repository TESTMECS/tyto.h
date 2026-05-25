//! @file tyto.h
//!
//!  /$$$$$$$$          /$$
//! |__  $$__/         | $$
//!    | $$ /$$   /$$ /$$$$$$    /$$$$$$
//!    | $$| $$  | $$|_  $$_/   /$$__  $$
//!    | $$| $$  | $$  | $$    | $$  \ $$
//!    | $$| $$  | $$  | $$ /$$| $$  | $$
//!    | $$|  $$$$$$$  |  $$$$/|  $$$$$$/
//!    |__/ \____  $$   \___/   \______/
//!         /$$  | $$
//!        |  $$$$$$/
//!         \______/
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

//! @puts!
//! printf macros
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

//! @TYTO_STATIC_ASSERT!
//! C99 static assert, creates a negative sized type if the condition is false.
#define TYTO_STATIC_ASSERT(e)                                                  \
    typedef char TYTO_CAT(compile_time_assertion, __LINE__)[(e) ? 1 : -1]

//! @ARRAY_SIZE!
//! Calculates the length of an array.
#define ARRAY_SIZE(...) (sizeof(__VA_ARGS__) / sizeof(*(__VA_ARGS__)))

//! @DA_TYPE!
//!		Create dynamic array struct for type.
//! Example:
//! ---
//! DA_TYPE(string, char)
//!
//! typedef struct string {
//!		char* items;
//!		size_t size;
//!		size_t cap;
//! } string;
#define DA_TYPE(name, type)                                                    \
    typedef struct name                                                        \
    {                                                                          \
        type*  items;                                                          \
        size_t size;                                                           \
        size_t capacity;                                                       \
    } name;

//! @DA_APPEND!
//! 	Append to dynamic array.
//! Example:
//! ---
//! DA_TYPE(msg_array, char)
//! DA_APPEND(msg_array, "Hello")
#define DA_APPEND(xs, x)                                                       \
    do {                                                                       \
        if (xs.count >= xs.capacity) {                                         \
            if (xs.capacity == 0)                                              \
                xs.capacity = 256;                                             \
            else                                                               \
                xs.capacity *= 2;                                              \
            xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items));     \
        }                                                                      \
        xs.items[xs.count++] = x;                                              \
    } while (0)

//! @DA_SETREF!
//! 	Set reference to dynamic array.
//! Example:
//! strings str = DA_TYPE(strings, char);
//! char* msg[] = {"Hello", "World"};
//! DA_SETREF(str, msg, 4, 2)
#define DA_SETREF(xs, ptr, size_, count_)                                      \
    do {                                                                       \
        xs.items    = ptr;                                                     \
        xs.count    = count_;                                                  \
        xs.capacity = size_;                                                   \
    } while (0)

//! @DA_FREE!
//! 	Free dynamic array.
#define DA_FREE(xs)                                                            \
    do {                                                                       \
        if (xs.items)                                                          \
            free(xs.items);                                                    \
        xs.count    = 0;                                                       \
        xs.capacity = 0;                                                       \
    } while (0)

//! @DA_RESET!
#define DA_RESET(xs)                                                           \
    do {                                                                       \
        if (xs.items)                                                          \
            free(xs.items);                                                    \
        xs.items    = NULL;                                                    \
        xs.count    = 0;                                                       \
        xs.capacity = 0;                                                       \
    } while (0)

//! @ensure!(arg)
//! 	Checks if [arg] is null and exits if so.
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
//! 	malloc [count] bytes and return [type*]
//!   Wraps [xmalloc_impl] for portability.
#define xmalloc(count, type) ((type*)xmalloc_impl(sizeof(type) * (count)))

//! malloc [size] bytes and panic if cannot.
static inline void*
xmalloc_impl(size_t size)
{
    void* result = malloc(size);

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

    return result;
}

//! @xcalloc!(count, type)
//! 	allocate [count] bytes with [size] bytes each, setting them to 0.
//! Wraps [xcalloc_impl] for portability.
#define xcalloc(count, type) ((type*)xcalloc_impl((count), sizeof(type)))

//! calloc [count] bytes of [size] and panic if cannot.
static inline void*
xcalloc_impl(size_t count, size_t size)
{
    void* result = calloc(count, size);

    if (result == NULL) {
        panic("Cannot allocate memory.");
    }

    return result;
}

//! @realloc!(ptr, count, type)
//! 	reallocate [ptr] to [count] bytes with the new block [size] wide.
//! Wraps [xrealloc_impl] for portability.
#define xrealloc(ptr, count, type)                                             \
    ((type*)xrealloc_impl((ptr), (count), sizeof(type)))

//! reallocate [ptr] checking for [count] > (SIZE_MAX / size) overflow.
static inline void*
xrealloc_impl(void* ptr, size_t count, size_t size)
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

//! ## Typedefs ##
//! Type definitions for <inttypes.h> similar to rust.
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

//! ## Arena ##
//! 	Simple Arena for EZ allocations, Linked list allocator.

#define ARENA_PAGE_SIZE (size_t)4096
//! @Arena(struct)
typedef struct Arena
{
    u8*           region; /* pointer to allocated memory */
    sz            total_sz;
    sz            current_sz;
    struct Arena* next; /* Pointer to the next arena in linked list */
} Arena;

//! Create a new arena.
Arena*
arena_new();

//! Allocate memory in the areana
void*
arena_alloc(Arena* a, sz size);

//! free the arena.
void
arena_free(Arena* a);

//! ## Slice ##
//! Just a pointer + size.
//! Stack allocated.

typedef struct
{
    const char* buffer;
    sz          size;
} Slice;

//! Create a [size] 0 slice.
Slice
slice_empty();

//! Create a slice from [cstring] and [size]
Slice
slice_make(const char* buffer, sz size);

//! Create a slice from [start] to [end] of cstring.
Slice
slice_range(const char* start, const char* end);

//! Create a slice from [cstring]
//! @panic_if [cstring] is NULL.
Slice
slice_cstring(const char* cstring);

//! Return size of the slice.
sz
slice_size(Slice s);

//! Return pointer to underlying buffer.
const char*
slice_raw(Slice s);

//! Return pointer to char of underlying buffer.
const char*
slice_ref(Slice s, sz idx);

//! Get char at [idx]-th char in [s].
//! @panic_if idx <= [s.size]
char
slice_at(Slice s, sz idx);

//! Compare slices.
//! 1  ? [s1] > [s2] :
//! 0  ? [s1] == [s2] :
//! -1 ? [s1] < [s2] ;
int
slice_cmp(Slice s1, Slice s2);

//! Return true if [s1] == [s2]
bool
slice_eq(Slice s1, Slice s2);

//! Splits [s] into part before [delim] and part after [delim]
//! If [delim] not found, [pre] gets whole [s] string and [post]
//! gets assigned the empty string starting at the first character
//! after the end of s.
bool
slice_split(Slice s, char delim, Slice* pre, Slice* post);

//! Same as [slice_split] however checks at most [n] characters.
bool
slice_split_n(Slice s, char delim, sz n, Slice* pre, Slice* post);

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
slice_find(Slice s, char c, sz* idx);

//! Find [c] char in [s] setting [idx], but from the end.
//! Complexity: TODO
bool
slice_rfind(Slice s, char c, sz* idx);

//! Count [c] chars in [s].
sz
slice_count(Slice s, char c);

//! Predicate on [s.buffer] starting with [prefix.buffer]
bool
slice_starts_with(Slice s, Slice prefix);

//! Predicate on [s.buffer] ending with [postfix.buffer]
bool
slice_ends_with(Slice s, Slice postfix);

//! ## String ##
//! A Heap allocated String.
//! Ptr + size + capacity

const sz INITIAL_CAPACITY = 8;
const sz GROWTH_FACTOR    = 2;

typedef struct
{
    char* buffer;
    sz    size;
    sz    capacity;
} String;

//! Creates an empty String.
String
string_empty();

//! Create String from cstring.
String
string_cstring(const char* buf);

//! Create String from [cap].
String
string_make(sz cap);

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
sz
string_size(String s);

//! Get [s.cap].
sz
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
string_ref(String s, sz idx);

//! Insert [c] into the [idx]-th position of [s].
void
string_insert(String* s, sz idx, char c);

//! Insert copy of [slice] into [idx]-th position of [s].
void
string_insert_slice(String* s, sz idx, Slice slice);

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
string_remove(String* s, sz idx);

//! ## String file api ##
String
read_file_tostring(char* name);

void
write_file_wstring(char* name, String data);

#ifdef TYTO_IMPL

//! ## Arena ##
static Arena*
_arena_new(sz size)
{
    Arena* arena = xcalloc(1, Arena);
    if (!arena)
        return NULL;
    arena->region   = xcalloc(size, u8);
    arena->total_sz = size;
    if (!arena->region) {
        free(arena);
        return NULL;
    }
    return arena;
}

Arena*
arena_new()
{
    return _arena_new(ARENA_PAGE_SIZE);
}

void*
arena_alloc(Arena* a, sz size)
{
    Arena* last = a;

    do {
        if ((a->total_sz - a->current_sz) >= size) {
            a->current_sz += size;
            return a->region + (a->current_sz - size);
        }
        last = a;
    } while ((a = a->next) != NULL);

    sz     asize     = (size > ARENA_PAGE_SIZE) ? size : ARENA_PAGE_SIZE;
    Arena* next      = _arena_new(asize);
    last->next       = next;
    next->current_sz = size;
    return next->region;
}

void
arena_free(Arena* a)
{
    Arena* next = {0};
    Arena* last = a;

    do {
        next = last->next;
        free(last->region);
        free(last);
        last = next;
    } while (next != NULL);
}

//! ## Slice ##

Slice
slice_empty()
{
    return (Slice){.buffer = "", .size = 0};
}

Slice
slice_make(const char* buffer, sz size)
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
    return slice_make(s, (sz)(e - s));
}

Slice
slice_cstring(const char* cstring)
{
    return slice_make(cstring, strlen(cstring));
}

sz
slice_size(Slice s)
{
    return s.size;
}

const char*
slice_raw(Slice s)
{
    return s.buffer;
}

const char*
slice_ref(Slice s, sz idx)
{
    return &s.buffer[idx];
}

char
slice_at(Slice s, sz idx)
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
    bool ret = false;
    sz   i   = 0;
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
slice_split_n(Slice s, char delim, sz n, Slice* pre, Slice* post)
{
    bool ret = false;
    sz   i   = 0;
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
    sz i = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            return true;
    }
    return false;
}

bool
slice_find(Slice s, char c, sz* idx)
{
    sz i = 0;
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
slice_rfind(Slice s, char c, sz* idx)
{
    sz i = 0;
    for (i = s.size; i > 0; i--) {
        if (s.buffer[i - 1] == c) {
            if (idx)
                *idx = i - 1;
            return true;
        }
    }
    return false;
}

sz
slice_count(Slice s, char c)
{
    sz i     = 0;
    sz count = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            count++;
    }
    return count;
}

bool
slice_starts_with(Slice s, Slice prefix)
{
    sz i = 0;
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
    sz i = 0;
    sz j = 0;
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
string_grow_to(String* s, sz cap);

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
    sz     len = strlen(buf);
    String s   = (String){.size     = len,
                          .capacity = len,
                          .buffer   = xmalloc_impl(s.capacity * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    memmove(s.buffer, buf, s.size);
    return s;
}

String
string_make(sz cap)
{
    String s = (String){
        .size = 0, .capacity = cap, .buffer = xmalloc_impl(cap * sizeof(char))};
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
        .buffer   = xmalloc_impl(sl.size * sizeof(char)),
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
        .buffer   = xmalloc_impl(s.capacity * sizeof(char)),
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

sz
string_size(String s)
{
    return s.size;
}

sz
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
string_ref(String s, sz idx);

void
string_insert(String* s, sz idx, char c)
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
string_insert_slice(String* s, sz idx, Slice v)
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
string_remove(String* s, sz idx)
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
        s->capacity = INITIAL_CAPACITY;
    } else {
        s->capacity *= GROWTH_FACTOR;
    }
    s->buffer = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static void
string_grow_to(String* s, sz cap)
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

    char* s       = xmalloc(size + 1, char);
    long  sz_read = fread(s, 1, size, f);

    panic_if(sz_read < size && feof(f) == 0, "Cannot read %s to end.\n", name);

    s[sz_read] = '\0';

    fclose(f);
    return string_slice((Slice){.buffer = s, .size = sz_read});
}

void
write_file_wstring(char* name, String data)
{
    ensure(name);

    FILE* f = fopen(name, "w");
    panic_if(f == NULL, "Cannot open %s", name);

    sz bytes_written = fwrite(data.buffer, 1, data.size, f);
    fclose(f);
    panic_if(bytes_written != data.size, "Cannot write data to file %s.", name);
}

#endif  // TYTO_IMPL
#endif  // TYTO_PROTO_H_

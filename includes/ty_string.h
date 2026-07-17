#ifndef TY_STRING_H_
#define TY_STRING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ty_assert.h>

//! === struct Slice ===
//! Just a buffer + size.
//! Stack allocated.
struct Slice
{
    const char* buffer;
    size_t      size;
};

//! Create a [size] 0 slice.
static inline struct Slice
slice_empty();

//! Create a slice from [cstring] and [size]
static inline struct Slice
slice_make(const char* buffer, size_t size);

//! Create a slice from [start] to [end] of cstring.
struct Slice
slice_range(const char* start, const char* end);

//! Create a slice from [cstring]
//! 	@panic_if [cstring] is NULL.
struct Slice
slice_cstring(const char* cstring);

//! Return size of the slice.
static inline size_t
slice_size(struct Slice s);

//! Return pointer to underlying buffer.
static inline const char*
slice_raw(struct Slice s);

//! Return pointer to char of underlying buffer.
static inline const char*
slice_ref(struct Slice s, size_t idx);

//! Get char at [idx]-th char in [s].
//! @panic_if idx <= [s.size]
static inline char
slice_at(struct Slice s, size_t idx);

//! Compare slices.
//! 1  ? [s1] > [s2] :
//! 0  ? [s1] == [s2] :
//! -1 ? [s1] < [s2] ;
int
slice_cmp(struct Slice s1, struct Slice s2);

//! Return true if [s1] == [s2]
bool
slice_eq(struct Slice s1, struct Slice s2);

//!  Splits [s] into part before [delim] and part after [delim]
//!  If [delim] not found, [pre] gets whole [s] string and [post]
//!  gets assigned the empty string starting at the first character
//!  after the end of s.
bool
slice_split(struct Slice s, char delim, struct Slice* pre, struct Slice* post);

//! Same as [slice_split] however checks at most [n] characters.
bool
slice_split_n(
    struct Slice  s,
    char          delim,
    size_t        n,
    struct Slice* pre,
    struct Slice* post);

//! Predicate on [s.size] and [s.buffer]
bool
slice_is_empty(struct Slice s);

//! Predicate on [s.buffer] for [c] char.
//! Complexity: TODO
bool
slice_has(struct Slice s, char c);

//! Find [c] char in [s] setting [idx] to where it was found.
//! Complexity: TODO
bool
slice_find(struct Slice s, char c, size_t* idx);

//! Find [c] char in [s] setting [idx], but from the end.
//! Complexity: TODO
bool
slice_rfind(struct Slice s, char c, size_t* idx);

//! Count [c] chars in [s].
size_t
slice_count(struct Slice s, char c);

//! Predicate on [s.buffer] starting with [prefix.buffer]
bool
slice_starts_with(struct Slice s, struct Slice prefix);

//! Predicate on [s.buffer] ending with [postfix.buffer]
bool
slice_ends_with(struct Slice s, struct Slice postfix);

//! @struct String
//! A Heap allocated struct String.
//! Ptr + size + capacity
const size_t STRING_INITIAL_CAPACITY = 8;
const size_t STRING_GROWTH_FACTOR    = 2;

struct String
{
    char*  buffer;
    size_t size;
    size_t capacity;
};

//! Creates an empty struct String.
struct String
string_empty();

//! Create struct String from cstring.
struct String
string_cstring(const char* buf);

//! Create struct String from [cap].
struct String
string_make(size_t cap);

//! Create struct String from [slice]
struct String
string_slice(struct Slice slice);

//! Copy [src] string and return the copy.
struct String
string_copy(struct String src);

//! Move [s] to the returned string.
struct String
string_move(struct String* s);

//! Create a struct Slice from [s].
struct Slice
string_toslice(struct String s);

//! Frees memory allocated for [s].
void
string_free(struct String* s);

//! Get [s.size].
size_t
string_size(struct String s);

//! Get [s.cap].
size_t
string_cap(struct String s);

//! Predicate on [s.size].
bool
string_is_empty(struct String s);

//! Predicate on [s.buffer] if null.
bool
string_is_null(struct String s);

//! Compare strings.
int
string_cmp(struct String s1, struct String s2);

//! Check if string is eq.
bool
string_eq(struct String s1, struct String s2);

//! Returns [idx]-th char in [s].
char
string_at(struct String s, size_t idx);

//! Returns pointer to [idx]-th char in [s].
char*
string_ref(struct String s, size_t idx);

//! Insert [c] into the [idx]-th position of [s].
void
string_insert(struct String* s, size_t idx, char c);

//! Insert copy of [slice] into [idx]-th position of [s].
void
string_insert_slice(struct String* s, size_t idx, struct Slice slice);

//! Push [c] to the end of [s].
void
string_push(struct String* s, char c);

//! Push copy of [v] to end of [s].
void
string_push_slice(struct String* s, struct Slice sl);

//! Pops and returns the last character in [s].
char
string_pop(struct String* s);

//! Removes and returns the [idx]-th char in [s].
char
string_remove(struct String* s, size_t idx);

//! @struct StringFile
struct String
read_file_tostring(char* name);

void
write_file_wstring(char* name, struct String data);

#endif  // TY_STRING_H_

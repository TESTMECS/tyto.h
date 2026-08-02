//! @file ty_string.h
//! 	Slice -> Ptr + Size;
//!   String -> Ptr + Size + Capacity;
#ifndef TY_STRING_H_
#define TY_STRING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ty_assert.h>

//! Stack allocated.
typedef struct ty_slice
{
    const char* buffer;
    size_t      size;
} ty_slice_t;

//! Create a [size] 0 slice.
static inline ty_slice_t
slice_empty();

//! Create a slice from [cstring] and [size]
static inline ty_slice_t
slice_make(const char* buffer, size_t size);

//! Create a slice from [start] to [end] of cstring.
ty_slice_t
slice_range(const char* start, const char* end);

//! Create a slice from [cstring]
//! 	@panic_if [cstring] is NULL.
ty_slice_t
slice_cstring(const char* cstring);

//! Return size of the slice.
static inline size_t
slice_size(ty_slice_t s);

//! Return pointer to underlying buffer.
static inline const char*
slice_raw(ty_slice_t s);

//! Return pointer to char of underlying buffer.
static inline const char*
slice_ref(ty_slice_t s, size_t idx);

//! Get char at [idx]-th char in [s].
//! @panic_if idx <= [s.size]
static inline char
slice_at(ty_slice_t s, size_t idx);

//! Compare slices.
//! 1  ? [s1] > [s2] :
//! 0  ? [s1] == [s2] :
//! -1 ? [s1] < [s2] ;
int
slice_cmp(ty_slice_t s1, ty_slice_t s2);

//! Return true if [s1] == [s2]
bool
slice_eq(ty_slice_t s1, ty_slice_t s2);

//!  Splits [s] into part before [delim] and part after [delim]
//!  If [delim] not found, [pre] gets whole [s] string and [post]
//!  gets assigned the empty string starting at the first character
//!  after the end of s.
bool
slice_split(ty_slice_t s, char delim, ty_slice_t* pre, ty_slice_t* post);

//! Same as [slice_split] however checks at most [n] characters.
bool
slice_split_n(
    ty_slice_t  s,
    char        delim,
    size_t      n,
    ty_slice_t* pre,
    ty_slice_t* post);

//! Predicate on [s.size] and [s.buffer]
bool
slice_is_empty(ty_slice_t s);

//! Predicate on [s.buffer] for [c] char.
//! Complexity: TODO
bool
slice_has(ty_slice_t s, char c);

//! Find [c] char in [s] setting [idx] to where it was found.
//! Complexity: TODO
bool
slice_find(ty_slice_t s, char c, size_t* idx);

//! Find [c] char in [s] setting [idx], but from the end.
//! Complexity: TODO
bool
slice_rfind(ty_slice_t s, char c, size_t* idx);

//! Count [c] chars in [s].
size_t
slice_count(ty_slice_t s, char c);

//! Predicate on [s.buffer] starting with [prefix.buffer]
bool
slice_starts_with(ty_slice_t s, ty_slice_t prefix);

//! Predicate on [s.buffer] ending with [postfix.buffer]
bool
slice_ends_with(ty_slice_t s, ty_slice_t postfix);

//! === ty_string_t ===
//! A Heap allocated String -> Ptr + size + capacity.
const size_t TY_STRING_INITIAL_CAPACITY = 8;
const size_t TY_STRING_GROWTH_FACTOR    = 2;

typedef struct ty_string
{
    char*  buffer;
    size_t size;
    size_t capacity;
} ty_string_t;

//! Creates an empty ty_string_t.
ty_string_t
string_empty();

//! Create ty_string_t from cstring.
ty_string_t
string_cstring(const char* buf);

//! Create ty_string_t from [cap].
ty_string_t
string_make(size_t cap);

//! Create ty_string_t from [slice]
ty_string_t
string_slice(ty_slice_t slice);

//! Copy [src] string and return the copy.
ty_string_t
string_copy(ty_string_t src);

//! Move [s] to the returned string.
ty_string_t
string_move(ty_string_t* s);

//! Create a ty_slice_t from [s].
ty_slice_t
string_toslice(ty_string_t s);

//! Frees memory allocated for [s].
void
string_free(ty_string_t* s);

//! Get [s.size].
size_t
string_size(ty_string_t s);

//! Get [s.cap].
size_t
string_cap(ty_string_t s);

//! Predicate on [s.size].
bool
string_is_empty(ty_string_t s);

//! Predicate on [s.buffer] if null.
bool
string_is_null(ty_string_t s);

//! Compare strings.
int
string_cmp(ty_string_t s1, ty_string_t s2);

//! Check if string is eq.
bool
string_eq(ty_string_t s1, ty_string_t s2);

//! Returns [idx]-th char in [s].
char
string_at(ty_string_t s, size_t idx);

//! Returns pointer to [idx]-th char in [s].
char*
string_ref(ty_string_t s, size_t idx);

//! Insert [c] into the [idx]-th position of [s].
void
string_insert(ty_string_t* s, size_t idx, char c);

//! Insert copy of [slice] into [idx]-th position of [s].
void
string_insert_slice(ty_string_t* s, size_t idx, ty_slice_t slice);

//! Push [c] to the end of [s].
void
string_push(ty_string_t* s, char c);

//! Push copy of [v] to end of [s].
void
string_push_slice(ty_string_t* s, ty_slice_t sl);

//! Pops and returns the last character in [s].
char
string_pop(ty_string_t* s);

//! Removes and returns the [idx]-th char in [s].
char
string_remove(ty_string_t* s, size_t idx);

//! @ty_string_tFile
ty_string_t
read_file_tostring(char* name);

void
write_file_wstring(char* name, ty_string_t data);

#endif  // TY_STRING_H_

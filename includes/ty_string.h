#ifndef TY_STRING_H_
#define TY_STRING_H_

#include "./ty_assert.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! === Slice ===
//! Just a buffer + size.
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

#endif  // TY_STRING_H_

//! @file tyto.c
//! Implementation for the tyto library.
#include "tyto.proto.h"

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
                          .buffer   = zmalloc(s.capacity * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    memmove(s.buffer, buf, s.size);
    return s;
}

String
string_make(sz cap)
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

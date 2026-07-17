#include <ty_string.h>

//! @struct Slice
static inline struct Slice
slice_empty()
{
    return (struct Slice){.buffer = "", .size = 0};
}

static inline struct Slice
slice_make(const char* buffer, size_t size)
{
    return (struct Slice){.buffer = buffer, .size = size};
}

struct Slice
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

struct Slice
slice_cstring(const char* cstring)
{
    return slice_make(cstring, strlen(cstring));
}

static inline size_t
slice_size(struct Slice s)
{
    return s.size;
}

static inline const char*
slice_raw(struct Slice s)
{
    return s.buffer;
}

static inline const char*
slice_ref(struct Slice s, size_t idx)
{
    return &s.buffer[idx];
}

static inline char
slice_at(struct Slice s, size_t idx)
{
    return s.buffer[idx];
}

int
slice_cmp(struct Slice s1, struct Slice s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size > s2.size ? s2.size : s1.size);
}

bool
slice_eq(struct Slice s1, struct Slice s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

bool
slice_split(struct Slice s, char delim, struct Slice* pre, struct Slice* post)
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
slice_split_n(
    struct Slice  s,
    char          delim,
    size_t        n,
    struct Slice* pre,
    struct Slice* post)
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
slice_is_empty(struct Slice s)
{
    return s.size == 0;
}

bool
slice_has(struct Slice s, char c)
{
    size_t i = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            return true;
    }
    return false;
}

bool
slice_find(struct Slice s, char c, size_t* idx)
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
slice_rfind(struct Slice s, char c, size_t* idx)
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
slice_count(struct Slice s, char c)
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
slice_starts_with(struct Slice s, struct Slice prefix)
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
slice_ends_with(struct Slice s, struct Slice postfix)
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
//! ## struct String ##
static void
string_grow(struct String* s);

static void
string_grow_to(struct String* s, size_t cap);

static struct String
string_null();

struct String
string_empty()
{
    return (struct String){.size = 0, .capacity = 0, .buffer = NULL};
}

struct String
string_cstring(const char* buf)
{
    size_t        len = strlen(buf);
    struct String s =
        (struct String){.size     = len,
                        .capacity = len,
                        .buffer   = malloc(s.capacity * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    memmove(s.buffer, buf, s.size);
    return s;
}

struct String
string_make(size_t cap)
{
    struct String s = (struct String){
        .size = 0, .capacity = cap, .buffer = malloc(cap * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    return s;
}

struct String
string_slice(struct Slice sl)
{
    struct String s = (struct String){
        .size     = sl.size,
        .capacity = sl.size,
        .buffer   = malloc(sl.size * sizeof(char)),
    };
    if (s.buffer == NULL)
        return string_null();

    memmove(s.buffer, sl.buffer, s.size);

    return s;
}

struct String
string_copy(struct String s)
{
    struct String str = (struct String){
        .size     = s.size,
        .capacity = s.capacity,
        .buffer   = malloc(s.capacity * sizeof(char)),
    };
    if (str.buffer == NULL)
        return string_null();
    memmove(str.buffer, s.buffer, str.size);
    return str;
}

struct String
string_move(struct String* s)
{
    struct String str = {0};
    str               = *s;
    *s                = string_empty();
    return str;
}

struct Slice
string_toslice(struct String s)
{
    return slice_make(s.buffer, s.size);
}

void
string_free(struct String* s)
{
    free(s->buffer);
    *s = string_empty();
}

size_t
string_size(struct String s)
{
    return s.size;
}

size_t
string_cap(struct String s)
{
    return s.capacity;
}

bool
string_is_empty(struct String s)
{
    return s.buffer == NULL && s.size == 0;
}

bool
string_is_null(struct String s)
{
    return s.buffer == NULL && s.size == SIZE_MAX && s.capacity == SIZE_MAX;
}

int
string_cmp(struct String s1, struct String s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size < s2.size ? s1.size : s2.size);
}

bool
string_eq(struct String s1, struct String s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

char
string_at(struct String s, size_t idx);

char*
string_ref(struct String s, size_t idx);

void
string_insert(struct String* s, size_t idx, char c)
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
string_insert_slice(struct String* s, size_t idx, struct Slice v)
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
string_push(struct String* s, char c)
{
    if (s->size == s->capacity) {
        string_grow(s);
        if (string_is_null(*s))
            return;
    }
    s->buffer[s->size++] = c;
}

void
string_push_slice(struct String* s, struct Slice sl)
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
string_pop(struct String* s)
{
    return s->buffer[--s->size];
}

char
string_remove(struct String* s, size_t idx)
{
    char c = s->buffer[idx];

    memmove(&s->buffer[idx], &s->buffer[idx + 1], s->size - idx - 1);
    s->size--;
    return c;
}

static void
string_grow(struct String* s)
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
string_grow_to(struct String* s, size_t cap)
{
    s->capacity = cap;
    s->buffer   = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static struct String
string_null()
{
    return (struct String){
        .size = SIZE_MAX, .capacity = SIZE_MAX, .buffer = NULL};
}

struct String
read_file_tostring(char* name)
{
    ensure(name);

    FILE* f = fopen(name, "r");
    panic_if(f == NULL, "Cannot open %s", name);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* s           = (char*)malloc(size + 1);
    long  size_t_read = fread(s, 1, size, f);

    panic_if(
        size_t_read < size && feof(f) == 0, "Cannot read %s to end.\n", name);

    s[size_t_read] = '\0';

    fclose(f);
    return string_slice((struct Slice){.buffer = s, .size = size_t_read});
}

void
write_file_wstring(char* name, struct String data)
{
    ensure(name);

    FILE* f = fopen(name, "w");
    panic_if(f == NULL, "Cannot open %s", name);

    size_t bytes_written = fwrite(data.buffer, 1, data.size, f);
    fclose(f);
    panic_if(bytes_written != data.size, "Cannot write data to file %s.", name);
}

#include <ty_string.h>

static inline ty_slice_t
slice_empty()
{
    return (ty_slice_t){.buffer = "", .size = 0};
}

static inline ty_slice_t
slice_make(const char* buffer, size_t size)
{
    return (ty_slice_t){.buffer = buffer, .size = size};
}

ty_slice_t
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

ty_slice_t
slice_cstring(const char* cstring)
{
    return slice_make(cstring, strlen(cstring));
}

static inline size_t
slice_size(ty_slice_t s)
{
    return s.size;
}

static inline const char*
slice_raw(ty_slice_t s)
{
    return s.buffer;
}

static inline const char*
slice_ref(ty_slice_t s, size_t idx)
{
    return &s.buffer[idx];
}

static inline char
slice_at(ty_slice_t s, size_t idx)
{
    return s.buffer[idx];
}

int
slice_cmp(ty_slice_t s1, ty_slice_t s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size > s2.size ? s2.size : s1.size);
}

bool
slice_eq(ty_slice_t s1, ty_slice_t s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

bool
slice_split(ty_slice_t s, char delim, ty_slice_t* pre, ty_slice_t* post)
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
    ty_slice_t  s,
    char        delim,
    size_t      n,
    ty_slice_t* pre,
    ty_slice_t* post)
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
slice_is_empty(ty_slice_t s)
{
    return s.size == 0;
}

bool
slice_has(ty_slice_t s, char c)
{
    size_t i = 0;
    for (i = 0; i < s.size; i++) {
        if (s.buffer[i] == c)
            return true;
    }
    return false;
}

bool
slice_find(ty_slice_t s, char c, size_t* idx)
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
slice_rfind(ty_slice_t s, char c, size_t* idx)
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
slice_count(ty_slice_t s, char c)
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
slice_starts_with(ty_slice_t s, ty_slice_t prefix)
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
slice_ends_with(ty_slice_t s, ty_slice_t postfix)
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
//! ## ty_string_t ##
static void
string_grow(ty_string_t* s);

static void
string_grow_to(ty_string_t* s, size_t cap);

static ty_string_t
string_null();

ty_string_t
string_empty()
{
    return (ty_string_t){.size = 0, .capacity = 0, .buffer = NULL};
}

ty_string_t
string_cstring(const char* buf)
{
    size_t      len = strlen(buf);
    ty_string_t s   = (ty_string_t){.size     = len,
                                    .capacity = len,
                                    .buffer = malloc(s.capacity * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    memmove(s.buffer, buf, s.size);
    return s;
}

ty_string_t
string_make(size_t cap)
{
    ty_string_t s = (ty_string_t){
        .size = 0, .capacity = cap, .buffer = malloc(cap * sizeof(char))};
    if (s.buffer == NULL)
        return string_null();
    return s;
}

ty_string_t
string_slice(ty_slice_t sl)
{
    ty_string_t s = (ty_string_t){
        .size     = sl.size,
        .capacity = sl.size,
        .buffer   = malloc(sl.size * sizeof(char)),
    };
    if (s.buffer == NULL)
        return string_null();

    memmove(s.buffer, sl.buffer, s.size);

    return s;
}

ty_string_t
string_copy(ty_string_t s)
{
    ty_string_t str = (ty_string_t){
        .size     = s.size,
        .capacity = s.capacity,
        .buffer   = malloc(s.capacity * sizeof(char)),
    };
    if (str.buffer == NULL)
        return string_null();
    memmove(str.buffer, s.buffer, str.size);
    return str;
}

ty_string_t
string_move(ty_string_t* s)
{
    ty_string_t str = {0};
    str             = *s;
    *s              = string_empty();
    return str;
}

ty_slice_t
string_toslice(ty_string_t s)
{
    return slice_make(s.buffer, s.size);
}

void
string_free(ty_string_t* s)
{
    free(s->buffer);
    *s = string_empty();
}

size_t
string_size(ty_string_t s)
{
    return s.size;
}

size_t
string_cap(ty_string_t s)
{
    return s.capacity;
}

bool
string_is_empty(ty_string_t s)
{
    return s.buffer == NULL && s.size == 0;
}

bool
string_is_null(ty_string_t s)
{
    return s.buffer == NULL && s.size == SIZE_MAX && s.capacity == SIZE_MAX;
}

int
string_cmp(ty_string_t s1, ty_string_t s2)
{
    return strncmp(s1.buffer, s2.buffer, s1.size < s2.size ? s1.size : s2.size);
}

bool
string_eq(ty_string_t s1, ty_string_t s2)
{
    return s1.size == s2.size && strncmp(s1.buffer, s2.buffer, s1.size) == 0;
}

char
string_at(ty_string_t s, size_t idx);

char*
string_ref(ty_string_t s, size_t idx);

void
string_insert(ty_string_t* s, size_t idx, char c)
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
string_insert_slice(ty_string_t* s, size_t idx, ty_slice_t v)
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
string_push(ty_string_t* s, char c)
{
    if (s->size == s->capacity) {
        string_grow(s);
        if (string_is_null(*s))
            return;
    }
    s->buffer[s->size++] = c;
}

void
string_push_slice(ty_string_t* s, ty_slice_t sl)
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
string_pop(ty_string_t* s)
{
    return s->buffer[--s->size];
}

char
string_remove(ty_string_t* s, size_t idx)
{
    char c = s->buffer[idx];

    memmove(&s->buffer[idx], &s->buffer[idx + 1], s->size - idx - 1);
    s->size--;
    return c;
}

static void
string_grow(ty_string_t* s)
{
    if (s->capacity == 0) {
        s->capacity = TY_STRING_INITIAL_CAPACITY;
    } else {
        s->capacity *= TY_STRING_GROWTH_FACTOR;
    }
    s->buffer = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static void
string_grow_to(ty_string_t* s, size_t cap)
{
    s->capacity = cap;
    s->buffer   = realloc(s->buffer, s->capacity * sizeof(char));
    if (s->buffer == NULL)
        *s = string_null();
}

static ty_string_t
string_null()
{
    return (ty_string_t){
        .size = SIZE_MAX, .capacity = SIZE_MAX, .buffer = NULL};
}

ty_string_t
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
    return string_slice((ty_slice_t){.buffer = s, .size = size_t_read});
}

void
write_file_wstring(char* name, ty_string_t data)
{
    ensure(name);

    FILE* f = fopen(name, "w");
    panic_if(f == NULL, "Cannot open %s", name);

    size_t bytes_written = fwrite(data.buffer, 1, data.size, f);
    fclose(f);
    panic_if(bytes_written != data.size, "Cannot write data to file %s.", name);
}

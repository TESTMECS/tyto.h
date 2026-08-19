#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TY_ALLOC_IMPL
#include <ty_alloc.h>

#define TEST(name) static void test_##name(void)

#define RUN(name)                                                              \
    do {                                                                       \
        printf(" %-42s", #name);                                               \
        fflush(stdout);                                                        \
        test_##name();                                                         \
        puts("OK");                                                            \
    } while (0)

static void
fill_bytes(void* p, size_t size, unsigned char value)
{
    memset(p, value, size);
}

static void
check_bytes(const void* p, size_t size, unsigned char value)
{
    const unsigned char* ptr = p;
    for (size_t i = 0; i < size; ++i)
        assert(ptr[i] == value);
}

TEST(xmalloc_basic)
{
    int* p = xmalloc(100);
    assert(p != NULL);

    for (size_t i = 0; i < 100; ++i)
        p[i] = (int)i;

    for (size_t i = 0; i < 100; ++i)
        assert(p[i] == (int)i);

    free(p);
}

TEST(xcalloc_zeros_memory)
{
    uint64_t* p = xcalloc(128, sizeof(uint64_t));
    assert(p != NULL);

    for (size_t i = 0; i < 128; ++i)
        assert(p[i] == 0);

    free(p);
}

TEST(xrealloc_grows_preserving_data)
{
    int* p = xmalloc(16);

    for (size_t i = 0; i < 16; ++i)
        p[i] = (int)(i * 7);

    p = xrealloc(p, 1024, sizeof(int));

    for (size_t i = 0; i < 16; ++i)
        assert(p[i] == (int)(i * 7));

    free(p);
}

TEST(xrealloc_shrinks_preserving_prefix)
{
    unsigned char* p = xmalloc(1024);
    for (size_t i = 0; i < 1024; ++i)
        p[i] = (unsigned char)i;
    p = xrealloc(p, 128, sizeof(unsigned char));
    for (size_t i = 0; i < 128; ++i)
        assert(p[i] == (unsigned char)i);
    free(p);
}

TEST(region_init)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    assert(region.data == storage);
    assert(region.capacity == sizeof(storage));
    assert(region.size == 0);
}

TEST(region_allocates)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    void* a = region_alloc(&region, 32);
    void* b = region_alloc(&region, 64);

    assert(a != NULL);
    assert(b != NULL);
    assert(a != b);

    assert(region_is_owned(&region, a));
    assert(region_is_owned(&region, b));

    fill_bytes(a, 32, 0xaa);
    fill_bytes(b, 64, 0xbb);

    check_bytes(a, 32, 0xaa);
    check_bytes(b, 64, 0xbb);
}

TEST(region_alignment)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    void*  ptrs[64];
    size_t sizes[64];

    for (size_t i = 0; i < 64; ++i) {
        sizes[i] = i + 1;
        ptrs[i]  = region_alloc(&region, sizes[i]);
        assert(ptrs[i] != NULL);
    }

    for (size_t i = 0; i < 64; ++i) {
        for (size_t j = i + 1; j < 64; ++j) {
            uintptr_t a0 = (uintptr_t)ptrs[i];
            uintptr_t a1 = a0 + sizes[i];
            uintptr_t b0 = (uintptr_t)ptrs[j];
            uintptr_t b1 = b0 + sizes[j];
            assert(a1 <= b0 || b1 <= a0);
        }
    }
}

TEST(region_buffer_size)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    void* a = region_alloc(&region, 37);
    void* b = region_alloc(&region, 91);

    assert(region_buffer_size(a) == 37);
    assert(region_buffer_size(b) == 91);
}

TEST(region_free_last_alloc)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    void* a = region_alloc(&region, 32);
    void* b = region_alloc(&region, 64);

    assert(a && b);

    size_t before = region.size;
    region_free(&region, b);

    assert(region.size < before);

    void* c = region_alloc(&region, 64);
    assert(c != NULL);
    assert(c == b);
}

TEST(region_free_non_last_does_not_corrupt)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    void* a = region_alloc(&region, 32);
    void* b = region_alloc(&region, 32);

    fill_bytes(a, 32, 0x11);
    fill_bytes(b, 32, 0x22);

    region_free(&region, a);

    check_bytes(b, 32, 0x22);

    void* c = region_alloc(&region, 32);
    assert(c != NULL);
    check_bytes(b, 32, 0x22);
}

TEST(region_remap_last_grows)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    unsigned char* p = region_alloc(&region, 32);
    assert(p != NULL);

    fill_bytes(p, 32, 0x5a);

    unsigned char* q = region_remap(&region, p, 128);
    assert(q != NULL);

    check_bytes(q, 32, 0x5a);
}

TEST(region_remap_last_shrinks)
{
    unsigned char storage[4096];
    region_t      region = region_init(storage, sizeof(storage));

    unsigned char* p = region_alloc(&region, 256);
    assert(p != NULL);

    fill_bytes(p, 256, 0x5a);

    unsigned char* q = region_remap(&region, p, 32);
    assert(q != NULL);

    check_bytes(q, 32, 0x5a);
}

TEST(region_exhaustion)
{
    unsigned char storage[128];
    region_t      region = region_init(storage, sizeof(storage));

    size_t count = 0;
    while (region_alloc(&region, 16) != NULL)
        ++count;

    assert(count > 0);
    assert(region.size <= region.capacity);
}

TEST(region_allocator_adapter)
{
    unsigned char storage[4096];
    region_t      region    = region_init(storage, sizeof(storage));
    allocator_t*  allocator = region_new(&region);

    assert(allocator != NULL);

    void* a = allocator->alloc(allocator, 64);
    assert(a != NULL);

    fill_bytes(a, 64, 0xcc);
    check_bytes(a, 64, 0xcc);

    allocator->free(allocator, a);
}

TEST(annex_uses_region_before_fallback)
{
    unsigned char storage[1024];
    unsigned char fallback_storage[4096];
    region_t      fallback_region =
        region_init(fallback_storage, sizeof(fallback_storage));

    allocator_t* fallback = region_new(&fallback_region);
    annex_t      annex    = annex_init(storage, sizeof(storage), fallback);

    void* p = annex_alloc(&annex, 32);
    assert(p != NULL);
    assert(region_is_owned(&annex.allocator, p));
}

TEST(annex_falls_back_when_region_is_full)
{
    unsigned char storage[64];
    unsigned char fallback_storage[4096];
    region_t      fallback_region =
        region_init(fallback_storage, sizeof(fallback_storage));
    allocator_t* fallback = region_new(&fallback_region);
    annex_t      annex    = annex_init(storage, sizeof(storage), fallback);
    void*        a        = annex_alloc(&annex, 32);
    void*        b        = annex_alloc(&annex, 1024);
    assert(a != NULL);
    assert(b != NULL);
    assert(region_is_owned(&annex.allocator, a));
    assert(!region_is_owned(&annex.allocator, b));
}

TEST(annex_free_handles_both_backends)
{
    unsigned char storage[256];
    unsigned char fallback_storage[4096];
    region_t      fallback_region =
        region_init(fallback_storage, sizeof(fallback_storage));
    allocator_t* fallback = region_new(&fallback_region);
    annex_t      annex    = annex_init(storage, sizeof(storage), fallback);
    void*        a        = annex_alloc(&annex, 32);
    void*        b        = annex_alloc(&annex, 2048);
    assert(a && b);
    annex_free(&annex, a);
    annex_free(&annex, b);
}

TEST(annex_adapter)
{
    unsigned char storage[1024];
    unsigned char fallback_storage[4096];
    region_t      fallback_region =
        region_init(fallback_storage, sizeof(fallback_storage));
    allocator_t* fallback  = region_new(&fallback_region);
    annex_t      annex     = annex_init(storage, sizeof(storage), fallback);
    allocator_t* allocator = annex_new(&annex);
    void*        p         = allocator->alloc(allocator, 64);
    assert(p != NULL);
    allocator->free(allocator, p);
}

TEST(arena_init)
{
    unsigned char backing[4096];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);
    arena_t       arena;
    arena.allocator        = allocator;
    arena.first_block_size = 64;
    assert(arena.blocks == NULL);
    assert(arena.current_block == NULL);
    assert(arena.last_block == NULL);
    arena_deinit(&arena);
}

TEST(arena_allocates)
{
    unsigned char backing[16384];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    arena_t arena;
    arena.allocator        = allocator;
    arena.first_block_size = 64;

    void* a = arena_alloc(&arena, 16);
    void* b = arena_alloc(&arena, 32);
    void* c = arena_alloc(&arena, 128);

    assert(a && b && c);
    assert(a != b);
    assert(b != c);
    assert(c != a);

    fill_bytes(a, 16, 0x11);
    fill_bytes(b, 32, 0x22);
    fill_bytes(c, 128, 0x33);

    check_bytes(a, 16, 0x11);
    check_bytes(b, 32, 0x22);
    check_bytes(c, 128, 0x33);

    arena_deinit(&arena);
}

TEST(arena_grows_blocks)
{
    unsigned char backing[65536];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    arena_t arena;
    arena.allocator        = allocator;
    arena.first_block_size = 32;

    for (size_t i = 0; i < 128; ++i)
        assert(arena_alloc(&arena, 64) != NULL);

    assert(arena.blocks != NULL);
    assert(arena.blocks->next != NULL);

    arena_deinit(&arena);
}

TEST(arena_alignment)
{
    unsigned char backing[65536];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);
    arena_t       arena;
    arena.allocator        = allocator;
    arena.first_block_size = 64;
    for (size_t i = 1; i <= 512; ++i) {
        void* p = arena_alloc(&arena, i);
        assert(p != NULL);
        assert((uintptr_t)p % ALLOCATOR_MAX_ALIGNMENT == 0);
    }
    arena_deinit(&arena);
}

TEST(arena_reset)
{
    unsigned char backing[65536];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);
    arena_t       arena;
    arena.allocator        = allocator;
    arena.first_block_size = 128;
    void* p                = arena_alloc(&arena, 64);
    assert(p != NULL);

    arena_block_t* blocks_before = arena.blocks;

    arena_reset(&arena);

    assert(arena.blocks == blocks_before);
    assert(arena.current_block == arena.blocks);
    assert(arena.current_block->size == 0);

    void* q = arena_alloc(&arena, 64);
    assert(q != NULL);
    arena_deinit(&arena);
}

TEST(arena_free_last_allocation)
{
    unsigned char backing[16384];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    arena_t arena;
    arena.allocator        = allocator;
    arena.first_block_size = 128;

    void* a = arena_alloc(&arena, 32);
    void* b = arena_alloc(&arena, 64);

    assert(a && b);

    arena_free(&arena, b);

    void* c = arena_alloc(&arena, 64);
    assert(c == b);

    (void)a;
    arena_deinit(&arena);
}

TEST(arena_remap_last)
{
    unsigned char backing[65536];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);
    arena_t       arena;
    arena.allocator        = allocator;
    arena.first_block_size = 128;
    unsigned char* p       = arena_alloc(&arena, 32);
    assert(p != NULL);

    fill_bytes(p, 32, 0x7e);

    unsigned char* q = arena_remap(&arena, p, 96);
    assert(q != NULL);
    check_bytes(q, 32, 0x7e);

    arena_deinit(&arena);
}

TEST(vector_initialization)
{
    unsigned char backing[4096];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    assert(v.size == 0);
    assert(v.capacity == 0);
    assert(v.data == NULL);
    assert(v.allocator == allocator);
}

TEST(vector_push)
{
    unsigned char backing[16384];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    for (int i = 0; i < 1000; ++i)
        assert(Vector_push(&v, i));

    assert(v.size == 1000);
    assert(v.capacity >= v.size);

    for (size_t i = 0; i < v.size; ++i)
        assert(v.data[i] == (int)i);
}

TEST(vector_pushN)
{
    unsigned char backing[16384];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    int values[] = {1, 2, 3, 5, 8, 13, 21, 34};

    assert(Vector_pushN(&v, values, sizeof(values) / sizeof(values[0])));
    assert(v.size == sizeof(values) / sizeof(values[0]));

    for (size_t i = 0; i < v.size; ++i)
        assert(v.data[i] == values[i]);
}

TEST(vector_pushN_many_times)
{
    unsigned char backing[32768];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    for (size_t round = 0; round < 100; ++round) {
        int values[16];

        for (size_t i = 0; i < 16; ++i)
            values[i] = (int)(round * 16 + i);

        assert(Vector_pushN(&v, values, 16));
    }

    assert(v.size == 1600);

    for (size_t i = 0; i < v.size; ++i)
        assert(v.data[i] == (int)i);
}

TEST(vector_pop)
{
    unsigned char backing[4096];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    for (int i = 0; i < 10; ++i)
        assert(Vector_push(&v, i));

    for (int i = 9; i >= 0; --i) {
        assert(v.data[v.size - 1] == i);
        assert(Vector_pop(&v));
    }

    assert(v.size == 0);
    assert(!Vector_pop(&v));
}

TEST(vector_clear)
{
    unsigned char backing[4096];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    for (int i = 0; i < 100; ++i)
        assert(Vector_push(&v, i));

    size_t capacity = v.capacity;
    Vector_clear(&v);

    assert(v.size == 0);
    assert(v.capacity == capacity);
}

TEST(vector_deinit)
{
    unsigned char backing[16384];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(int) v = Vector_init(allocator);

    for (int i = 0; i < 100; ++i)
        assert(Vector_push(&v, i));

    Vector_deinit(&v);

    assert(v.size == 0);
    assert(v.capacity == 0);
    assert(v.data == NULL);
}

TEST(vector_nontrival_element_type)
{
    typedef struct
    {
        uint64_t      id;
        double        value;
        unsigned char bytes[17];
    } item_t;

    unsigned char backing[32768];
    region_t      region    = region_init(backing, sizeof(backing));
    allocator_t*  allocator = region_new(&region);

    Vector(item_t) v = Vector_init(allocator);

    for (size_t i = 0; i < 128; ++i) {
        item_t item;
        item.id    = i;
        item.value = (double)i * 1.25;
        memset(item.bytes, (int)i, sizeof(item.bytes));
        assert(Vector_push(&v, item));
    }

    for (size_t i = 0; i < v.size; ++i) {
        assert(v.data[i].id == i);
        assert(v.data[i].value == (double)i * 1.25);
        for (size_t j = 0; j < sizeof(v.data[i].bytes); ++j)
            assert(v.data[i].bytes[j] == (unsigned char)i);
    }
}

int
main(void)
{
    puts("=== BEGIN TESTS ===");

    puts("\nBasic allocation:");
    RUN(xmalloc_basic);
    RUN(xcalloc_zeros_memory);
    RUN(xrealloc_grows_preserving_data);
    RUN(xrealloc_shrinks_preserving_prefix);

    puts("\nRegion:");
    RUN(region_init);
    RUN(region_allocates);
    RUN(region_alignment);
    RUN(region_buffer_size);
    RUN(region_free_last_alloc);
    RUN(region_free_non_last_does_not_corrupt);
    RUN(region_remap_last_grows);
    RUN(region_remap_last_shrinks);
    RUN(region_exhaustion);
    RUN(region_allocator_adapter);

    puts("\nAnnex:");
    RUN(annex_uses_region_before_fallback);
    RUN(annex_falls_back_when_region_is_full);
    RUN(annex_free_handles_both_backends);
    RUN(annex_adapter);

    puts("\nArena:");
    RUN(arena_init);
    RUN(arena_allocates);
    RUN(arena_grows_blocks);
    RUN(arena_alignment);
    RUN(arena_reset);
    RUN(arena_free_last_allocation);
    RUN(arena_remap_last);

    puts("\nVector:");
    RUN(vector_initialization);
    RUN(vector_push);
    RUN(vector_pushN);
    RUN(vector_pushN_many_times);
    RUN(vector_pop);
    RUN(vector_clear);
    RUN(vector_deinit);
    RUN(vector_nontrival_element_type);

    puts("=== END TESTS ===");
    putc('\n', stdout);
}

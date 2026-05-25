#define TYTO_IMPL
#include "./tyto.h"

void
test_slice(void)
{
    Slice s = slice_cstring("Hello World!");

    xassert("Should have size 12", slice_size(s) == 12);
    xassert("Should get first char", slice_at(s, 0) == 'H');
    xassert("Should get last char", slice_at(s, slice_size(s) - 1) == '!');
    xassert("Should search for char", slice_has(s, 'W'));
    xassert("Should search from left side", slice_find(s, 'W', NULL));
    xassert("Should search from right side", slice_rfind(s, 'W', NULL));
    xassert(
        "Should check starts with",
        slice_starts_with(s, slice_cstring("Hello")));
    xassert("Should check empty", slice_is_empty(slice_empty()));
    xassert("Should check empty", slice_is_empty(slice_cstring("")));
}

void
test_arena(void)
{
    Arena* a = arena_new();
    char*  x = arena_alloc(a, 10);
    char*  y = arena_alloc(a, 10);
    char*  z = arena_alloc(a, 10);
    arena_free(a);
}

void
test_dynamic_array(void)
{
}

int
main(void)
{
    puts("=== Begin Tests ===");
    test_slice();
    test_arena();
    puts("=== Tests passed! ===");
    return 0;
}

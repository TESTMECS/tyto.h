#include <stddef.h>
#include <stdio.h>

typedef struct
{
    void*  p;
    size_t len;
} array_t;

typedef struct
{
    size_t start;
    size_t end;
    size_t step;
} array_iter_t;

#define ARRAY(TYPE, ...)                                                       \
    (array_t)                                                                  \
    {                                                                          \
        .p   = (TYPE[]){__VA_ARGS__},                                          \
        .len = sizeof((TYPE[]){__VA_ARGS__}) / sizeof(TYPE)                    \
    }

#define ARRAY_ITER_BEGIN(TYPE, array, i, ...)                                  \
    {                                                                          \
        array_iter_t i##_opts = {                                              \
            .start = 0, .end = (array).len, .step = 1, __VA_ARGS__};           \
                                                                               \
        TYPE* i##_data = (TYPE*)(array).p;                                     \
                                                                               \
        for (size_t i = i##_opts.start; i < i##_opts.end;                      \
             i += i##_opts.step) {                                             \
            TYPE* it = &i##_data[i];

#define ARRAY_ITER_END()                                                       \
    }                                                                          \
    }

int
main(void)
{
    array_t arr = ARRAY(int, 10, 20, 30, 40, 50, 60, 70, 80);

    printf("Entire array:\n");
    ARRAY_ITER_BEGIN(int, arr, i)
    {
        printf("i=%zu value=%d\n", i, *it);
    }
    ARRAY_ITER_END();

    printf("\nSubset:\n");
    ARRAY_ITER_BEGIN(int, arr, i, .start = 2, .end = 6)
    {
        printf("i=%zu value=%d\n", i, *it);
    }
    ARRAY_ITER_END();

    printf("\nEvery other element:\n");
    ARRAY_ITER_BEGIN(int, arr, i, .step = 2)
    {
        printf("i=%zu value=%d\n", i, *it);
    }
    ARRAY_ITER_END();

    return 0;
}

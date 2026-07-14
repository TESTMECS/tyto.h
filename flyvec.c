#include "flyvec.h"

// 16 bytes for this one. Two pointers.
// 24 for the struct version. (data, size, capacity).
// fly vector can be generic ie. "int a = ints[1][i];" no casting required.
// Reallocations and capacity must always be power of 2.
// Assumes that (void*) and (uintptr_t) are same size,
// Resize by casting integer to pointer ie. "ints[0] = (void*)3" is techincally
// undefined behavior.
#include <stdint.h>
#include <stdio.h>

int
main(void)
{
    int* ints[2] = {0};
    for (uintptr_t i = 0; i < 5; i++) {
        if (!FLY_PUSH(ints, int, i + 42)) {
            return 1;
        }
    }

    for (uintptr_t i = 0; i < (uintptr_t)ints[0]; i++) {
        printf("%d\n", ints[1][i]);
    }

    ints[0] = (void*)3;
    ints[1] = realloc(ints[1], sizeof(**ints) * 4);

    for (uintptr_t i = 0; i < (uintptr_t)ints[0]; i++) {
        printf("%d\n", ints[1][i]);
    }

    free(ints[1]);

    struct person
    {
        char* name;
        int   age;
    }* people[2] = {0};

    FLY_PUSH(people, struct person, ((struct person){"bob", 21}));
    FLY_PUSH(people, struct person, ((struct person){"jane", 42}));

    for (uintptr_t i = 0; i < (uintptr_t)people[0]; i++) {
        printf("%s: %d\n", people[1][i].name, people[1][i].age);
    }
    free(people[1]);
    return 0;
}

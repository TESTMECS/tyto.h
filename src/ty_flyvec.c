#include "../includes/ty_flyvec.h"

#include <stdint.h>
#include <stdio.h>

int
main(void)
{
    //! create a pointer to [2]int and zero initialize the array.
    int* ints[2] = {0};
    printf("=== ===\n");
    //! have to use `uintptr_t` for the index.
    for (uintptr_t i = 0; i < 5; i++) {
        //! Macro handles resizing.
        if (!FLY_PUSH(ints, int, i + 42)) {
            fprintf(stderr, "[flyvec] Error pushing integer");
            return 1;
        }
    }
    //! Iterate over each integer that we just pushed.
    //! using ints[0] for the length
    for (uintptr_t i = 0; i < (uintptr_t)ints[0]; i++) {
        //! using ints[1][i] for the actual values.
        printf("%d\n", ints[1][i]);
    }
    //! This is technically undefined but we can do it.
    //! Change size to 3.
    ints[0] = (void*)3;
    //! And then resize the actual values in ints[1]
    ints[1] = realloc(ints[1], sizeof(**ints) * 4);
    //! And it hopefully doesn't print garbage!
    printf("=== ===\n");
    for (uintptr_t i = 0; i < (uintptr_t)ints[0]; i++) {
        printf("%d\n", ints[1][i]);
    }
    //! Make sure to clean up the dynamic part!
    free(ints[1]);
    //! Works with structs as well!
    struct person
    {
        char* name;
        int   age;
    }* people[2] = {0};

    FLY_PUSH(people, struct person, ((struct person){"bob", 21}));
    FLY_PUSH(people, struct person, ((struct person){"jane", 42}));
    //! Very same iteration
    printf("=== ===\n");
    for (uintptr_t i = 0; i < (uintptr_t)people[0]; i++) {
        printf("%s: %d\n", people[1][i].name, people[1][i].age);
    }
    //! Make sure to clean up the dynamic part!
    free(people[1]);
    return 0;
}

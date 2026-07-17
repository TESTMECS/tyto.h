//! @module ty_assert.h
//! 	Assertion helpers.
#ifndef TY_ASSERT_H_
#define TY_ASSERT_H_

#include <stdio.h>

//! Concatenation macro.
#define TYTO_CONCAT__(a, b) a##b
#define TYTO_CONCAT_(a, b)  TYTO_CONCAT__(a, b)
#define TYTO_CONCAT(a, b)   TYTO_CONCAT_(a, b)

//! @macro UNIQUE_ID(name)
//! Creates a unique identifier with the line number.
#define UNIQUE_ID(name) TYTO_CONCAT(name, __LINE__)

//! @macro TYTO_STATIC_ASSERT
//! 	C99 static assert, creates a negative sized type if the condition is
//! false.
#define TYTO_STATIC_ASSERT(e)                                                  \
    typedef char TYTO_CONCAT(compile_time_assertion, __LINE__)[(e) ? 1 : -1]

//! === Preconditions ===

//! @macro ensure(ptr)
//! 	check if a [ptr] is null and exit if so.
#define ensure(ptr)                                                            \
    if (ptr == NULL) {                                                         \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: %s's precondition \"not null\" (" #ptr               \
            ") violated\n",                                                    \
            __FILE__,                                                          \
            __LINE__,                                                          \
            __func__);                                                         \
        exit(EXIT_FAILURE);                                                    \
    }

//! @require!(condition, description)
//! 	(similar to assert) requires that [condition] is true.
#define require(condition, description)                                        \
    if (!(condition)) {                                                        \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: %s's precondition \"%s\" (%s) violated\n",           \
            __FILE__,                                                          \
            __LINE__,                                                          \
            __func__,                                                          \
            description,                                                       \
            #condition);                                                       \
        exit(EXIT_FAILURE);                                                    \
    }

//! === Assert ===

//! @macro xassert(condition, description)
//! 	Assert macro with a description.
//! Same as [require(condition, description)].
#define xassert(condition, description)                                        \
    if (!(condition)) {                                                        \
        fprintf(                                                               \
            stderr,                                                            \
            "%s, line %d: assertion \"%s\" (%s) violated\n",                   \
            __FILE__,                                                          \
            __LINE__,                                                          \
            description,                                                       \
            #condition);                                                       \
        exit(EXIT_FAILURE);                                                    \
    }

//! === Panics ===

//! @panic!(message)
//! 	Prints error message and exits the program.
#define panic(message)                                                         \
    {                                                                          \
        fprintf(                                                               \
            stderr, "%s:%d, %s: %s\n", __FILE__, __LINE__, __func__, message); \
        exit(EXIT_FAILURE);                                                    \
    }

//! @panicf!(message)
//! 	Prints error message and exits the program with format args.
#define panicf(...)                                                            \
    {                                                                          \
        fprintf(stderr, "%s:%d, %s: ", __FILE__, __LINE__, __func__);          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }

//! @panic_if!(condition, ...)
//! 	If the condition is true exits the program after printing the format
//! args.
#define panic_if(condition, ...)                                               \
    if (condition) {                                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    }

#endif  // TY_ASSERT_H_

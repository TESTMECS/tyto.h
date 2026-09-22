#ifndef TY_TEST_H_
#define TY_TEST_H_
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TY_TEST_OUTPUT_BUFFER_SIZE 1024

#define TY_TEST_FAILURE_BUFFER_SIZE 1024

typedef enum ty_test_result_kind
{
    TY_TEST_RESULT_PASS = 0,
    TY_TEST_RESULT_FAIL,
    TY_TEST_RESULT_FAIL_ASSERT,
    TY_TEST_RESULT_FAIL_STRING,
    TY_TEST_RESULT_FAIL_NUMBER,
    TY_TEST_RESULT_FAIL_MEMORY,
    TY_TEST_RESULT_FAIL_TIMEOUT,
    TY_TEST_RESULT_ERROR,
    TY_TEST_RESULT_SKIP,
} ty_test_result_kind_t;

typedef struct ty_test
{
    void (*ty_test_function)(void);
    const char* suite;
    const char* skip_because;
    const char* function_name;
    int         timeout;
    bool        should_fail;
} ty_test_t;

typedef struct ty_test_result
{
    ty_test_result_kind_t result;
    int                   exit_code;
    int                   signal;
    long long             duration_ms;
    char                  captured_output[TY_TEST_OUTPUT_BUFFER_SIZE];
    char                  captured_stderr[TY_TEST_OUTPUT_BUFFER_SIZE];
    char                  failure_expression[TY_TEST_FAILURE_BUFFER_SIZE];
    char                  failure_expected[TY_TEST_FAILURE_BUFFER_SIZE];
    char                  failure_got[TY_TEST_FAILURE_BUFFER_SIZE];
    char                  failure_file[TY_TEST_FAILURE_BUFFER_SIZE];
    int                   failure_line;
    int                   timeout;
} ty_test_result_t;

static ty_test_t* __ty_tests;
static int        __ty_testsc;
static int        __ty_testsk;
static bool       __ty_test_should_capture_output;
static int        __ty_test_failure_pipe[2];

//! String concatenation macros.
#define _TYTO_STR(x)    #x
#define _TYTO_STRCAT(x) _TYTO_STR(x)

typedef struct ty_test_optionals
{
    const char* suite;
    const char* skip_because;
    int         timeout;
    bool        should_fail;
} ty_test_optionals_t;

#define ty_test(test_function, ...)                                            \
    _ty_test(                                                                  \
        (test_function),                                                       \
        _TYTO_STRCAT(test_function),                                           \
        _TYTO_STRCAT(__FILE__),                                                \
        (ty_test_optionals_t){__VA_ARGS__})

//! Add a [[test_function]] to testing suite.
//! @private use macro [[ty_test]] which wraps this function.
void
_ty_test(
    void (*test_function)(void),
    const char*         function_name,
    const char*         file_name,
    ty_test_optionals_t parameters);

typedef struct ty_test_run_optionals
{
    int         default_timeout;
    bool        quiet;
    bool        no_capture_output;
    const char* suite;
} ty_test_run_optionals_t;

//! Run a test
//! @public
#define ty_test_run(...) _ty_test_run((ty_test_run_optionals_t){__VA_ARGS__})

//! Run a test.
//! @private use macro [[ty_test_run]] which wraps this function.
int
_ty_test_run(ty_test_run_optionals_t options);

//! Assert a condition.
//! @public
#define ty_tassert(condition)                                                  \
    _ty_test_assert((condition), #condition, __FILE__, __LINE__)

//! Assert a condition.
//! @private use macro [[ty_tassert]] which wraps this function.
void
_ty_test_assert(
    bool        condition,
    const char* expression,
    const char* file,
    int         line);

//! Test assert string [[s1 == s2]]
//! @public
#define ty_tassert_string(s1, s2)                                              \
    _ty_test_assert_string((s1), (s2), __FILE__, __LINE__)

//! Test assert string [[s1 == s2]] .
//! @private use macro [ty_tassert_string] which wraps this function.
void
_ty_test_assert_string(
    const char* s1,
    const char* s2,
    const char* file,
    int         line);

typedef struct ty_assert_num_optionals
{
    long double tolerance; /* default 0.8 */
} ty_assert_num_optionals_t;

//! Test assert numbers [[n1 == n2]]
//! @public
#define ty_tassert_number(n1, n2, ...)                                         \
    _ty_test_assert_number(                                                    \
        (n1),                                                                  \
        (n2),                                                                  \
        (ty_assert_num_optionals_t){__VA_ARGS__},                              \
        __FILE__,                                                              \
        __LINE__)

//! Test assert numbers [[n1 == n2]].
//! @private use macro [[ty_tassert_number]] which wraps this function.
//! @NOTE: not portable to WINDOWS or MACOS
void
_ty_test_assert_number(
    long double               n1,
    long double               n2,
    ty_assert_num_optionals_t parameters,
    const char*               file,
    int                       line);

//! Test assert two pointers are the same.
//! @public
#define ty_tassert_ptr(p1, p2, size)                                           \
    _ty_test_assert_ptr((p1), (p2), (size), __FILE__, __LINE__)

//! Test assert two pointers are the same.
//! @private use macro [[ty_tassert_ptr]] which wraps this function.
void
_ty_test_assert_ptr(
    const void* p1,
    const void* p2,
    size_t      sz,
    const char* file,
    int         line);

#endif /* TY_TEST_H_ */

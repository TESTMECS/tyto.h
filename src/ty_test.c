#include "stdlib.h"
#include "sys/socket.h"
#include "time.h"
#include "unistd.h"

#include <stdio.h>
#include <ty_test.h>

#define TY_COLOR_RESET  "\x1b[0m"
#define TY_COLOR_RED    "\x1b[31m"
#define TY_COLOR_GREEN  "\x1b[32m"
#define TY_COLOR_YELLOW "\x1b[33m"
#define TY_COLOR_BLUE   "\x1b[34m"
#define TY_COLOR_CYAN   "\x1b[36m"
#define TY_COLOR_BOLD   "\x1b[1m"

static ty_test_t* __ty_tests                      = NULL;
static int        __ty_testsc                     = 0;
static int        __ty_testsk                     = 0;
static bool       __ty_test_should_capture_output = true;
static int        __ty_test_failure_pipe[2]       = {-1, -1};

//! Trim .ext from a filename
//! @example
//! "foo/bar/foobar.txt" -> "foobar"
//! Also clears quotes:
//! "\"foo/bar/foobar.txt\"" -> "foobar"
static const char*
_ty_base_strip_extension(const char* filename)
{
#define TY_FILE_BASE(file)                                                     \
    (strrchr((file), '/') ? strrchr((file), '/') + 1 : (file))
    static char buffer[256];
    const char* base   = TY_FILE_BASE(filename);
    int         length = (int)strlen(base);
    //! strip beginning and ending quotes.
    if (length > 0 && base[(length - 1)] == '"')
        length--;
    if (length > 0 && base[0] == '"') {
        base++;
        length--;
    }
    snprintf(buffer, sizeof(buffer), "%.*s", length, base);
    char* extension = strrchr(buffer, '.');
    if (extension)
        *extension = '\0';
    return buffer;
#undef TY_FILE_BASE
}

//! Get clocktime using [[clock_gettime]]
//! Return value in milliseconds.
static long long
_ty_test_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

//! Append a string [[s]] to [[p]] preserving escapes.
static char*
_ty_test_append_escaped(char* p, char* end, const char* s)
{
    if (!s)
        return p;
    for (; *s && p < end; s++) {
        if ((*s == '\\' || *s == '|') && p + 1 < end)
            *p++ = '\\';
        *p++ = *s;
    }
    return p;
}

//! Append a string [[s]] to [[p]].
static char*
_ty_test_append_literal(char* p, char* end, const char* s)
{
    while (s && *s && p < end)
        *p++ = *s++;
    return p;
}

//! Append the integer [[value]] to string [[p]].
static char*
_ty_test_append_int(char* p, char* end, int value)
{
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%d", value);
    return _ty_test_append_literal(p, end, tmp);
}

//! Write the failure message to the global failure pipe instance.
//! @global __ty_test_failure_pipe
static void
_ty_test_write_failure_to_pipe(
    ty_test_result_kind_t kind,
    const char*           expression,
    const char*           expected,
    const char*           got,
    const char*           file,
    int                   line)
{
    if (__ty_test_failure_pipe[1] < 0) {
        _exit(1);
        return;
    }

    const char* expression_text;
    switch (kind) {
        case TY_TEST_RESULT_FAIL_ASSERT:
            expression_text = expression ? expression : "";
            break;
        case TY_TEST_RESULT_FAIL_STRING:
        case TY_TEST_RESULT_FAIL_NUMBER:
            expression_text = expression ? expression : "-";
            break;
        case TY_TEST_RESULT_FAIL_MEMORY:
            expression_text = "Memory compare failed.";
            break;
        case TY_TEST_RESULT_FAIL_TIMEOUT:
            expression_text = "Timeout exceeded";
            break;
        default:
            return;
    }

    char  buffer[4096];
    char* end = buffer + sizeof(buffer) - 1;
    char* p   = buffer;

    p  = _ty_test_append_int(p, end, (int)kind);
    p  = _ty_test_append_literal(p, end, "|");
    p  = _ty_test_append_escaped(p, end, file);
    p  = _ty_test_append_literal(p, end, "|");
    p  = _ty_test_append_escaped(p, end, expression_text);
    p  = _ty_test_append_literal(p, end, "|");
    p  = _ty_test_append_escaped(p, end, expected);
    p  = _ty_test_append_literal(p, end, "|");
    p  = _ty_test_append_escaped(p, end, got);
    p  = _ty_test_append_literal(p, end, "|");
    p  = _ty_test_append_int(p, end, line);
    p  = _ty_test_append_literal(p, end, "|");
    *p = '\0';

    ssize_t written =
        write(__ty_test_failure_pipe[1], buffer, (size_t)(p - buffer));
    (void)written;
    _exit(1);
}

static int
_ty_split_fields(char* buffer, char** fields, int max_fields)
{
    int   count = 0;
    char* p     = buffer;
    while (count < max_fields) {
        fields[count++] = p;
        bool split      = false;
        while (*p) {
            if (*p == '\\' && p[1] != '\0')
                p += 2;
            else if (*p == '|') {
                *p++  = '\0';
                split = true;
                break;
            } else {
                p++;
            }
        }
        if (!split)
            break;
    }
    return count;
}

static void
_ty_test_unescape(char* s)
{
    char* w = s;
    for (char* r = s; *r; r++) {
        if (*r == '\\' && (r[1] == '\\' || r[1] == '|'))
            r++;
        *w++ = *r;
    }
    *w = '\0';
}

//! @global __ty_test_failure_pipe
static bool
_ty_read_failure_from_pipe(ty_test_result_t* result)
{
    if (__ty_test_failure_pipe[0] < 0)
        return false;
    fd_set         fds;
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 1000;

    FD_ZERO(&fds);
    FD_SET(__ty_test_failure_pipe[0], &fds);

    int ret = select(__ty_test_failure_pipe[0] + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0)
        return false;

    char    buffer[4096];
    ssize_t n = read(__ty_test_failure_pipe[0], buffer, sizeof(buffer) - 1);
    if (n <= 0)
        return false;
    buffer[n] = '\0';

    char* fields[6] = {0};
    int   nfields   = _ty_split_fields(buffer, fields, 6);
    if (nfields < 2)
        return false;

    char* type_string = fields[0];
    char* file_string = fields[1];
    char* expression  = (nfields > 2) ? fields[2] : NULL;
    char* expected    = (nfields > 3) ? fields[3] : NULL;
    char* got         = (nfields > 4) ? fields[4] : NULL;
    char* line_string = (nfields > 5) ? fields[5] : NULL;

    _ty_test_unescape(file_string);
    if (expression)
        _ty_test_unescape(expression);
    if (expected)
        _ty_test_unescape(expected);
    if (got)
        _ty_test_unescape(got);

    result->result = (ty_test_result_kind_t)atoi(type_string);
    snprintf(
        result->failure_file,
        sizeof(result->failure_file),
        "%s",
        file_string ? file_string : "");
    if (line_string)
        result->failure_line = atoi(line_string);

    switch (result->result) {
        case TY_TEST_RESULT_FAIL_ASSERT:
            snprintf(
                result->failure_expression,
                sizeof(result->failure_expression),
                "%s",
                expression ? expression : "");
            break;
        case TY_TEST_RESULT_FAIL_STRING:
        case TY_TEST_RESULT_FAIL_NUMBER: {
            snprintf(
                result->failure_expression,
                sizeof(result->failure_expression),
                "%s",
                expression ? expression : "");
            snprintf(
                result->failure_expected,
                sizeof(result->failure_expected),
                "%s",
                expected ? expected : "");
            snprintf(
                result->failure_got,
                sizeof(result->failure_got),
                "%s",
                got ? got : "");
        } break;
        case TY_TEST_RESULT_FAIL_MEMORY:
            snprintf(
                result->failure_expression,
                sizeof(result->failure_expression),
                "timeout");
            break;
        default:
            return true;
    }
    return true;
}

static void
_ty_test_capture_output(
    int    pipe_fd,
    char*  buffer,
    size_t buffer_size,
    int    timeout_sec)
{
    if (pipe_fd < 0)
        return;

    if (timeout_sec > 0) {
        struct timeval tv;
        tv.tv_sec  = timeout_sec;
        tv.tv_usec = 0;
        setsockopt(pipe_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    ssize_t n = read(pipe_fd, buffer, buffer_size - 1);
    if (n > 0)
        buffer[n] = '\0';
    else
        buffer[0] = '\0';

    close(pipe_fd);
}

void
_ty_test_assert(
    bool        condition,
    const char* expression,
    const char* file,
    int         line)
{
    if (condition)
        return;
    _ty_test_write_failure_to_pipe(
        TY_TEST_RESULT_FAIL_ASSERT, expression, NULL, NULL, file, line);
    _exit(1);
}

void
_ty_test_assert_string(
    const char* s1,
    const char* s2,
    const char* file,
    int         line)
{
    if (s1 == s2)
        return;
    if (s1 && s2 && strcmp(s1, s2) == 0)
        return;
    _ty_test_write_failure_to_pipe(
        TY_TEST_RESULT_FAIL_STRING,
        NULL,
        s1 ? s1 : "(null)",
        s2 ? s2 : "(null)",
        file,
        line);
    _exit(1);
}

void
_ty_test_assert_number(
    long double               n1,
    long double               n2,
    ty_assert_num_optionals_t parameters,
    const char*               file,
    int                       line)
{
    long double tolerance = parameters.tolerance;

    if (n1 == n2)
        return;

    long double diff = (n1 - n2);
    if (diff < 0)
        diff = -diff;
    if (diff <= tolerance)
        return;

    char bufferA[64];
    char bufferB[64];
    snprintf(bufferA, sizeof(bufferA), "%.6Lg", n1);
    snprintf(bufferB, sizeof(bufferB), "%.6Lg", n2);

    char tolerance_buffer[64];
    snprintf(tolerance_buffer, sizeof(tolerance_buffer), "%.6Lg", tolerance);

    char expression_buffer[256];
    snprintf(
        expression_buffer,
        sizeof(expression_buffer),
        "%s ~= %s (tolerance: %s)",
        bufferA,
        bufferB,
        tolerance_buffer);

    _ty_test_write_failure_to_pipe(
        TY_TEST_RESULT_FAIL_NUMBER,
        expression_buffer,
        bufferB,
        bufferA,
        file,
        line);
    _exit(1);
}

void
_ty_test_assert_ptr(
    const void* p1,
    const void* p2,
    size_t      sz,
    const char* file,
    int         line)
{
    if (p1 == p2)
        return;
    if (memcmp(p1, p2, sz) == 0)
        return;
    _ty_test_write_failure_to_pipe(
        TY_TEST_RESULT_FAIL_MEMORY, NULL, NULL, NULL, file, line);
    _exit(1);
}

static void
_ty_test_print_result(ty_test_result_t* result)
{
    if (result->result > TY_TEST_RESULT_FAIL_TIMEOUT)
        return;
    if (result->failure_file[0])
        fprintf(
            stdout, "    %s:%d\n", result->failure_file, result->failure_line);
    switch (result->result) {
        case TY_TEST_RESULT_FAIL:
        case TY_TEST_RESULT_FAIL_ASSERT:
            if (result->failure_expression[0])
                fprintf(stdout, "    %s\n", result->failure_expression);
            break;
        case TY_TEST_RESULT_FAIL_STRING:
            if (result->failure_expected[0] || result->failure_got[0]) {
                fprintf(stdout, "    got:    \"%s\"\n", result->failure_got);
                fprintf(
                    stdout, "    expected: \"%s\"\n", result->failure_expected);
            }
            break;
        case TY_TEST_RESULT_FAIL_NUMBER:
            if (result->failure_expected[0] || result->failure_got[0]) {
                fprintf(stdout, "    %s\n", result->failure_expression);
                fprintf(stdout, "    got:    %s\n", result->failure_got);
                fprintf(stdout, "    expected: %s\n", result->failure_expected);
            }
            break;
        case TY_TEST_RESULT_FAIL_MEMORY:
            fputs("    memory regions differ\n", stdout);
            break;
        case TY_TEST_RESULT_FAIL_TIMEOUT:
            fprintf(
                stdout, "    test exceeded timeout (%d s)\n", result->timeout);
            break;
        default:
            return;
    }

    if (result->captured_output[0]) {
        fputs("    stdout:\n", stdout);
        const char* start = result->captured_output;
        while (*start) {
            const char* newline = strchr(start, '\n');
            if (!newline) {
                fprintf(stdout, "        %s\n", start);
                break;
            }
            fprintf(stdout, "    %.*s\n", (int)(newline - start), start);
            start = newline + 1;
        }
    }

    if (result->captured_stderr[0]) {
        fputs("    stderr:\n", stdout);
        const char* start = result->captured_stderr;
        while (*start) {
            const char* newline = strchr(start, '\n');
            if (!newline) {
                fprintf(stdout, "        %s\n", start);
                break;
            }
            fprintf(stdout, "    %.*s\n", (int)(newline - start), start);
            start = newline + 1;
        }
    }
}

static void
_ty_test_summary(
    int       passed,
    int       failed,
    int       skipped,
    int       errors,
    long long total_duration_ms)
{
    fputs(TY_COLOR_BOLD "\n[summary]" TY_COLOR_RESET, stdout);
    if (failed > 0)
        fputs(TY_COLOR_RED, stdout);
    fprintf(stdout, "    failed: %d\n", failed);
    if (failed > 0)
        fputs(TY_COLOR_RESET, stdout);
    if (passed > 0)
        fputs(TY_COLOR_GREEN, stdout);
    fprintf(stdout, "    passed: %d\n", passed);
    if (passed > 0)
        fputs(TY_COLOR_RESET, stdout);
    if (errors > 0)
        fputs(TY_COLOR_RED, stdout);
    fprintf(stdout, "    errors: %d\n", errors);
    if (errors > 0)
        fputs(TY_COLOR_RESET, stdout);
    if (skipped > 0)
        fputs(TY_COLOR_YELLOW, stdout);
    fprintf(stdout, "    skipped: %d\n", skipped);
    if (skipped > 0)
        fputs(TY_COLOR_RESET, stdout);
    fprintf(stdout, "  total: %d\n", passed + failed + skipped + errors);
    fprintf(
        stdout,
        "  duration: %lld.%03dms\n",
        (long long)(total_duration_ms / 1000),
        (int)(total_duration_ms % 1000));
    fputs(TY_COLOR_RESET, stdout);
}

static void
_ty_test_runInChild(
    ty_test_t* test,
    int        pipe_stdout[2],
    int        pipe_stderr[2],
    bool       captured_output)
{
    if (captured_output) {
        dup2(pipe_stdout[1], STDOUT_FILENO);
        dup2(pipe_stderr[1], STDERR_FILENO);
        close(pipe_stdout[0]);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        close(pipe_stderr[1]);
    } else {
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);
        close(pipe_stderr[0]);
        close(pipe_stderr[1]);
    }
    close(__ty_test_failure_pipe[0]);
    test->ty_test_function();
    _exit(0);
}

static ty_test_result_t
_ty_run_test(ty_test_t* test, int global_timeout)
{
    ty_test_result_t result = {0};
    result.result           = TY_TEST_RESULT_PASS;

    bool captured_output = __ty_test_should_capture_output;
    int  timeout         = test->timeout > 0 ? test->timeout : global_timeout;

    int pipe_stdout[2] = {-1, -1};
    int pipe_stderr[2] = {-1, -1};

    if (captured_output) {
        if (pipe(pipe_stdout) != 0 || pipe(pipe_stderr) != 0) {
            result.result = TY_TEST_RESULT_ERROR;
            snprintf(
                result.failure_expression,
                sizeof(result.failure_expression),
                "Failed to create pipe");
            return result;
        }
    }

    if (pipe(__ty_test_failure_pipe) != 0) {
        result.result = TY_TEST_RESULT_ERROR;
        snprintf(
            result.failure_expression,
            sizeof(result.failure_expression),
            "Failed to create failure pipe");
        return result;
    }

    long long start_ms = _ty_test_now_ms();

    pid_t pid = fork();
    if (pid < 0) {
        result.result = TY_TEST_RESULT_ERROR;
        snprintf(
            result.failure_expression,
            sizeof(result.failure_expression),
            "Failed to fork: %s",
            strerror(errno));
        return result;
    }

    if (pid == 0)
        _ty_test_runInChild(test, pipe_stdout, pipe_stderr, captured_output);

    if (pipe_stdout[1] >= 0)
        close(pipe_stdout[1]);

    if (pipe_stderr[1] >= 0)
        close(pipe_stderr[1]);

    int status       = 0;
    int wait_counter = 0;
    while (1) {
        pid_t ret = waitpid(pid, &status, WNOHANG);
        if (ret != 0)
            break;
        wait_counter++;
        if (timeout > 0 && wait_counter >= timeout * 100) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            status = 0;
            break;
        }
        usleep(10000);
    }

    int timeout_val = (test->timeout > 0) ? test->timeout : global_timeout;
    result.timeout  = timeout_val;

    if (timeout > 0 && wait_counter >= timeout_val * 100)
        result.result = TY_TEST_RESULT_FAIL_TIMEOUT;

    long long end_ms   = _ty_test_now_ms();
    result.duration_ms = end_ms - start_ms;

    _ty_read_failure_from_pipe(&result);

    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        int sig       = WTERMSIG(status);
        result.signal = sig;
        result.result = TY_TEST_RESULT_FAIL;
        snprintf(
            result.failure_expression,
            sizeof(result.failure_expression),
            "Test killed by signal %d",
            sig);
    }

    if (result.result == TY_TEST_RESULT_PASS && WIFEXITED(status) &&
        result.exit_code != 0)
        result.result = TY_TEST_RESULT_FAIL;

    close(__ty_test_failure_pipe[0]);
    __ty_test_failure_pipe[0] = -1;
    __ty_test_failure_pipe[1] = -1;

    if (captured_output) {
        _ty_test_capture_output(
            pipe_stdout[0],
            result.captured_output,
            sizeof(result.captured_output),
            timeout);
        _ty_test_capture_output(
            pipe_stderr[0],
            result.captured_stderr,
            sizeof(result.captured_stderr),
            timeout);
    }
    return result;
}

void
_ty_test(
    void (*test_function)(void),
    const char*         function_name,
    const char*         file_name,
    ty_test_optionals_t parameters)
{
    if (__ty_testsk == 0 || __ty_testsc >= __ty_testsk) {
        int        new_capacity = __ty_testsk == 0 ? 64 : __ty_testsk + 64;
        ty_test_t* new_tests =
            (ty_test_t*)realloc(__ty_tests, new_capacity * sizeof(ty_test_t));
        if (!new_tests) {
            fprintf(
                stderr, "[Tyto::Tests] Failed to allocate memory for tests\n");
            return;
        }
        __ty_tests  = new_tests;
        __ty_testsk = new_capacity;
    }

    ty_test_t* t = &__ty_tests[__ty_testsc];

    t->ty_test_function = test_function;
    t->suite            = parameters.suite;
    t->skip_because     = parameters.skip_because;
    t->function_name    = function_name;
    t->timeout          = parameters.timeout;
    t->should_fail      = parameters.should_fail;

    if (!t->suite)
        t->suite = strdup(_ty_base_strip_extension(file_name));

    __ty_testsc++;
}

int
_ty_test_run(ty_test_run_optionals_t options)
{
    int passed                      = 0;
    int failed                      = 0;
    int skipped                     = 0;
    int errors                      = 0;
    int default_timeout             = options.default_timeout;
    __ty_test_should_capture_output = !options.no_capture_output;

    long long total_start = _ty_test_now_ms();

    fputs("\n[tests]\n", stdout);

    const char* filter_suite = options.suite;

    for (int i = 0; i < __ty_testsc; i++) {
        ty_test_t* t = &__ty_tests[i];
        if (!t->function_name)
            t->function_name = "unknown";
    }

    const char* current_suite = NULL;

    for (int i = 0; i < __ty_testsc; i++) {
        ty_test_t* t = &__ty_tests[i];

        if (filter_suite && strcmp(t->suite, filter_suite) != 0)
            continue;

        if (!current_suite || strcmp(t->suite, current_suite) != 0) {
            current_suite = t->suite;
            fprintf(
                stdout,
                "\n" TY_COLOR_BOLD "[%s]" TY_COLOR_RESET "\n",
                current_suite);
        }

        if (t->skip_because) {
            skipped++;
            fputs("  " TY_COLOR_YELLOW "SKIP" TY_COLOR_RESET, stdout);
            fprintf(stdout, " %s %s\n", t->function_name, t->skip_because);
            continue;
        }

        ty_test_result_t result = _ty_run_test(t, default_timeout);

        if (result.result == TY_TEST_RESULT_PASS) {
            if (t->should_fail) {
                failed++;
                fputs("  " TY_COLOR_RED "FAIL" TY_COLOR_RESET, stdout);
                fprintf(
                    stdout,
                    " %s (expected to fail but passed)\n",
                    t->function_name);
            } else {
                passed++;
                if (!options.quiet) {
                    fputs("  " TY_COLOR_GREEN "PASS" TY_COLOR_RESET, stdout);
                    fprintf(stdout, " %s\n", t->function_name);
                }
            }
        } else if (result.result > TY_TEST_RESULT_PASS) {
            if (t->should_fail) {
                passed++;
                if (!options.quiet) {
                    fputs("  " TY_COLOR_GREEN "PASS" TY_COLOR_RESET, stdout);
                    fprintf(
                        stdout, " %s (expected to fail)\n", t->function_name);
                }
            } else {
                failed++;
                fputs("  " TY_COLOR_RED "FAIL" TY_COLOR_RESET, stdout);
                fprintf(
                    stdout,
                    " %s (%.3fs)\n",
                    t->function_name,
                    result.duration_ms / 1000.0);
                _ty_test_print_result(&result);
            }
        } else if (result.result == TY_TEST_RESULT_ERROR) {
            errors++;
            fputs("  " TY_COLOR_RED "ERROR" TY_COLOR_RESET, stdout);
            fprintf(stdout, " %s", t->function_name);
            if (result.failure_expression[0])
                fprintf(stdout, ": %s", result.failure_expression);
            fprintf(stdout, "\n");
        }
    }

    long long total_duration = _ty_test_now_ms() - total_start;

    _ty_test_summary(passed, failed, skipped, errors, total_duration);

    free(__ty_tests);
    __ty_tests  = NULL;
    __ty_testsc = 0;
    __ty_testsk = 0;

    if (failed > 0)
        return 1;
    if (errors > 0)
        return 2;
    return 0;
}

#include <stdio.h>
#include <ty_argparse.h>
#include <ty_test.h>

void
test_basic(void)
{
    ty_tassert(true);
}

void
test_strings(void)
{
    ty_tassert_string("should equal", "should equal");
}

void
test_captured_output(void)
{
    fprintf(stdout, "captured stdout\n");
    fprintf(stderr, "captured stderr\n");
    ty_tassert(true);
}

void
test_numerics(void)
{
    ty_tassert_number(42, 42);
}

void
test_numerics_with_tolerance(void)
{
    ty_tassert_number(1.0f / 3.0f, 0.333f, .tolerance = 0.01f);
}

void
test_memory(void)
{
    int a[4] = {1, 2, 3, 4};
    int b[4] = {1, 2, 3, 4};
    ty_tassert_ptr(a, b, sizeof(a));
}

void
test_secondary(void)
{
    ty_tassert(true);
}

void
test_skipped(void)
{
    ty_tassert(true);
}

void
test_timeout(void)
{
    sleep(2);
    ty_tassert(true);
}

void
test_expected_failure_passes(void)
{
    ty_tassert(false);
}

void
test_sleep5(void)
{
    sleep(5);
    ty_tassert(true);
}

void
register_tests(void)
{
    ty_test(test_basic);
    ty_test(test_strings, .suite = "asserts");
    ty_test(test_numerics, .suite = "asserts");
    ty_test(test_numerics_with_tolerance, .suite = "asserts");
    ty_test(test_memory, .suite = "asserts");
    ty_test(test_secondary, .suite = "secondary suite");
    ty_test(test_skipped, .skip_because = "not implemented yet");
    ty_test(test_timeout);
    ty_test(test_captured_output);
    ty_test(test_expected_failure_passes, .should_fail = true);
    ty_test(test_sleep5, .should_fail = true, .timeout = 1);
}

int
main(int argc, char* argv[])
{
    register_tests();
    if (argc == 1)
        ty_test_run();

    arg_ctxt_t ctxt = arg_init(argv);
    arg_t      arg;
    while (!ARG_DONE(arg = arg_next(&ctxt))) {
        if (ARG_LONG(arg, "no-pass"))
            ty_test_run(.quiet = true);
        else if (ARG_LONG(arg, "no-capture"))
            ty_test_run(.no_capture_output = true);
    }
    return 0;
}

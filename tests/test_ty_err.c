#include <stdio.h>
#include <ty_array.h>
#include <ty_err.h>
#include <ty_test.h>

//! We can define error constants up to 4095
enum
{
    FILE_OPEN_ERROR = 12,
    NO_ERROR_MSG    = 13,
};
//! Mapping codes to diagnostics.
static const char* error_msgs[] = {
    [FILE_OPEN_ERROR] = "Could not open file, ensure path is correct",
};

static const char*
ty_get_err_msg(err_t code)
{
    //! Make sure to rule out negative values for array index here.
    //! The expression [(if (code >= ARRAY_SIZE(error_msgs)))] will make the
    //! compiler warn because comparing [signed >= unsigned]. If [code] is
    //! negative it will be converted to [unsigned] BEFORE comparison.
    //! -1  -> 18446744073709551615UL
    //! This could produce suprising results.
    //! Solutions:
    //! 1. Rule out unsigned values [code < 0] make safe cast [(size_t)code].
    //! 2. Could make [typedef err_t uintptr_t] then no negatives for constants.
    if (code < 0 || (size_t)code >= ARRAY_SIZE(error_msgs))
        return "Unknown error";
    if (!error_msgs[code])
        return "Unknown error";
    return error_msgs[code];
}

//! For libc functions that return pointers,
//! throw the monadic error we want.
FILE*
ty_fopen(const char* path, const char* perms)
{
    FILE* f = fopen(path, perms);
    return (!f) ? ERR(FILE_OPEN_ERROR) : f;
}

int
main(void)
{
    TY_TEST_BEGIN();
    FILE* file = ty_fopen("", "rb");
    if (IS_ERR(file)) {
        err_t err = ERR_CODE(file);
        fprintf(stderr, "[Error] " ERR_FMT "\n", err);
        fprintf(stderr, "[Error::msg] %s\n", ty_get_err_msg(err));
        return 1;
    }
    TY_TEST_END();
    return 0;
}

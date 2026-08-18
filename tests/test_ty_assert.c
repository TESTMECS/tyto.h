#include <ty_assert.h>

int
main(void)
{
    int UNIQUE_ID(tmp) = 1;  // tmp6

    int sizeof_int = 4;  // change this to see static assert!
    TYTO_STATIC_ASSERT(sizeof(int) == sizeof_int);
}

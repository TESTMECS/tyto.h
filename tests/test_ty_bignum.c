#include <stdio.h>
#include <ty_bignum.h>
#include <ty_test.h>

void
fact(struct ty_bn* bn, struct ty_bn* res)
{
    struct ty_bn tmp;
    ty_bignum_assign(&tmp, bn);
    ty_bignum_dec(bn);
    while (!ty_bignum_is_zero(bn)) {
        ty_bignum_mul(&tmp, bn, res);
        ty_bignum_dec(bn);
        ty_bignum_assign(&tmp, res);
    }
    ty_bignum_assign(res, &tmp);
}

void
test_fact(void)
{
    struct ty_bn num;
    struct ty_bn result;
    char         buffer[8192];
    int          n = 100;
    ty_bignum_from_int(&num, n);
    fact(&num, &result);
    ty_bignum_to_str(&result, buffer, sizeof(buffer));
    //! Reminder! output is hex
    printf("Factorial(%d) *Hex* = %s\n", n, buffer);
    printf("Factorial(%d) *int* = %d\n", n, ty_bignum_to_int(&result));
}

int
main(void)
{
    TY_TEST_BEGIN();
    test_fact();
    TY_TEST_END();
    return 0;
}

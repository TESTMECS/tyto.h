#include <stdio.h>
#include <ty_bignum.h>
static void
shl_one(struct ty_bn* a);

static void
shr_one(struct ty_bn* a);

static void
shl_word(struct ty_bn* a, int nwords);

static void
shr_word(struct ty_bn* a, int nwords);

void
ty_bignum_init(struct ty_bn* bn)
{
    require(bn, "big-num cannot be null");
    for (int i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i)
        bn->array[i] = 0;
}

void
ty_bignum_from_int(struct ty_bn* bn, TY_BIGNUM_DTYPE_TMP i)
{
    require(bn, "big-num cannot be null");
    ty_bignum_init(bn);
    //! TODO(little endian issue)
#ifdef TY_BIGNUM_WORD_SIZE
#if (TY_BIGNUM_WORD_SIZE == 1)
    bn->array[0] = (i & 0x000000ff);
    bn->array[1] = (i & 0x0000ff00) >> 8;
    bn->array[2] = (i & 0x00ff0000) >> 16;
    bn->array[3] = (i & 0xff000000) >> 24;
#elif (TY_BIGNUM_WORD_SIZE == 2)
    bn->array[0] = (i & 0x0000ffff);
    bn->array[1] = (i & 0xffff0000) >> 16;
#elif (TY_BIGNUM_WORD_SIZE == 4)
    bn->array[0]               = i;
    TY_BIGNUM_DTYPE_TMP num_32 = 32;
    TY_BIGNUM_DTYPE_TMP tmp =
        i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
    bn->array[1] = tmp;
#endif  // TY_BIGNUM_WORD_SIZE#if
#endif  // TY_BIGNUM_WORD_SIZE
}

int
ty_bignum_to_int(struct ty_bn* bn)
{
    require(bn, "big-num is null");
    int ret = 0;
#if (TY_BIGNUM_WORD_SIZE == 1)
    ret += bn->array[0];
    ret += bn->array[1] << 8;
    ret += bn->array[2] << 16;
    ret += bn->array[3] << 24;
#elif (TY_BIGNUM_WORD_SIZE == 2)
    ret += bn->array[0];
    ret += bn->array[1] << 16;
#elif (TY_BIGNUM_WORD_SIZE == 4)
    ret += bn->array[0];
#endif  // TY_BIGNUM_WORD_SIZE
    return ret;
}

void
ty_bignum_from_str(struct ty_bn* bn, char* str, int nbytes)
{
    require(bn, "big-num is null");
    require(str, "str is null");
    require(nbytes > 0, "nbytes must be positive");
    require(
        (nbytes & 1) == 0,
        "string format must be in hex -> equal number of bytes");
    require(
        (nbytes % (sizeof(TY_BIGNUM_DTYPE) * 2)) == 0,
        "string length must be a multiple of (TY_BIGNUM_DTYPE*2) characters.");

    ty_bignum_init(bn);

    TY_BIGNUM_DTYPE tmp;
    int             i = nbytes - (2 * TY_BIGNUM_WORD_SIZE);
    int             j = 0;

    while (i >= 0) {
        tmp = 0;
        sscanf(&str[i], TY_BIGNUM_SSCANF_FMT_STRING, &tmp);
        bn->array[j] = tmp;
        i -= (2 * TY_BIGNUM_WORD_SIZE);
        j += 1;
    }
}

void
ty_bignum_to_str(struct ty_bn* bn, char* str, int nbytes)
{
    require(bn, "big-num is null");
    require(str, "str is null");
    require(nbytes > 0, "nbytes must be positive");
    require(
        (nbytes & 1) == 0,
        "string format must be in hex -> equal number of bytes");

    int j = TY_BIGNUM_ARRAY_SIZE - 1;
    int i = 0;

    //! reading last array element "MSB" first, big-endian.
    while ((j >= 0) && (nbytes > (i + 1))) {
        sprintf(&str[i], TY_BIGNUM_SPRINTF_FMT_STRING, bn->array[j]);
        i += (2 * TY_BIGNUM_WORD_SIZE);
        j -= 1;
    }
    //! Count leading zeros.
    j = 0;
    while (str[j] == '0')
        j += 1;

    //! Move string j places ahead, effectively skipping leading zeros.
    for (i = 0; i < (nbytes - j); ++i) {
        str[i] = str[i + j];
    }
    str[i] = 0;
}

void
ty_bignum_dec(struct ty_bn* bn)
{
    require(bn, "big-num is null");
    TY_BIGNUM_DTYPE tmp;
    TY_BIGNUM_DTYPE res;

    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        tmp          = bn->array[i];
        res          = tmp - 1;
        bn->array[i] = res;
        if (!(res > tmp))
            break;
    }
}

void
ty_bignum_inc(struct ty_bn* bn)
{
    require(bn, "big-num is null");
    TY_BIGNUM_DTYPE     res;
    TY_BIGNUM_DTYPE_TMP tmp;
    int                 i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        tmp          = bn->array[i];
        res          = tmp + 1;
        bn->array[i] = res;
        if (res > tmp)
            break;
    }
}

void
ty_bignum_add(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");

    TY_BIGNUM_DTYPE_TMP tmp;
    int                 carry = 0;
    int                 i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        tmp         = (TY_BIGNUM_DTYPE_TMP)a->array[i] + b->array[i] + carry;
        carry       = (tmp > TY_BIGNUM_MAX_VAL);
        c->array[i] = (tmp & TY_BIGNUM_MAX_VAL);
    }
}

void
ty_bignum_sub(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");

    TY_BIGNUM_DTYPE_TMP res;
    TY_BIGNUM_DTYPE_TMP tmp1;
    TY_BIGNUM_DTYPE_TMP tmp2;
    int                 borrow = 0;
    int                 i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        tmp1 = (TY_BIGNUM_DTYPE_TMP)a->array[i] + (TY_BIGNUM_MAX_VAL + 1);
        tmp2 = (TY_BIGNUM_DTYPE_TMP)b->array[i] + borrow;
        res  = (tmp1 - tmp2);
        c->array[i] = (TY_BIGNUM_DTYPE)(res & TY_BIGNUM_MAX_VAL);
        borrow      = (res <= TY_BIGNUM_MAX_VAL);
    }
}

void
ty_bignum_mul(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");

    struct ty_bn row;
    struct ty_bn tmp;
    int          i, j;
    ty_bignum_init(c);
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        ty_bignum_init(&row);
        for (j = 0; j < TY_BIGNUM_ARRAY_SIZE; ++j) {
            if (i + j < TY_BIGNUM_ARRAY_SIZE) {
                ty_bignum_init(&tmp);
                TY_BIGNUM_DTYPE_TMP intermediate =
                    ((TY_BIGNUM_DTYPE_TMP)a->array[i] *
                     (TY_BIGNUM_DTYPE_TMP)b->array[j]);
                ty_bignum_from_int(&tmp, intermediate);
                shl_word(&tmp, i + j);
                ty_bignum_add(&tmp, &row, &row);
            }
        }
        ty_bignum_add(c, &row, c);
    }
}

void
ty_bignum_div(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");

    struct ty_bn current;
    struct ty_bn denom;
    struct ty_bn tmp;

    ty_bignum_from_int(&current, 1);
    ty_bignum_assign(&denom, b);
    ty_bignum_assign(&tmp, a);

    const TY_BIGNUM_DTYPE_TMP half_max =
        1 + (TY_BIGNUM_DTYPE_TMP)(TY_BIGNUM_MAX_VAL / 2);
    bool overflow = false;
    while (ty_bignum_cmp(&denom, a) != LARGER) {
        if (denom.array[TY_BIGNUM_ARRAY_SIZE - 1] >= half_max) {
            overflow = true;
            break;
        }
        shl_one(&current);
        shl_one(&denom);
    }
    if (!overflow) {
        shr_one(&denom);
        shr_one(&current);
    }
    ty_bignum_init(c);

    while (!ty_bignum_is_zero(&current)) {
        if (ty_bignum_cmp(&tmp, &denom) != SMALLER) {
            ty_bignum_sub(&tmp, &denom, &tmp);
            ty_bignum_or(c, &current, c);
        }
        shr_one(&current);
        shr_one(&denom);
    }
}

void
ty_bignum_shl(struct ty_bn* a, struct ty_bn* b, int nbits)
{
    require(a, "a is null");
    require(b, "b is null");
    require(nbits >= 0, "no negative shifts");

    ty_bignum_assign(b, a);
    const int nbits_pr_word = (TY_BIGNUM_WORD_SIZE * 8);
    int       nwords        = nbits / nbits_pr_word;
    if (nwords != 0) {
        shl_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }
    if (nbits != 0) {
        int i;
        for (i = (TY_BIGNUM_ARRAY_SIZE - 1); i > 0; --i) {
            b->array[i] =
                (b->array[i] << nbits) |
                (b->array[i - 1] >> ((8 * TY_BIGNUM_WORD_SIZE) - nbits));
        }
        b->array[i] <<= nbits;
    }
}

void
ty_bignum_shr(struct ty_bn* a, struct ty_bn* b, int nbits)
{
    require(a, "a is null");
    require(b, "b is null");
    require(nbits >= 0, "no negative shifts");

    ty_bignum_assign(b, a);
    const int nbits_pr_word = (TY_BIGNUM_WORD_SIZE * 8);
    int       nwords        = nbits / nbits_pr_word;
    if (nwords != 0) {
        shr_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }
    if (nbits != 0) {
        int i;
        for (i = 0; i < (TY_BIGNUM_WORD_SIZE - 1); ++i) {
            b->array[i] =
                (b->array[i] >> nbits) |
                (b->array[i + 1] << ((8 * TY_BIGNUM_WORD_SIZE) - nbits));
        }
        b->array[i] >>= nbits;
    }
}

void
ty_bignum_mod(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    struct ty_bn tmp;
    ty_bignum_divmod(a, b, &tmp, c);
}

void
ty_bignum_divmod(
    struct ty_bn* a,
    struct ty_bn* b,
    struct ty_bn* c,
    struct ty_bn* d)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    struct ty_bn tmp;

    ty_bignum_div(a, b, c);
    ty_bignum_mul(c, b, &tmp);
    ty_bignum_sub(a, &tmp, d);
}

void
ty_bignum_and(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        c->array[i] = (a->array[i] & b->array[i]);
    }
}

void
ty_bignum_or(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        c->array[i] = (a->array[i] | b->array[i]);
    }
}

void
ty_bignum_xor(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        c->array[i] = (a->array[i] ^ b->array[i]);
    }
}

int
ty_bignum_cmp(struct ty_bn* a, struct ty_bn* b)
{
    require(a, "a is null");
    require(b, "b is null");
    int i = TY_BIGNUM_ARRAY_SIZE;
    do {
        i -= 1;
        if (a->array[i] > b->array[i])
            return LARGER;
        else if (a->array[i] < b->array[i])
            return SMALLER;
    } while (i != 0);
    return EQUAL;
}

int
ty_bignum_is_zero(struct ty_bn* bn)
{
    require(bn, "a is null");

    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i) {
        if (bn->array[i])
            return 0;
    }
    return 1;
}

void
ty_bignum_pow(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c)
{
    require(a, "a is null");
    require(b, "b is null");
    require(c, "c is null");
    struct ty_bn tmp;
    ty_bignum_init(c);

    if (ty_bignum_cmp(b, c) == EQUAL) {
        ty_bignum_inc(c);
    } else {
        struct ty_bn bcopy;
        ty_bignum_assign(&bcopy, b);

        ty_bignum_assign(&tmp, a);
        ty_bignum_dec(&bcopy);

        while (!ty_bignum_is_zero(&bcopy)) {
            ty_bignum_mul(&tmp, a, c);
            ty_bignum_dec(&bcopy);
            ty_bignum_assign(&tmp, c);
        }
        ty_bignum_assign(c, &tmp);
    }
}

void
ty_bignum_isqrt(struct ty_bn* a, struct ty_bn* b)
{
    require(a, "a is null");
    require(b, "b is null");

    struct ty_bn low, high, mid, tmp;

    ty_bignum_init(&low);
    ty_bignum_assign(&high, a);
    ty_bignum_shr(&high, &mid, 1);
    ty_bignum_inc(&mid);

    while (ty_bignum_cmp(&high, &low) > 0) {
        ty_bignum_mul(&mid, &mid, &tmp);
        if (ty_bignum_cmp(&tmp, a) > 0) {
            ty_bignum_assign(&high, &mid);
            ty_bignum_dec(&high);
        } else {
            ty_bignum_assign(&low, &mid);
        }
        ty_bignum_sub(&high, &low, &mid);
        shr_one(&mid);
        ty_bignum_add(&low, &mid, &mid);
        ty_bignum_inc(&mid);
    }
    ty_bignum_assign(b, &low);
}

void
ty_bignum_assign(struct ty_bn* dst, struct ty_bn* src)
{
    require(dst, "dst is null");
    require(src, "src is null");

    int i;
    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i)
        dst->array[i] = src->array[i];
}

static void
shl_one(struct ty_bn* a)
{
    require(a, "a is null");
    int i;
    for (i = (TY_BIGNUM_ARRAY_SIZE - 1); i > 0; --i)
        a->array[i] = (a->array[i] << 1) |
                      (a->array[i - 1] >> ((8 * TY_BIGNUM_WORD_SIZE) - 1));

    a->array[0] <<= 1;
}

static void
shr_one(struct ty_bn* a)
{
    require(a, "a is null");
    int i;
    for (i = 0; i < (TY_BIGNUM_ARRAY_SIZE - 1); ++i)
        a->array[i] = (a->array[i] >> 1) |
                      (a->array[i - 1] << ((8 * TY_BIGNUM_WORD_SIZE) - 1));

    a->array[TY_BIGNUM_ARRAY_SIZE - 1] >>= 1;
}

static void
shl_word(struct ty_bn* a, int nwords)
{
    require(a, "a is null");
    require(nwords >= 0, "no negative shifts");

    int i;
    for (i = (TY_BIGNUM_ARRAY_SIZE - 1); i >= nwords; --i)
        a->array[i] = a->array[i - nwords];
    for (; i >= 0; --i)
        a->array[i] = 0;
}

static void
shr_word(struct ty_bn* a, int nwords)
{
    require(a, "a is null");
    require(nwords >= 0, "no negative shifts");

    int i;
    if (nwords >= TY_BIGNUM_ARRAY_SIZE) {
        for (i = 0; i < TY_BIGNUM_ARRAY_SIZE; ++i)
            a->array[i] = 0;
        return;
    }

    for (i = 0; i < TY_BIGNUM_ARRAY_SIZE - nwords; ++i)
        a->array[i] = a->array[i + nwords];
    for (; i < TY_BIGNUM_ARRAY_SIZE; ++i)
        a->array[i] = 0;
}

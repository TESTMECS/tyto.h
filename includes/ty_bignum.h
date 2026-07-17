#ifndef TY_BIGNUM_H_
#define TY_BIGNUM_H_

#include "./ty_assert.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef TY_BIGNUM_WORD_SIZE
#define TY_BIGNUM_WORD_SIZE 4
#endif

#define TY_BIGNUM_ARRAY_SIZE (128 / TY_BIGNUM_WORD_SIZE)

#ifndef TY_BIGNUM_WORD_SIZE
#error must define TYTY_BIGNUM_WORD_SIZE to be 1, 2, 4
#elif (TY_BIGNUM_WORD_SIZE == 1)
#define TY_BIGNUM_DTYPE              uint8_t
#define TY_BIGNUM_DTYPE_MSB          ((TY_BIGNUM_DTYPE_TMP)(0x80))
#define TY_BIGNUM_DTYPE_TMP          uint32_t
#define TY_BIGNUM_SPRINTF_FMT_STRING "%.02x"
#define TY_BIGNUM_SSCANF_FMT_STRING  "%2hhx"
#define TY_BIGNUM_MAX_VAL            ((TY_BIGNUM_DTYPE_TMP)0xFF)
#elif (TY_BIGNUM_WORD_SIZE == 2)
#define TY_BIGNUM_DTYPE              uint16_t
#define TY_BIGNUM_DTYPE_MSB          ((TY_BIGNUM_DTYPE_TMP)(0x8000))
#define TY_BIGNUM_DTYPE_TMP          uint32_t
#define TY_BIGNUM_SPRINTF_FMT_STRING "%.04x"
#define TY_BIGNUM_SSCANF_FMT_STRING  "%4hx"
#define TY_BIGNUM_MAX_VAL            ((TY_BIGNUM_DTYPE_TMP)0xFFFF)
#elif (TY_BIGNUM_WORD_SIZE == 4)
#define TY_BIGNUM_DTYPE              uint32_t
#define TY_BIGNUM_DTYPE_MSB          ((TY_BIGNUM_DTYPE_TMP)(0x80000000))
#define TY_BIGNUM_DTYPE_TMP          uint64_t
#define TY_BIGNUM_SPRINTF_FMT_STRING "%.08x"
#define TY_BIGNUM_SSCANF_FMT_STRING  "%8x"
#define TY_BIGNUM_MAX_VAL            ((TY_BIGNUM_DTYPE_TMP)0xFFFFFFFF)
#endif  // TY_BIGNUM_WORD_SIZE

#ifndef TY_BIGNUM_DTYPE
#error TY_BIGTY_BIGNUM_DTYPE must be either (uint8_t, uint16_t, or uint32_t)
#endif  // TY_BIGNUM_DTYPE

struct ty_bn
{
    TY_BIGNUM_DTYPE array[TY_BIGNUM_ARRAY_SIZE];
};

enum
{
    SMALLER = -1,
    EQUAL   = 0,
    LARGER  = 1,
};

void
ty_bignum_init(struct ty_bn* bn);

void
ty_bignum_from_int(struct ty_bn* bn, TY_BIGNUM_DTYPE_TMP i);

int
ty_bignum_to_int(struct ty_bn* bn);

void
ty_bignum_from_str(struct ty_bn* bn, char* str, int nbytes);

void
ty_bignum_to_str(struct ty_bn* bn, char* str, int maxsize);

//! c = a + b
void
ty_bignum_add(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a - b
void
ty_bignum_sub(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a * b
void
ty_bignum_mul(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a / b
void
ty_bignum_div(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a % b
void
ty_bignum_mod(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a / b; d = a % b
void
ty_bignum_divmod(
    struct ty_bn* a,
    struct ty_bn* b,
    struct ty_bn* c,
    struct ty_bn* d);

//! c = a & b
void
ty_bignum_and(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a | b
void
ty_bignum_or(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a ^ b
void
ty_bignum_xor(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

//! c = a << nbits
void
ty_bignum_shl(struct ty_bn* a, struct ty_bn* b, int nbits);

//! c = a >> nbits
void
ty_bignum_shr(struct ty_bn* a, struct ty_bn* b, int nbits);

int
ty_bignum_cmp(struct ty_bn* a, struct ty_bn* b);

int
ty_bignum_is_zero(struct ty_bn* n);

void
ty_bignum_inc(struct ty_bn* n);

void
ty_bignum_dec(struct ty_bn* n);

void
ty_bignum_pow(struct ty_bn* a, struct ty_bn* b, struct ty_bn* c);

void
ty_bignum_isqrt(struct ty_bn* a, struct ty_bn* b);

void
ty_bignum_assign(struct ty_bn* dst, struct ty_bn* src);

#endif  // TY_BIGNUM_H_

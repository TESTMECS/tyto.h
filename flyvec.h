//! @module flyvec.h
//! The idea is that we have a pointer to 2d array [ [2]void ] and,
//! void[0] -> Stores the length of the array.
//! void[2] -> Stores the actual values of the array.
//! FlyVec 16 bytes, just two pointers.
//! 24 bits for the normal: [struct String { data, size, capacity }].
//! FlyVec can be generic: [int a = myarr[1][i];] no casting required.
//! Assumptions ---
//! Reallocations and capacity must always be power of 2.
//! Assumes that (void*) and (uintptr_t) are same size.
//! Resize by casting integer to pointer [ints[0] = (void*)3] is undefined
//! behavior.
//! ---
#ifndef FLYVEC_H_
#define FLYVEC_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static inline bool
flyvec_resize_(void** data_ptr, uintptr_t len, size_t element_size)
{
    if ((len & (len - 1)) == 0) {
        uintptr_t capacity = len ? len * 2 : 1;
        if (capacity > SIZE_MAX / element_size || capacity <= len) {
            return false;
        }

        void* allocation = realloc(*data_ptr, element_size * capacity);
        if (allocation == NULL) {
            return false;
        }
        *data_ptr = allocation;
    }
    return true;
}

#define FLY_PUSH(vec_expr, type, value_expr)                                   \
    (flyvec_resize_(                                                           \
         (void**)&((vec_expr)[1]), (uintptr_t)(vec_expr)[0], sizeof(type))     \
         ? (((type*)(vec_expr)[1])[(uintptr_t)(vec_expr)[0]] = (value_expr),   \
            (vec_expr)[0] = (void*)((uintptr_t)(vec_expr)[0] + 1),             \
            true)                                                              \
         : false)

#endif  // FLYVEC_H_

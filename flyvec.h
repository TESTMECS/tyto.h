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

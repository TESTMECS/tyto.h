//! @module ty_err.h
//! 	Monadic error handling inspired by linux kernel.
#ifndef TY_ERR_H_
#define TY_ERR_H_

#include <stdbool.h>
#include <stdint.h>

#define ERR_FMT "%ld"

#ifndef TY_ERRPTR_MAX
#define TY_ERRPTR_MAX 4095
#endif  // TY_ERRPTR_MAX

typedef intptr_t err_t;

#define ERR(code) ((void*)(intptr_t)(-(err_t)(code)))

#define IS_ERR(ptr) ((uintptr_t)(ptr) >= (uintptr_t)-TY_ERRPTR_MAX)

#define IS_OK(ptr) (!IS_ERR(ptr))

#define ERR_CODE(ptr) ((err_t)(-(intptr_t)(ptr)))

#endif /* TY_ERR_H_ */

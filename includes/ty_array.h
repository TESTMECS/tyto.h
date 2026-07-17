#ifndef TY_ARRAY_H_
#define TY_ARRAY_H_

//! @ARRAY_SIZE!
//! 	Calculates the length of an array.
#define ARRAY_SIZE(...) (sizeof(__VA_ARGS__) / sizeof(*(__VA_ARGS__)))

#endif  // TY_ARRAY_H_

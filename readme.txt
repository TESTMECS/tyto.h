tyto.h
---
c functions to not write twice.

# types 

- ty_slice -> Stack allocated, ptr + len
- ty_string -> Heap allocated, ptr + len + size
- ty_flyvec -> Heap allocated, smaller size.

# io 

- ty_assert -> Pre and post conditions.
- ty_io -> Printing convience.
- ty_alloc -> wrappers around libc functions + interface, requires ty_panic/assert.


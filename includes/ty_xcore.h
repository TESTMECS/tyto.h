/// Core definitions for tyto library functions.
/// free software, discretion advised.
#ifndef TY_CORE_H_
#define TY_CORE_H_
/// /// /// /// /// /// ///
/// Includes
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/// /// /// /// /// /// ///
/// Useful macros
#define Stringify_(S)             #S
#define Stringify(S)              Stringify_(S)
#define Glue_(A, B)               A##B
#define Glue(A, B)                Glue_(A, B)
#define ArraySize(a)              (sizeof(a) / sizeof((a)[0]))
#define CeilIntegerDivision(a, b) (((a) + (b) - 1) / (b))
#define Swap(T, a, b)                                                          \
    do {                                                                       \
        T tmp_ = a;                                                            \
        a      = b;                                                            \
        b      = tmp_;                                                         \
    } while (0)
#define Min(A, B)      (((A) < (B)) ? (A) : (B))
#define Max(A, B)      (((A) > (B)) ? (A) : (B))
#define Clamp(A, X, B) (((X) < (A)) ? (A) : ((X) > (B)) ? (B) : (X))
/// /// /// /// /// /// ///
/// Compiler Conditionals.
/// Clang
//! COMPILER_CLANG
#if defined(__clang__)
#define COMPILER_CLANG 1
//! OS_WINDOWS
//! || OS_LINUX
//! || OS_MAC
#if defined(_WIN32)
#define OS_WINDOWS 1
#elif defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error "This Compiler/OS combo is not supported."
#endif
//! ARCH_X86
//! || ARCH_ARM64
//! || ARCH_ARM32
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||           \
    defined(__x86_64)
#define ARCH_X86 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error "Architecture not supported."
#endif
/// /// ///
/// MSVC
//! COMPILER_MSVC
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
//! COMPILER_MSVC_YEAR
#if _MSC_VER >= 1920
#define COMPILER_MSVC_YEAR 2019
#elif _MSC_VER >= 1910
#define COMPILER_MSVC_YEAR 2017
#elif _MSC_VER >= 1900
#define COMPILER_MSVC_YEAR 2015
#elif _MSC_VER >= 1800
#define COMPILER_MSVC_YEAR 2013
#elif _MSC_VER >= 1700
#define COMPILER_MSVC_YEAR 2012
#elif _MSC_VER >= 1600
#define COMPILER_MSVC_YEAR 2010
#elif _MSC_VER >= 1500
#define COMPILER_MSVC_YEAR 2008
#elif _MSC_VER >= 1400
#define COMPILER_MSVC_YEAR 2005
#else
#define COMPILER_MSVC_YEAR 0
#endif
//! OS_WINDOWS
#if defined(_WIN32)
#define OS_WINDOWS 1
#else
#error "This Compiler/OS combo is not supported."
#endif
//! ARCH_X86
//! || ARCH_ARM64
//! || ARCH_ARM32
#if defined(_M_AMD64)
#define ARCH_X64 1
#elif defined(_M_IX86)
#define ARCH_X86 1
#elif defined(_M_ARM64)
#define ARCH_ARM64 1
#elif defined(_M_ARM)
#define ARCH_ARM32 1
#else
#error "Architecture not supported."
#endif
/// /// ///
/// GCC
#elif defined(__GNUC__) || defined(__GNUG__)
//! COMPILER_GCC
#define COMPILER_GCC 1
//! OS_LINUX
#if defined(__gnu_linux__) || defined(__linux__)
#define OS_LINUX 1
#else
#error "This Compiler/OS combo is not supported."
#endif
//! ARCH_X86
//! || ARCH_ARM64
//! || ARCH_ARM32
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||           \
    defined(__x86_64)
#define ARCH_X86 1
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define ARCH_X86 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#elif defined(__arm__)
#define ARCH_ARM32 1
#else
#error "Architecture not supported."
#endif
#else
#error "Compiler not supported"
#endif
/// /// ///
/// Architecture Conditionals.
//! ARCH_64BIT
//! || ARCH_32BIT
#if defined(ARCH_X86)
#define ARCH_64BIT 1
#elif defined(ARCH_X86)
#define ARCH_32BIT 1
#endif
//! ARCH_LITTLE_ENDIAN
#if ARCH_ARM32 || ARCH_ARM64 || ARCH_X64 || ARCH_X86
#define ARCH_LITTLE_ENDIAN 1
#else
#error "Endianness of this architecture not supported by this library."
#endif
/// /// ///
/// Build Mode Macros.
//! BUILD_DEBUG
#if !defined(BUILD_DEBUG)
#define BUILD_DEBUG 1
#endif
//! BUILD_TESTS
#if !defined(BUILD_TESTS)
#define BUILD_TESTS 0
#endif
//! BUILD_VERSION_MAJOR
#if !defined(BUILD_VERSION_MAJOR)
#define BUILD_VERSION_MAJOR 0
#endif
//! BUILD_VERSION_MINOR
#if !defined(BUILD_VERSION_MINOR)
#define BUILD_VERSION_MINOR 0
#endif
//! BUILD_VERSION_PATCH
#if !defined(BUILD_VERSION_PATCH)
#define BUILD_VERSION_PATCH 7
#endif
//! BUILD_VERSION_STRING
#define BUILD_VERSION_STRING                                                   \
    Stringify(BUILD_VERSION_MAJOR) "." Stringify(                              \
        BUILD_VERSION_MINOR) "." Stringify(BUILD_VERSION_PATCH)
/// /// /// /// /// ///
/// Version info
#define Version(major, minor, patch)                                           \
    (uint64_t)((((uint64_t)(major) & 0xffff) << 32) |                          \
               ((((uint64_t)(minor) & 0xffff) << 16)) |                        \
               ((((uint64_t)(patch) & 0xffff) << 0)))
#define MajorVersion(version) (((version) & 0xffff00000000ull) >> 32)
#define MinorVersion(version) (((version) & 0x0000ffff0000ull) >> 16)
#define PatchVersion(version) (((version) & 0x00000000ffffull) >> 0)
/// /// /// /// /// ///
/// Undefined options.
#if !defined(ARCH_32BIT)
#define ARCH_32BIT 0
#endif
#if !defined(ARCH_64BIT)
#define ARCH_64BIT 0
#endif
#if !defined(ARCH_X64)
#define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
#define ARCH_X86 0
#endif
#if !defined(ARCH_ARM64)
#define ARCH_ARM64 0
#endif
#if !defined(ARCH_ARM32)
#define ARCH_ARM32 0
#endif
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_GCC)
#define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
#define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
#define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
#define OS_LINUX 0
#endif
#if !defined(OS_MAC)
#define OS_MAC 0
#endif
/// /// /// /// ///
/// Keywords.
//! readonly
#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
#pragma section(".rdata$", read)
#define readonly __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
#define readonly __attribute__((section(".rodata")))
#else
/// todo on gcc.
#define readonly
#endif
//! threadstatic
#if COMPILER_MSVC
#define threadstatic __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
#define threadstatic __thread
#else
#error "threadstatic not defined for this compiler."
#endif
//! forceinline
#if COMPILER_MSVC
#define foreceinline __forceinline
#elif COMPILER_CLANG || COMPILER_GCC
#define forceinline inline __attribute__((always_inline))
#else
#error "foreceinline not defined for this compiler."
#endif
//! noinline
#if COMPILER_MSVC
#define noinline __declspec(noinline)
#elif COMPILER_CLANG || COMPILER_GCC
#define noinline __attribute__((noinline))
#else
#error "noinline not defined for this compiler."
#endif
//! sharedfunction
#if OS_WINDOWS
#define sharedfunction C_LINKAGE __declspec(dllexport)
#else
#define sharedfunction C_LINKAGE
#endif
//! OPTIMIZE_BEGIN
//! OPTIMIZE_END
#if COMPILER_MSVC
#define OPTIMIZE_BEGIN _Pragma("optimize(\"\", on)")
#define OPTIMIZE_END   _Pragma("optimize(\"\", off)")
#elif COMPILER_CLANG
#define OPTIMIZE_BEGIN _Pragma("clang optimize on")
#define OPTIMIZE_END   _Pragma("clang optimize off")
#elif COMPILER_GCC
#define OPTIMIZE_BEGIN                                                         \
    _Pragma("GCC push_options") _Pragma("GCC optimize(\"O2\")")
#define OPTIMIZE_END _Pragma("GCC pop_options")
#else
#define OPTIMIZE_BEGIN
#define OPTIMIZE_END
#endif
//! NO_OPTIMIZE_BEGIN
//! NO_OPTIMIZE_END
#if COMPILER_MSVC && !BUILD_DEBUG
#define NO_OPTIMIZE_BEGIN _Pragma("optimize(\"\", off)")
#define NO_OPTIMIZE_END   _Pragma("optimize(\"\", on)")
#elif COMPILER_CLANG && !BUILD_DEBUG
#define NO_OPTIMIZE_BEGIN _Pragma("clang optimize off")
#define NO_OPTIMIZE_END   _Pragma("clang optimize on")
#elif COMPILER_GCC && !BUILD_DEBUG
#define NO_OPTIMIZE_BEGIN                                                      \
    _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#define NO_OPTIMIZE_END _Pragma("GCC pop_options")
#else
#define NO_OPTIMIZE_BEGIN
#define NO_OPTIMIZE_END
#endif
/// /// /// /// ///
/// Branch Predictor Hints.
#if defined(__clang__)
#define Expect(expr, val) __builtin_expect((expr), (val))
#else
#define Expect(expr, val) (expr)
#endif
#define Likely(expr)   Expect(expr, 1)
#define Unlikely(expr) Expect(expr, 0)
/// /// /// /// /// ///
//! AlignOf
#if COMPILER_MSVC
#define AlignOf(T) __alignof(T)
#elif COMPILER_CLANG
#define AlignOf(T) __alignof(T)
#elif COMPILER_GCC
#define AlignOf(T) __alignof__(T)
#else
#error "AlignOf not defined for this compiler."
#endif
//! AlignType
#if COMPILER_MSVC
#define AlignType(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
#define AlignType(x) __attribute__((aligned(x)))
#else
#error "AlignType not defined for this compiler."
#endif
/// /// /// /// /// ///
/// Units
#define KB(n) (((uint64_t)(n)) << 10)
#define MB(n) (((uint64_t)(n)) << 20)
#define GB(n) (((uint64_t)(n)) << 30)
#define TB(n) (((uint64_t)(n)) << 40)
/// /// /// /// /// ///
/// Member offsets.
#define Member(T, m)                  (((T*)0)->m)
#define OffsetOf(T, m)                offsetof(T, m)
#define MemberFromOffset(T, ptr, off) (T)((((uint8_t*)ptr) + (off)))
#define MemberFromPtr(T, ptr, m)      (void*)((((uint8_t*)ptr) + OffsetOf(T, m)))
#define CastFromMember(T, m, ptr)     (T*)(((uint8_t*)ptr) - OffsetOf(T, m))
/// /// /// /// /// ///
/// For loops.
#define DeferLoop(begin, end)                                                  \
    for (int i__ = ((begin), 0); !i__; i__ += 1, (end))
#define DeferLoopTill(begin, end)                                              \
    for (int i__ = 2 * !(begin); (i__ == 2 ? ((end), 0) : !i__);               \
         i__ += 1, (end))
#define ForEachIndex(it, count) (uint64_t it = 0; it < (count); it += 1)
#define ForEachElement(it, array)                                              \
    (uint64_t it = 0; it < ArraySize(array); it += 1)
#define ForEachEnum(type, it)                                                  \
    (type it = (type)0; it < type##_COUNT; it = (type)(it + 1))
#define ForEachEnumNonZero(type, it)                                           \
    (type it = (type)1; it < type##_COUNT; it = (type)(it + 1))
#define ForEachInRange(it, range)                                              \
    (uint64_t it = (range).min; it < (range).max; it += 1)
#define ForEachNode(it, T, first) (T* it = first; it != 0; it = it->next)
#define ForEachBit(it, flags)                                                  \
    (uint64_t(i__) = (flags), it = (i__) & -(i__); (i__) != 0;                 \
     (i__) &= ((i__) - 1), it    = (i__) & -(i__))
/// /// /// /// /// /// ///
/// mem
#define MemCopy(dst, src, size) memmove((dst), (src), (size))
#define MemSet(dst, byte, size) memset((dst), (byte), (size))
#define MemCompare(a, b, size)  memcmp((a), (b), (size))
#define MemCopyStruct(d, s)     MemCopy((d), (s), sizeof(*(d)))
#define MemCopyArray(d, s)      MemCopy((d), (s), sizeof(d))
#define MemCopyTyped(d, s, c)   MemCopy((d), (s), sizeof(*(d)) * (c))
#define MemCopyS8(dst, s)       MemCopy(dst, (s).data, (s).size)
#define MemZero(s, z)           memset((s), 0, (z))
#define MemZeroStruct(s)        MemZero((s), sizeof(*(s)))
#define MemZeroArray(a)         MemZero((a), sizeof(a))
#define MemZeroTyped(m, c)      MemZero((m), sizeof(*(m)) * (c))
#define MemMatch(a, b, z)       (MemCompare((a), (b), (z)) == 0)
#define MemMatchStruct(a, b)    MemMatch((a), (b), sizeof(*(a)))
#define MemMatchArray(a, b)     MemMatch((a), (b), sizeof(a))
#define MemIsZeroStruct(ptr)    isMemZero((ptr), sizeof(*(ptr)))
/// /// /// /// /// /// /// ///
/// Asserts
//! Trap
#if COMPILER_MSVC
#define Trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
#define Trap() __builtin_trap()
#else
#error "Unknown trap intrinsic for this compiler."
#endif
//! PanicIf
#define PanicIf(x)                                                             \
    do {                                                                       \
        if (!(x)) {                                                            \
            Trap();                                                            \
        }                                                                      \
    } while (0)
//! Assert
#if BUILD_DEBUG
#define Assert(x) PanicIf(x)
#else
#define Assert(x) (void)(x)
#endif

#define Unreachable           Assert(!"Unreachable!")
#define Todo                  Assert(!"Todo!")
#define Noop                  ((void)0)
#define StaticAssert(_c, _id) static uint8_t Glue(_id, __LINE__)[(_c) ? 1 : -1]
/// /// /// /// /// /// ///
/// Atomic operations.
#if COMPILER_MSVC
#include <intrin.h>
#if ARCH_X64
#define ins_atomic_u128_eval_cond_assign(x, k, c)                              \
    (int32_t)_InterlockedCompareExchange128(                                   \
        (__int64*)(x),                                                         \
        ((__int64*)&(k))[1],                                                   \
        ((__int64*)&(k))[0],                                                   \
        (__int64*)(c))
#define ins_atomic_uint64_t_eval(x)                                            \
    (uint64_t)__iso_volatile_load64((__int64*)(x))
#define ins_atomic_uint64_t_inc_eval(x) _InterlockedIncrement64((__int64*)(x))
#define ins_atomic_uint64_t_dec_eval(x) _InterlockedDecrement64((__int64*)(x))
#define ins_atomic_uint64_t_eval_assign(x, c)                                  \
    _InterlockedExchange64((__int64*)(x), (c))
#define ins_atomic_uint64_t_add_eval(x, c) _interlockedadd64((__int64*)(x), (c))
#define ins_atomic_uint64_t_eval_cond_assign(x, k, c)                          \
    _InterlockedCompareExchange64((__int64*)(x), (k), (c))
#define ins_atomic_uint32_t_eval(x)                                            \
    (uint32_t)__iso_volatile_load32((__int32*)(x))
#define ins_atomic_uint32_t_inc_eval(x) _InterlockedIncrement((long*)(x))
#define ins_atomic_uint32_t_dec_eval(x) _InterlockedDecrement((long*)(x))
#define ins_atomic_uint32_t_eval_assign(x, c)                                  \
    _InterlockedExchange((long*)(x), (c))
#define ins_atomic_uint32_t_eval_cond_assign(x, k, c)                          \
    _InterlockedCompareExchange((long*)(x), (k), (c))
#define ins_atomic_uint32_t_add_eval(x, c) _interlockedadd((long*)(x), (c))
#define ins_atomic_uint8_t_eval_assign(x, c)                                   \
    _InterlockedExchange8((char*)(x), (char)(c))
#define ins_atomic_uint8_t_or(x, c)  _InterlockedOr8((char*)(x), (char)(c))
#define ins_atomic_uint32_t_or(x, c) _InterlockedOr((long*)(x), (long)(c))
#else
#  error Atomic intrinsics not defined for this compiler / architecture combination.
#endif
#elif COMPILER_CLANG || COMPILER_GCC
#include <emmintrin.h>
#include <immintrin.h>
#define ins_atomic_u128_eval_cond_assign(x, k, c)                              \
    (int32_t)__atomic_compare_exchange_n(                                      \
        (__int128*)(x),                                                        \
        (__int128*)(c),                                                        \
        *(__int128*)(k),                                                       \
        0,                                                                     \
        __ATOMIC_SEQ_CST,                                                      \
        __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_eval(x)                                            \
    __atomic_load_n((uint64_t*)(x), __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_inc_eval(x)                                        \
    __atomic_add_fetch((uint64_t*)(x), 1, __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_dec_eval(x)                                        \
    __atomic_sub_fetch((uint64_t*)(x), 1, __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_eval_assign(x, c)                                  \
    __atomic_exchange_n(x, c, __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_add_eval(x, c)                                     \
    __atomic_add_fetch((uint64_t*)(x), c, __ATOMIC_SEQ_CST)
#define ins_atomic_uint64_t_eval_cond_assign(x, k, c)                          \
    ({                                                                         \
        uint64_t _new = (c);                                                   \
        __atomic_compare_exchange_n(                                           \
            (uint64_t*)(x),                                                    \
            &_new,                                                             \
            (k),                                                               \
            0,                                                                 \
            __ATOMIC_SEQ_CST,                                                  \
            __ATOMIC_SEQ_CST);                                                 \
        _new;                                                                  \
    })
#define ins_atomic_uint32_t_eval(x) __atomic_load_n(x, __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_inc_eval(x)                                        \
    __atomic_add_fetch((uint32_t*)(x), 1, __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_dec_eval(x)                                        \
    __atomic_sub_fetch((uint32_t*)(x), 1, __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_add_eval(x, c)                                     \
    __atomic_add_fetch((uint32_t*)(x), c, __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_eval_assign(x, c)                                  \
    __atomic_exchange_n((x), (c), __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_eval_cond_assign(x, k, c)                          \
    ({                                                                         \
        uint32_t _new = (c);                                                   \
        __atomic_compare_exchange_n(                                           \
            (uint32_t*)(x),                                                    \
            &_new,                                                             \
            (k),                                                               \
            0,                                                                 \
            __ATOMIC_SEQ_CST,                                                  \
            __ATOMIC_SEQ_CST);                                                 \
        _new;                                                                  \
    })
#define ins_atomic_uint8_t_eval_assign(x, c)                                   \
    __atomic_exchange_n((x), (c), __ATOMIC_SEQ_CST)
#define ins_atomic_uint8_t_or(x, c)                                            \
    __atomic_fetch_or((uint8_t*)(x), (uint8_t)(c), __ATOMIC_SEQ_CST)
#define ins_atomic_uint32_t_or(x, c)                                           \
    __atomic_fetch_or((uint32_t*)(x), (uint32_t)(c), __ATOMIC_SEQ_CST)
#else
#error "Atomic intrinsics not defined for this compiler / architecture."
#endif

#if ARCH_64BIT
#define ins_atomic_ptr_eval_cond_assign(x, k, c)                               \
    (void*)ins_atomic_uint64_t_eval_cond_assign(                               \
        (uint64_t*)(x), (uint64_t)(k), (U64)(c))
#define ins_atomic_ptr_eval_assign(x, c)                                       \
    (void*)ins_atomic_uint64_t_eval_assign((uint64_t*)(x), (uint64_t)(c))
#define ins_atomic_ptr_eval(x) (void*)ins_atomic_uint64_t_eval((uint64_t*)x)
#else
#error "Atomic intrinsics for pointers not defined for this architecture."
#endif
/// /// /// /// /// /// ///
/// Address Sanitizer
#if COMPILER_MSVC
#if defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED 1
#define ASAN_NO_ADDR __declspec(no_sanitize_address)
#endif
#elif COMPILER_CLANG
#if defined(__has_feature)
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED 1
#endif
#endif
#define ASAN_NO_ADDR   __attribute__((no_sanitize("address")))
#define UBSAN_NO_ALIGN __attribute__((no_sanitize("alignment")))
#endif

#ifndef ASAN_NO_ADDR
#define ASAN_NO_ADDR
#endif
#ifndef UBSAN_NO_ALIGN
#define UBSAN_NO_ALIGN
#endif

#if ASAN_ENABLED
C_LINKAGE void
__asan_poison_memory_region(void const volatile* addr, size_t size);
C_LINKAGE void
__asan_unpoison_memory_region(void const volatile* addr, size_t size);
#define AsanPoisonMemoryRegion(addr, size)                                     \
    __asan_poison_memory_region((addr), (size))
#define AsanUnpoisonMemoryRegion(addr, size)                                   \
    __asan_unpoison_memory_region((addr), (size))
#else
#define AsanPoisonMemoryRegion(addr, size)   ((void)(addr), (void)(size))
#define AsanUnpoisonMemoryRegion(addr, size) ((void)(addr), (void)(size))
#endif
/// /// /// /// /// /// ///
/// Bit macros.
#define Compose64Bit(a, b)    ((((uint64_t)a) << 32) | ((uint64_t)b))
#define Compose32Bit(a, b)    ((((uint32_t)a) << 16) | ((uint32_t)b))
#define AlignPow2(x, b)       (((x) + (b) - 1) & (~((b) - 1)))
#define AlignDownPow2(x, b)   ((x) & (~((b) - 1)))
#define AlignPadPow2(x, b)    ((0 - (x)) & ((b) - 1))
#define IsPow2(x)             ((x) != 0 && ((x) & ((x) - 1)) == 0)
#define IsPow2OrZero(x)       ((((x) - 1) & (x)) == 0)
#define ExtractBit(word, idx) (((word) >> (idx)) & 1)
#define Extract8(word, pos)   (((word) >> ((pos) * 8)) & max_uint8_t)
#define Extract16(word, pos)  (((word) >> ((pos) * 16)) & max_uint16_t)
#define Extract32(word, pos)  (((word) >> ((pos) * 32)) & max_uint32_t)
#endif

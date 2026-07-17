//! @module ty_log.h
//! 	General print helper macros.
#ifndef TY_LOG_H_
#define TY_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#define TYTO_PREFIX "[TYTO] "

//! @macro puts
//! 	helpers for printing values.
#define putsf(...)                                                             \
    {                                                                          \
        fprintf(stderr, TYTO_PREFIX "%s:%d: ", __func__, __LINE__);            \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    }

//! @Logger
enum TytoLogKind
{
    LOG_DEBUG,
    LOG_NOTE,
    LOG_WARNING,
    LOG_ERROR,
};

static const char* level_name[] = {
    [LOG_DEBUG]   = "debug",
    [LOG_NOTE]    = "note",
    [LOG_WARNING] = "warning",
    [LOG_ERROR]   = "error",
};

static const char* level_color[] = {
    [LOG_DEBUG]   = "\x1b[1m",
    [LOG_NOTE]    = "\x1b[1;96m",
    [LOG_WARNING] = "\x1b[1;95m",
    [LOG_ERROR]   = "\x1b[1;91m",
};

//! Debug Macro.
#define dbg(...) tyto_log(LOG_DEBUG, __VA_ARGS__)

//! Logging function.
static void
tyto_log(enum TytoLogKind kind, const char* fmt, ...);

#endif  // TY_LOG_H_

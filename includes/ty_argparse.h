#ifndef TY_ARGPARSE_H_
#define TY_ARGPARSE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum arg_kind
{
    ARG_KIND_SHORT,
    ARG_KIND_LONG,
    ARG_KIND_POSITIONAL,
    ARG_KIND_DONE,
} arg_kind_t;

typedef struct arg
{
    arg_kind_t kind;
    union
    {
        char  as_char;
        char* as_string;
    };
} arg_t;

#define ARG_SHORT(arg, c) ((arg).kind == ARG_KIND_SHORT && (arg).as_char == c)
#define ARG_LONG(arg, s)                                                       \
    ((arg).kind == ARG_KIND_LONG && !strcmp((arg).as_string, s))
#define ARG_POS(arg)  ((arg).kind == ARG_KIND_POSITIONAL)
#define ARG_DONE(arg) ((arg).kind == ARG_KIND_DONE)

typedef struct arg_ctxt
{
    bool   only_pos;
    int    short_index;
    char** args;
} arg_ctxt_t;

static void
arg_error(arg_t arg, const char* err, const char* usage)
{
    if (err != NULL) {
        switch (arg.kind) {
            case ARG_KIND_SHORT:
                fprintf(stderr, "option '-%c': %s\n", arg.as_char, err);
                break;
            case ARG_KIND_LONG:
                fprintf(stderr, "option '--%s': %s\n", arg.as_string, err);
                break;
            case ARG_KIND_POSITIONAL:
                fprintf(stderr, "argument '%s': %s\n", arg.as_string, err);
                break;
            case ARG_KIND_DONE:
                fprintf(stderr, "unexpected end of arguments\n");
                break;
        }
    }
    fprintf(err != NULL ? stderr : stdout, "usage: %s\n", usage);
    exit(err != NULL ? 1 : 0);
}

static arg_ctxt_t
arg_init(char** argv)
{
    return (arg_ctxt_t){
        .only_pos    = false,
        .short_index = 0,
        .args        = argv[0] == NULL ? argv : argv + 1,
    };
}

static char*
arg_next_value(arg_ctxt_t* ctxt)
{
    if (ctxt->short_index > 0) {
        int a             = ctxt->short_index;
        ctxt->short_index = 0;
        return (*ctxt->args++) + a;
    }
    if (*ctxt->args == NULL)
        return NULL;
    return *ctxt->args++;
}

static arg_t
arg_next(arg_ctxt_t* ctxt)
{
start_label:
    if (*ctxt->args == NULL)
        return (arg_t){.kind = ARG_KIND_DONE};
    if (ctxt->only_pos)
        return (arg_t){.kind = ARG_KIND_POSITIONAL, .as_string = *ctxt->args++};
    if (ctxt->short_index > 0) {
        arg_t ret = (arg_t){.kind    = ARG_KIND_SHORT,
                            .as_char = (*ctxt->args)[ctxt->short_index++]};
        if ((*ctxt->args)[ctxt->short_index] == '\0') {
            ctxt->args += 1;
            ctxt->short_index = 0;
        }
        return ret;
    }
    if (**ctxt->args == '-') {
        if ((*ctxt->args)[1] == '\0')
            return (arg_t){.kind      = ARG_KIND_POSITIONAL,
                           .as_string = *ctxt->args++};
        if ((*ctxt->args)[1] == '-') {
            if ((*ctxt->args)[2] == '\0') {
                ctxt->only_pos = true;
                ctxt->args += 1;
                goto start_label;
            }
            char* a = *ctxt->args + 2;
            ctxt->args += 1;
            return (arg_t){.kind = ARG_KIND_LONG, .as_string = a};
        }
        ctxt->short_index = 1;
        goto start_label;
    }
    return (arg_t){.kind = ARG_KIND_POSITIONAL, .as_string = *ctxt->args++};
}

#endif  // TY_ARGPARSE_H_

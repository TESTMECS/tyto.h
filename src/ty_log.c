#include <ty_log.h>

//! @Logging
static void
tyto_log(enum TytoLogKind kind, const char* fmt, ...)
{
    fprintf(stderr, "%s%s:\x1b[0m ", level_color[kind], level_name[kind]);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

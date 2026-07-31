//! @file ty_argparse.c
//! 	Example of nicer `getopt`
#include <ty_argparse.h>

static char* USAGE = "tyto.test [-a] [-s/--string STR] [-t/--test] ARG...";

int
main(int argc, char** argv)
{
    if (argc <= 1) {
        printf("usage: %s\n", USAGE);
        return 1;
    }

    //! Initialize the context.
    arg_ctxt_t ctxt = arg_init(argv);
    //! The loop variable.
    arg_t arg;

    //! Consume each arg.
    while (!ARG_DONE(arg = arg_next(&ctxt))) {
        //! short flag.
        if (ARG_SHORT(arg, 'a'))
            printf("option -a passed\n");
        //! short and long.
        else if (ARG_SHORT(arg, 's') || ARG_LONG(arg, "string")) {
            //! grab the value
            char* s = arg_next_value(&ctxt);
            //! throw the error.
            if (s == NULL)
                arg_error(arg, "requires argument.", USAGE);
            //! Use the value.
            printf("option -s/--string is: %s\n", s);
            //! short and long flag.
        } else if (ARG_LONG(arg, "test") || ARG_SHORT(arg, 't'))
            printf("option --test or -t passed!\n");
        //! positional args that come after the other ones.
        else if (ARG_POS(arg))
            printf("positional argument: %s\n", arg.as_string);
        //! otherwise.
        else
            arg_error(arg, "unknown option.", USAGE);
    }
}

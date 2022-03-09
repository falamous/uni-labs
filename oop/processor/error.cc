#include <cstdarg>
#include "error.h"

AsmException::AsmException(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    int res = vasprintf(&text, fmt, ap);
    va_end(ap);

    if (res == -1) {
        /* we probably ran out of memory */
        text = NULL;
    }
}

const char * AsmException::what () const throw () {
    return text;
}

AsmException::~AsmException() {
    free(text);
}

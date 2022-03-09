#pragma once
#include <stdexcept>

class AsmException : public std::exception {
    char *text;

    public:
    AsmException(const char *fmt, ...);
    const char * what () const throw ();
    ~AsmException();
};

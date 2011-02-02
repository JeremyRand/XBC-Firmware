// -*-c++-*-
#ifndef __STRING_PRINTF__
#define __STRING_PRINTF__

#include <stdarg.h>
#include <string>

#ifdef __GNUC__
#define _STRING_PRINTF_ATTRIB __attribute__((format(printf,1,2)))
#define _STRING_VPRINTF_ATTRIB __attribute__((format(printf,1,0)))
#else   // __GNUC__
#define _STRING_PRINTF_ATTRIB
#define _STRING_VPRINTF_ATTRIB
#endif  // __GNUC__

std::string string_vprintf(const char *fmt, va_list args)
    _STRING_VPRINTF_ATTRIB;
std::string string_printf(const char *fmt, ...)
    _STRING_PRINTF_ATTRIB;

#endif // __string_printf__

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:


#include "rccrsv.h"

// void
// eprintf(const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);
//   vfprintf(stderr, fmt, ap);
// }

// void
// error(stderr, const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);
//   eprintf(fmt, ap);
//   eprintf("\n");
//   exit(1);
// }

// void
// eprintf_at(const char* loc, const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);

//   int pos = loc - user_input;
//   eprintf("%s\n", user_input);
//   eprintf("%*s", pos, " ");
//   eprintf("^ ");
//   eprintf(fmt, ap);
// }

// void
// error_at(const char* loc, const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);

//   eprintf_at(loc, fmt, ap);
//   eprintf("\n");
//   exit(1);
// }

// void
// eprintf_at_until(const char* loc, size_t len, const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);

//   char* underline = malloc(len + 1);
//   memset(underline, '^', len);
//   underline[len] = '\0';

//   int pos = loc - user_input;
//   eprintf("%s\n", user_input);
//   eprintf("%*s", pos, " ");
//   eprintf("%s", underline);
//   eprintf(" ");
//   eprintf(fmt, ap);
// }

// void
// error_at_until(const char* loc, size_t len, const char* fmt, ...)
// {
//   va_list ap;
//   va_start(ap, fmt);
//   eprintf_at_until(loc, len, fmt, ap);
//   eprintf("\n");
//   exit(1);
// }

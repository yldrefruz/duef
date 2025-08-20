#include "duef_printing.h"

int g_is_verbose = 0;

void print_verbose(const char *format, ...)
{
  if (!g_is_verbose)
    return;
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

void print_debug(const char *format, ...)
{
#if defined(NDEBUG) || defined(DEBUG)
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
#endif
}
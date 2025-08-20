#ifndef PRINTING_H
#define PRINTING_H
#include <stdarg.h>
#include <stdio.h>

extern int g_is_verbose;

// Function declarations
void print_verbose(const char *format, ...);
void print_debug(const char *format, ...);

#endif
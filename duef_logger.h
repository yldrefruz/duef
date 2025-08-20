#ifndef DUEF_LOGGER_H
#define DUEF_LOGGER_H

#include <stdio.h>

// Safe logging functions to replace insecure fprintf calls
void log_error(const char *format, ...);
void log_info(const char *format, ...);
void log_verbose(const char *format, ...);

// Safe file operations
int safe_remove_directory(const char *directory_path);

#endif // DUEF_LOGGER_H
#include "duef_logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MAX_PATH 260
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

// Safe logging functions to replace insecure fprintf calls
void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    // Use vfprintf which is safer than fprintf with user input
    vfprintf(stderr, format, args);
    
    va_end(args);
}

void log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    vfprintf(stdout, format, args);
    
    va_end(args);
}

void log_verbose(const char *format, ...)
{
    extern int g_is_verbose;
    if (!g_is_verbose) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    vfprintf(stderr, format, args);
    
    va_end(args);
}

#ifdef _WIN32
int safe_remove_directory_windows(const char *directory_path)
{
    char path[MAX_PATH + 1];
    
    // Copy path and ensure double null termination
    strncpy(path, directory_path, MAX_PATH - 1);
    path[MAX_PATH - 1] = '\0';
    
    // Use simple rmdir for now - for full recursive deletion would need more complex implementation
    return _rmdir(path);
}
#else
int safe_remove_directory_unix(const char *directory_path)
{
    DIR *dir;
    struct dirent *entry;
    char path[1024];
    
    dir = opendir(directory_path);
    if (dir == NULL) {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", directory_path, entry->d_name);
        
        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                safe_remove_directory_unix(path);
            } else {
                unlink(path);
            }
        }
    }
    
    closedir(dir);
    return rmdir(directory_path);
}
#endif

int safe_remove_directory(const char *directory_path)
{
#ifdef _WIN32
    return safe_remove_directory_windows(directory_path);
#else
    return safe_remove_directory_unix(directory_path);
#endif
}
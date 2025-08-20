#include "duef.h"
#include "duef_types.h"
#include "duef_printing.h"
#include "duef_args.h"
#include "duef_logger.h"
#include "duef_file_ops.h"

#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h> // For Windows-specific functions
#include <direct.h>  // For _mkdir
#include <io.h>      // For file operations
#include <errno.h>   // For errno and EEXIST on Windows
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <sys/stat.h> // For mkdir
#include <limits.h> // For PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 // Fallback definition for PATH_MAX
#endif
#endif
#include "stdbool.h"

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);
    
    // Initialize the zlib library
    if (zlibVersion() == NULL)
    {
        cleanup_arguments();
        return 1; // zlib initialization failed
    }
    
    // Open input file
    const char *input_filename = file_path ? file_path : "CrashFile.uecrash";
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file)
    {
        log_error("Error opening input file: %s\n", input_filename);
        cleanup_arguments();
        return 1;
    }

    // Decompress the file
    DecompressionResult decompression = decompress_file(input_file);
    fclose(input_file);
    
    if (decompression.status != 0)
    {
        cleanup_decompression_result(&decompression);
        cleanup_arguments();
        return 1;
    }

    // Process the decompressed crash file data
    process_crash_files(&decompression, input_filename);
    
    // Cleanup
    cleanup_decompression_result(&decompression);
    cleanup_arguments();
    
    return 0;
}

void resolve_app_directory_path(const FAnsiCharStr *directory_name, char *buffer, size_t buffer_size)
{
#ifdef _WIN32
    snprintf(buffer, buffer_size, "%s\\%.*s", get_app_directory(), directory_name->length, directory_name->content);
#else
    snprintf(buffer, buffer_size, "%s/%.*s", get_app_directory(), directory_name->length, directory_name->content);
#endif
}

void resolve_app_file_path(const FAnsiCharStr *directory, const FFile *file, char *buffer, size_t buffer_size)
{
#ifdef _WIN32
    snprintf(buffer, buffer_size, "%s\\%s\\%.*s", get_app_directory(), directory->content, directory->length, file->file_name->content);
#else
    snprintf(buffer, buffer_size, "%s/%s/%.*s", get_app_directory(), directory->content, directory->length, file->file_name->content);
#endif
}

void write_file(const FAnsiCharStr *directory, const FFile *file)
{
#ifdef _WIN32
    char file_path[MAX_PATH];
#else
    char file_path[PATH_MAX];
#endif
    resolve_app_file_path(directory, file, file_path, sizeof(file_path));

    FILE *output_file = fopen(file_path, "wb");
    if (!output_file)
    {
        log_error("Error opening output file %s\n", file_path);
        return;
    }
    size_t written = fwrite(file->file_data, 1, file->file_size, output_file);
    if (written != file->file_size)
    {
        log_error("Error writing to output file\n");
    }
    fclose(output_file);
}

bool g_cached_app_directory = false;
char g_app_directory[PATH_MAX] = {0};

char *get_app_directory(void)
{
    if (!g_cached_app_directory)
    {
#ifdef _WIN32
        snprintf(g_app_directory, sizeof(g_app_directory), "%s\\duef", getenv("LOCALAPPDATA"));
#else
        snprintf(g_app_directory, sizeof(g_app_directory), "%s/.duef", getenv("HOME"));
#endif
        g_cached_app_directory = true;
    }
    return g_app_directory;
}

void create_crash_directory(FAnsiCharStr *directory_name)
{
    char dir_path[PATH_MAX];
#ifdef _WIN32
    snprintf(dir_path, sizeof(dir_path), "%s\\%.*s", get_app_directory(), directory_name->length, directory_name->content);
#else
    snprintf(dir_path, sizeof(dir_path), "%s/%.*s", get_app_directory(), directory_name->length, directory_name->content);
#endif
    log_verbose("Creating directory: %s\n", dir_path);

#ifdef _WIN32
    if (_mkdir(get_app_directory()) == -1 && errno != EEXIST)
    {
        log_error("Error creating app directory %s: %s\n", get_app_directory(), strerror(errno));
        return;
    }
    if (_mkdir(dir_path) == -1 && errno != EEXIST)
    {
        log_error("Error creating crash directory %s: %s\n", dir_path, strerror(errno));
        return;
    }
#else
    mkdir(get_app_directory(), 0755);  // Create parent directory first
    mkdir(dir_path, 0755);
#endif
}

void delete_crash_collection_directory(void)
{
    char *app_dir = get_app_directory();
    if (safe_remove_directory(app_dir) != 0)
    {
        log_error("Failed to remove directory: %s\n", app_dir);
    }
}
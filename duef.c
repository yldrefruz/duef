#include "duef.h"
#include "duef_types.h"
#include "duef_printing.h"

#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h> // For Windows-specific functions
#include <direct.h>  // For _mkdir
#include <io.h>      // For file operations
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <sys/stat.h> // For mkdir
#include <limits.h> // For PATH_MAX
#endif
#include <errno.h>
#include "stdbool.h"

int g_is_verbose = false;
int g_print_mode_file = false;
char *file_path = NULL;

void print_usage(const char *program_name);

void parse_arguments(int argc, char **argv)
{
    // add support for gnu style combinable options
    for (int i = 1; i < argc; i++)
    {

        if (argv[i][0] == '-' && argv[i][1] != '-')
        {
            bool exit_j_loop = false; // Flag to check if the next argument is expected
            // the combinable options here
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                if (exit_j_loop)
                    break; // If we are expecting the next argument, break out of the loop
                switch (argv[i][j])
                {
                case 'v':
                    g_is_verbose = true;
                    print_verbose("Verbose mode enabled.\n");
                    break;
                case 'f':
                    if (i + 1 < argc)
                    {
                        file_path = strdup(argv[++i]);
                        print_verbose("File path set to: %s\n", file_path);
                        if (!file_path)
                        {
                            fprintf(stderr, "Memory allocation failed for file path\n");
                            exit(EXIT_FAILURE);
                        }
                        exit_j_loop = true; // Set the flag to true to skip the next argument
                    }
                    else
                    {
                        fprintf(stderr, "Option -f requires an argument\n\n");
                        print_usage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'i':
                    g_print_mode_file = true;
                    print_verbose("Print mode file enabled.\n");
                    break;
                case 'h':
                    print_usage(argv[0]);
                    exit(EXIT_SUCCESS);
                    break;
                default:
                    fprintf(stderr, "Unknown option: -%c\n\n", argv[i][j]);
                    print_usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            // handle long options
            if (strcmp(argv[i], "--verbose") == 0)
            {
                g_is_verbose = true;
                print_verbose("Verbose mode enabled.\n");
            }
            else if (strcmp(argv[i], "--file") == 0)
            {
                if (i + 1 < argc)
                {
                    file_path = strdup(argv[++i]);
                    print_verbose("File path set to: %s\n", file_path);
                    if (!file_path)
                    {
                        fprintf(stderr, "Memory allocation failed for file path\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    fprintf(stderr, "Option --file requires an argument\n\n");
                    print_usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (strcmp(argv[i], "--help") == 0)
            {
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "--clean") == 0)
            {
                // clear the crash directory by deleting all files in the crash directory
                delete_crash_collection_directory();
                print_verbose("Crash collection directory cleared.\n");
                exit(EXIT_SUCCESS);
                return;
            }
            else
            {
                fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Handle positional arguments (non-option arguments)
            if (file_path == NULL)
            {
                file_path = strdup(argv[i]);
                if (!file_path)
                {
                    fprintf(stderr, "Memory allocation failed for file path\n");
                    exit(EXIT_FAILURE);
                }
                print_verbose("File path set to: %s\n", file_path);
            }
            else
            {
                fprintf(stderr, "Multiple file arguments provided. Only one file can be processed at a time.\n\n");
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void cleanup_arguments()
{
    if (file_path)
    {
        free(file_path);
        file_path = NULL;
    }
}

void print_usage(const char *program_name)
{
    printf("duef - Unreal Engine Crash File Decompressor\n\n");
    printf("Usage: %s [OPTIONS] [file]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help        Show this help message and exit\n");
    printf("  -v, --verbose     Enable verbose output to stderr\n");
    printf("  -f, --file FILE   Specify .uecrash file to process\n");
    printf("  -i                Print individual file paths instead of directory path\n");
    printf("      --clean       Remove all extracted files from ~/.duef directory\n\n");
    printf("Examples:\n");
    printf("  %s CrashReport.uecrash     # Decompress crash file\n", program_name);
    printf("  %s -v -f crash.uecrash     # Decompress with verbose output\n", program_name);
    printf("  %s -i crash.uecrash        # Print individual file paths\n", program_name);
    printf("  %s --clean                 # Clean up extracted files\n\n", program_name);
    printf("Output:\n");
    printf("  On Unix: Files extracted to ~/.duef/<directory>/\n");
    printf("  On Windows: Files extracted to %%LocalAppData%%\\duef\\<directory>\\\n");
    printf("  Default file: CrashFile.uecrash (if no file specified)\n");
}

void write_file(const FAnsiCharStr *directory, const FFile *file);

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);
    // Initialize the zlib library
    if (zlibVersion() == NULL)
    {
        return 1; // zlib initialization failed
    }
    // decompress input file.
    FILE *input_file = fopen(file_path ? file_path : "CrashFile.uecrash", "rb");
    if (!input_file)
    {
        fprintf(stderr, "Error opening input file: %s\n", file_path ? file_path : "CrashFile.uecrash");
        return 1; // File open failed
    }

    z_stream strm = {0};
    if (inflateInit(&strm) != Z_OK)
    {
        fprintf(stderr, "Failed to initialize zlib stream\n");
        fclose(input_file);
        return 1; // Initialization failed
    }

    unsigned char in[4096];
    unsigned char out[4096];
    int ret;
    size_t total_out = 0;
    size_t buffer_size = 4096;
    unsigned char *decompressed = malloc(buffer_size);
    if (!decompressed)
    {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(input_file);
        inflateEnd(&strm);
        exit(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    do
    {
        strm.avail_in = fread(in, 1, sizeof(in), input_file);
        if (ferror(input_file))
        {
            fprintf(stderr, "Error reading input file\n");
            inflateEnd(&strm);
            fclose(input_file);
            free(decompressed);
            return 1;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        do
        {
            strm.avail_out = sizeof(out);
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                fprintf(stderr, "Decompression error\n");
                inflateEnd(&strm);
                fclose(input_file);
                free(decompressed);
                return 1;
            }
            size_t have = sizeof(out) - strm.avail_out;
            if (total_out + have > buffer_size)
            {
                buffer_size = (total_out + have) * 2;
                unsigned char *tmp = realloc(decompressed, buffer_size);
                if (!tmp)
                {
                    fprintf(stderr, "Memory reallocation failed\n");
                    inflateEnd(&strm);
                    fclose(input_file);
                    free(decompressed);
                    return 1;
                }
                decompressed = tmp;
            }
            memcpy(decompressed + total_out, out, have);
            total_out += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    fclose(input_file);

    if (ret != Z_STREAM_END)
    {
        fprintf(stderr, "Incomplete decompression\n");
        free(decompressed);
        return 1;
    }

    // decompressed data is in 'decompressed', size is 'total_out'
    print_verbose("Decompression successful. Decompressed size: %zu bytes\n", total_out);
    uint8_t *cursor = decompressed; // the main decompressed data pointer shouldn't move
    // Use decompressed data here...
    FUECrashFile *read_file = UECrashFile_Read(&cursor);
    print_verbose("File header version: %d.%d.%d\n", read_file->file_header->version[0], read_file->file_header->version[1], read_file->file_header->version[2]);
    print_verbose("Directory name: %s\n", read_file->file_header->directory_name);
    print_verbose("File name: %s\n", read_file->file_header->file_name);
    print_verbose("Uncompressed size: %d bytes\n", read_file->file_header->uncompressed_size);
    print_verbose("File count: %d\n", read_file->file_header->file_count);
    create_crash_directory(read_file->file_header->directory_name);
    print_verbose("Files in the crash report:\n");
    int files_combine_length = 0;
    // 1 MB buffer for combined file names, a little little (very little) bit larger than needed
    // Windows won't event allow this large of a file name, so it should be fine. Also Unreal Engine generally won't output much files.
    // generally unreal gives 4 files, but it can be as much as 6 if the old files are still produced.
    char files_combine_buffer[1024 * 24];
    memset(files_combine_buffer, 0, sizeof(files_combine_buffer)); // Initialize the buffer to zero
    for (int i = 0; i < read_file->file_header->file_count; i++)
    {
        print_verbose("- File %d: %.*s, size: %d bytes\n", i + 1, read_file->file[i].file_name->length, read_file->file[i].file_name->content, read_file->file[i].file_size);
        write_file(read_file->file_header->directory_name, &read_file->file[i]);
        if (g_print_mode_file)
        {
            char file_buffer[2048];
            resolve_app_file_path(read_file->file_header->directory_name, &read_file->file[i], file_buffer, sizeof(file_buffer));
            if (i > 0)
            {
                strncat(files_combine_buffer, " ", sizeof(files_combine_buffer) - strlen(files_combine_buffer) - 1);
            }
            if (strchr(file_buffer, ' ') != NULL || strchr(file_buffer, '\n') != NULL || strchr(file_buffer, '\t') != NULL)
            {
                // If the file path contains spaces or newlines, wrap it in quotes
                char temp_buffer[2052]; // Larger buffer to accommodate quotes
                size_t path_len = strlen(file_buffer);
                if (path_len < sizeof(temp_buffer) - 3) // -3 for quotes and null terminator
                {
                    (void)snprintf(temp_buffer, sizeof(temp_buffer), "\"%s\"", file_buffer);
                    strncpy(file_buffer, temp_buffer, sizeof(file_buffer) - 1);
                    file_buffer[sizeof(file_buffer) - 1] = '\0';
                }
            }
            strncat(files_combine_buffer, file_buffer, sizeof(files_combine_buffer) - strlen(files_combine_buffer) - 1);
            files_combine_length += strlen(file_buffer) + 1; // +1 for the space or null terminator
        }
    }
    if (g_print_mode_file)
    {
        printf("%s\n", files_combine_buffer); // write to the standard output for piping
    }
    else
    {
        char directory_path[2048];
        resolve_app_directory_path(read_file->file_header->directory_name, directory_path, sizeof(directory_path));
        printf("%s\n", directory_path); // print the directory path to the standard output for piping
    }
    (void)fflush(stdout); // Ensure the output is flushed immediately
    free(decompressed);
    decompressed = NULL;
    UECrashFile_Destroy(read_file);
    cleanup_arguments();
    print_verbose("All files written successfully.\n");
    return 0; // Successful execution
}

void resolve_app_directory_path(const FAnsiCharStr *directory_name, char *buffer, size_t buffer_size)
{
#ifdef _WIN32
    snprintf(buffer, buffer_size, "%s\\%.*s", get_app_directory(), directory_name->length, directory_name->content);
#else
    (void)snprintf(buffer, buffer_size, "%s/%.*s", get_app_directory(), directory_name->length, directory_name->content);
#endif
}

void resolve_app_file_path(const FAnsiCharStr *directory, const FFile *file, char *buffer, size_t buffer_size)
{
#ifdef _WIN32
    snprintf(buffer, buffer_size, "%s\\%s\\%.*s", get_app_directory(), directory->content, directory->length, file->file_name->content);
#else
    (void)snprintf(buffer, buffer_size, "%s/%s/%.*s", get_app_directory(), directory->content, directory->length, file->file_name->content);
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
        fprintf(stderr, "Error opening output file %s\n", file_path);
        return;
    }
    size_t written = fwrite(file->file_data, 1, file->file_size, output_file);
    if (written != file->file_size)
    {
        fprintf(stderr, "Error writing to output file\n");
    }
    (void)fclose(output_file);
}
bool g_cached_app_directory = false;
char g_app_directory[PATH_MAX] = {0};

char *get_app_directory()
{
    if (!g_cached_app_directory)
    {
#ifdef _WIN32
        (void)snprintf(g_app_directory, sizeof(g_app_directory), "%s\\duef", getenv("LOCALAPPDATA"));
#else
        (void)snprintf(g_app_directory, sizeof(g_app_directory), "%s/.duef", getenv("HOME"));
#endif
    }
    return g_app_directory;
}

// maybe make it so we use the mkdir command with system() instead of using the mkdir function directly
void create_crash_directory(FAnsiCharStr *directory_name)
{
    char dir_path[PATH_MAX];
#ifdef _WIN32
    (void)snprintf(dir_path, sizeof(dir_path), "%s\\%.*s", get_app_directory(), directory_name->length, directory_name->content);
#else
    (void)snprintf(dir_path, sizeof(dir_path), "%s/%.*s", get_app_directory(), directory_name->length, directory_name->content);
#endif
    print_verbose("Creating directory: %s\n", dir_path);

#ifdef _WIN32
    if (_mkdir(get_app_directory()) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "Error creating app directory %s: %s\n", get_app_directory(), strerror(errno));
        return;
    }
    if (_mkdir(dir_path) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "Error creating crash directory %s: %s\n", dir_path, strerror(errno));
        return;
    }
#else
    mkdir(get_app_directory(), 0755);  // Create parent directory first
    mkdir(dir_path, 0755);
#endif
}

// deletes the %LOCALPPDATA%\duef or ~/.duef/ directory
void delete_crash_collection_directory()
{
#ifdef _WIN32
    char command_buffer[1024];
    (void)snprintf(command_buffer, sizeof(command_buffer), "rmdir /S /Q \"%s\"", get_app_directory());
    system(command_buffer);
#else
    char command_buffer[1024];
    (void)snprintf(command_buffer, sizeof(command_buffer), "rm -r \"%s\"", get_app_directory());
    system(command_buffer); // no forcing here, be careful to not remove root
#endif
}
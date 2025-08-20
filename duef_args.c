#include "duef_args.h"
#include "duef_printing.h"
#include "duef_logger.h"
#include "duef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables for command line arguments
extern int g_is_verbose;
int g_print_mode_file = false;
char *file_path = NULL;

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

void process_file_option(int *i, int argc, char **argv)
{
    if (*i + 1 < argc)
    {
        file_path = strdup(argv[++(*i)]);
        print_verbose("File path set to: %s\n", file_path);
        if (!file_path)
        {
            log_error("Memory allocation failed for file path\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        log_error("Option -f requires an argument\n\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void handle_single_short_option(char option, int *i, int argc, char **argv, bool *exit_j_loop)
{
    switch (option)
    {
    case 'v':
        g_is_verbose = true;
        print_verbose("Verbose mode enabled.\n");
        break;
    case 'f':
        process_file_option(i, argc, argv);
        *exit_j_loop = true;
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
        log_error("Unknown option: -%c\n\n", option);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void handle_short_options(char *arg, int *i, int argc, char **argv)
{
    bool exit_j_loop = false;
    
    for (int j = 1; arg[j] != '\0'; j++)
    {
        if (exit_j_loop) {
            break;
        }
        handle_single_short_option(arg[j], i, argc, argv, &exit_j_loop);
    }
}

void handle_verbose_option(void)
{
    g_is_verbose = true;
    print_verbose("Verbose mode enabled.\n");
}

void handle_file_long_option(int *i, int argc, char **argv)
{
    if (*i + 1 < argc)
    {
        file_path = strdup(argv[++(*i)]);
        print_verbose("File path set to: %s\n", file_path);
        if (!file_path)
        {
            log_error("Memory allocation failed for file path\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        log_error("Option --file requires an argument\n\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void handle_clean_option(void)
{
    delete_crash_collection_directory();
    print_verbose("Crash collection directory cleared.\n");
    exit(EXIT_SUCCESS);
}

void handle_long_options(char *arg, int *i, int argc, char **argv)
{
    if (strcmp(arg, "--verbose") == 0)
    {
        handle_verbose_option();
    }
    else if (strcmp(arg, "--file") == 0)
    {
        handle_file_long_option(i, argc, argv);
    }
    else if (strcmp(arg, "--help") == 0)
    {
        print_usage(argv[0]);
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(arg, "--clean") == 0)
    {
        handle_clean_option();
    }
    else
    {
        log_error("Unknown option: %s\n\n", arg);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void handle_positional_argument(char *arg)
{
    if (file_path == NULL)
    {
        file_path = strdup(arg);
        if (!file_path)
        {
            log_error("Memory allocation failed for file path\n");
            exit(EXIT_FAILURE);
        }
        print_verbose("File path set to: %s\n", file_path);
    }
    else
    {
        log_error("Multiple file arguments provided. Only one file can be processed at a time.\n\n");
        print_usage("duef");
        exit(EXIT_FAILURE);
    }
}

void parse_arguments(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] != '-')
        {
            handle_short_options(argv[i], &i, argc, argv);
        }
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            handle_long_options(argv[i], &i, argc, argv);
        }
        else
        {
            handle_positional_argument(argv[i]);
        }
    }
}

void cleanup_arguments(void)
{
    if (file_path)
    {
        free(file_path);
        file_path = NULL;
    }
}
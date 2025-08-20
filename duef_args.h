#ifndef DUEF_ARGS_H
#define DUEF_ARGS_H

#include <stdbool.h>

// Global variables for command line arguments
extern int g_is_verbose;
extern int g_print_mode_file;
extern char *file_path;

// Function declarations for argument parsing
void parse_arguments(int argc, char **argv);
void cleanup_arguments(void);
void print_usage(const char *program_name);

// Helper functions to reduce cognitive complexity
void handle_short_options(char *arg, int *i, int argc, char **argv);
void handle_long_options(char *arg, int *i, int argc, char **argv);
void handle_positional_argument(char *arg);
void process_file_option(int *i, int argc, char **argv);

#endif // DUEF_ARGS_H
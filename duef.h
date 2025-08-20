#ifndef DUEF_H
#define DUEF_H
#include "duef_types.h"

char *get_app_directory(void);
void resolve_app_directory_path(const FAnsiCharStr *directory_name, char *buffer, size_t buffer_size);
void resolve_app_file_path(const FAnsiCharStr *directory, const FFile *file, char *buffer, size_t buffer_size);
void write_file(const FAnsiCharStr *directory, const FFile *file);

void create_crash_directory(FAnsiCharStr *directory_name);
void delete_crash_collection_directory(void);

#endif
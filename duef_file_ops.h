#ifndef DUEF_FILE_OPS_H
#define DUEF_FILE_OPS_H

#include "duef_types.h"
#include <stdio.h>
#include <stddef.h>

// File decompression functions
typedef struct {
    unsigned char *data;
    size_t size;
    int status;
} DecompressionResult;

DecompressionResult decompress_file(FILE *input_file);
void cleanup_decompression_result(DecompressionResult *result);

// File processing functions
void process_crash_files(const DecompressionResult *decompression, const char *input_filename);
void output_results(const FUECrashFile *crash_file);

#endif // DUEF_FILE_OPS_H
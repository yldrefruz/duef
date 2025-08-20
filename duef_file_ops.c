#include "duef_file_ops.h"
#include "duef_args.h"
#include "duef_logger.h"
#include "duef_printing.h"
#include "duef.h"
#include "zlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

DecompressionResult decompress_file(FILE *input_file)
{
    DecompressionResult result = {NULL, 0, 1}; // Initialize with error status
    
    z_stream strm = {0};
    if (inflateInit(&strm) != Z_OK)
    {
        log_error("Failed to initialize zlib stream\n");
        return result;
    }

    unsigned char input_buffer[4096];
    unsigned char out[4096];
    int ret = Z_OK; // Initialize ret variable to fix garbage value warning
    size_t total_out = 0;
    size_t buffer_size = 4096;
    unsigned char *decompressed = malloc(buffer_size);
    
    if (!decompressed)
    {
        log_error("Memory allocation failed\n");
        inflateEnd(&strm);
        return result;
    }

    do
    {
        strm.avail_in = fread(input_buffer, 1, sizeof(input_buffer), input_file);
        if (ferror(input_file))
        {
            log_error("Error reading input file\n");
            inflateEnd(&strm);
            free(decompressed);
            return result;
        }
        if (strm.avail_in == 0) {
            break;
        }
        strm.next_in = input_buffer;

        do
        {
            strm.avail_out = sizeof(out);
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                log_error("Decompression error\n");
                inflateEnd(&strm);
                free(decompressed);
                return result;
            }
            size_t have = sizeof(out) - strm.avail_out;
            if (total_out + have > buffer_size)
            {
                buffer_size = (total_out + have) * 2;
                unsigned char *tmp = realloc(decompressed, buffer_size);
                if (!tmp)
                {
                    log_error("Memory reallocation failed\n");
                    inflateEnd(&strm);
                    free(decompressed);
                    return result;
                }
                decompressed = tmp;
            }
            memcpy(decompressed + total_out, out, have);
            total_out += have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);

    if (ret != Z_STREAM_END)
    {
        log_error("Incomplete decompression\n");
        free(decompressed);
        return result;
    }

    // Success
    result.data = decompressed;
    result.size = total_out;
    result.status = 0;
    return result;
}

void cleanup_decompression_result(DecompressionResult *result)
{
    if (result && result->data)
    {
        free(result->data);
        result->data = NULL;
        result->size = 0;
    }
}

void build_file_output_string(const FUECrashFile *crash_file, char *files_combine_buffer, size_t buffer_size)
{
    memset(files_combine_buffer, 0, buffer_size);
    
    for (int i = 0; i < crash_file->file_header->file_count; i++)
    {
        if (g_print_mode_file)
        {
            char file_buffer[2048];
            resolve_app_file_path(crash_file->file_header->directory_name, &crash_file->file[i], file_buffer, sizeof(file_buffer));
            
            if (i > 0)
            {
                size_t current_len = strlen(files_combine_buffer);
                if (current_len < buffer_size - 2) {
                    strncat(files_combine_buffer, " ", buffer_size - current_len - 1);
                }
            }
            
            if (strchr(file_buffer, ' ') != NULL || strchr(file_buffer, '\n') != NULL || strchr(file_buffer, '\t') != NULL)
            {
                char temp_buffer[2052];
                size_t path_len = strlen(file_buffer);
                if (path_len < sizeof(temp_buffer) - 3)
                {
                    snprintf(temp_buffer, sizeof(temp_buffer), "\"%s\"", file_buffer);
                    strncpy(file_buffer, temp_buffer, sizeof(file_buffer) - 1);
                    file_buffer[sizeof(file_buffer) - 1] = '\0';
                }
            }
            
            size_t current_len = strlen(files_combine_buffer);
            if (current_len < buffer_size - 1) {
                strncat(files_combine_buffer, file_buffer, buffer_size - current_len - 1);
            }
        }
    }
}

void process_crash_files(const DecompressionResult *decompression, const char *input_filename)
{
    log_verbose("Decompression successful. Decompressed size: %zu bytes\n", decompression->size);
    
    uint8_t *cursor = decompression->data;
    FUECrashFile *read_file = UECrashFile_Read(&cursor);
    
    if (!read_file) {
        log_error("Failed to parse crash file structure\n");
        return;
    }
    
    log_verbose("File header version: %d.%d.%d\n", 
                read_file->file_header->version[0], 
                read_file->file_header->version[1], 
                read_file->file_header->version[2]);
    log_verbose("Directory name: %s\n", read_file->file_header->directory_name);
    log_verbose("File name: %s\n", read_file->file_header->file_name);
    log_verbose("Uncompressed size: %d bytes\n", read_file->file_header->uncompressed_size);
    log_verbose("File count: %d\n", read_file->file_header->file_count);
    
    create_crash_directory(read_file->file_header->directory_name);
    log_verbose("Files in the crash report:\n");
    
    for (int i = 0; i < read_file->file_header->file_count; i++)
    {
        log_verbose("- File %d: %.*s, size: %d bytes\n", 
                    i + 1, 
                    read_file->file[i].file_name->length, 
                    read_file->file[i].file_name->content, 
                    read_file->file[i].file_size);
        write_file(read_file->file_header->directory_name, &read_file->file[i]);
    }
    
    output_results(read_file);
    
    UECrashFile_Destroy(read_file);
    log_verbose("All files written successfully.\n");
}

void output_results(const FUECrashFile *crash_file)
{
    if (g_print_mode_file)
    {
        char files_combine_buffer[1024 * 24];
        build_file_output_string(crash_file, files_combine_buffer, sizeof(files_combine_buffer));
        log_info("%s\n", files_combine_buffer);
    }
    else
    {
        char directory_path[2048];
        resolve_app_directory_path(crash_file->file_header->directory_name, directory_path, sizeof(directory_path));
        log_info("%s\n", directory_path);
    }
    
    fflush(stdout);
}
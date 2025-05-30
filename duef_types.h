#ifndef DUEF_TYPES_H
#define DUEF_TYPES_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct FAnsiCharStr
{
  int32_t length;
  char *content;
} FAnsiCharStr;

typedef struct FFileHeader
{
  uint8_t version[3];
  FAnsiCharStr *directory_name;
  FAnsiCharStr *file_name;
  int32_t uncompressed_size;
  int32_t file_count;
} FFileHeader;

typedef struct FFile
{
  int current_file_index;
  FAnsiCharStr *file_name;
  int32_t file_size;
  uint8_t *file_data; // Pointer to the file data in memory
} FFile;

typedef struct FUECrashFile
{
  FFileHeader *file_header;
  FFile *file;
} FUECrashFile;

int32_t read_int32(uint8_t **data)
{
  int32_t value = *(int32_t *)(*data);
  (*data) += sizeof(int32_t);
  return value;
}

char read_char(uint8_t **data)
{
  char value = (char)(**data);
  (*data)++;
  return value;
}

FAnsiCharStr *AnsiCharStr_Read(uint8_t **data)
{
  FAnsiCharStr *str = (FAnsiCharStr *)malloc(sizeof(FAnsiCharStr));
  if (!str)
  {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  str->length = read_int32(data);
  str->content = (char *)malloc(str->length + 1);
  if (!str->content)
  {
    fprintf(stderr, "Memory allocation for content failed\n");
    free(str);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < str->length; i++)
  {
    str->content[i] = read_char(data);
  }
  return str;
}

void AnsiCharStr_Destroy(FAnsiCharStr *str)
{
  if (str)
  {
    free(str->content);
    free(str);
  }
}

FFileHeader *FileHeader_Read(uint8_t **data)
{
  FFileHeader *header = (FFileHeader *)malloc(sizeof(FFileHeader));
  if (!header)
  {
    fprintf(stderr, "Memory allocation for FFileHeader failed\n");
    exit(EXIT_FAILURE);
  }
  header->version[0] = read_char(data);
  header->version[1] = read_char(data);
  header->version[2] = read_char(data);

  header->directory_name = AnsiCharStr_Read(data);
  header->file_name = AnsiCharStr_Read(data);

  header->uncompressed_size = *(int32_t *)(*data);
  (*data) += sizeof(int32_t);

  header->file_count = *(int32_t *)(*data);
  (*data) += sizeof(int32_t);

  return header;
}

void FileHeader_Destroy(FFileHeader *header)
{
  if (header)
  {
    AnsiCharStr_Destroy(header->directory_name);
    AnsiCharStr_Destroy(header->file_name);
    free(header);
  }
}

FFile *File_Read(uint8_t **data)
{
  FFile *file = (FFile *)malloc(sizeof(FFile));
  if (!file)
  {
    fprintf(stderr, "Memory allocation for FFile failed\n");
    exit(EXIT_FAILURE);
  }
  file->current_file_index = read_int32(data);
  file->file_name = AnsiCharStr_Read(data);
  file->file_size = read_int32(data);
  file->file_data = (uint8_t *)malloc(file->file_size);
  for (int i = 0; i < file->file_size; i++)
  {
    file->file_data[i] = read_char(data);
  }

  return file;
}

void File_Destroy(FFile *file)
{
  if (file)
  {
    AnsiCharStr_Destroy(file->file_name);
    free(file->file_data);
    free(file);
  }
}

struct FUECrashFile *UECrashFile_Read(uint8_t **data)
{
  FUECrashFile *ue_crash_file = (FUECrashFile *)malloc(sizeof(FUECrashFile));
  if (!ue_crash_file)
  {
    fprintf(stderr, "Memory allocation for FUECrashFile failed\n");
    exit(EXIT_FAILURE);
  }
  ue_crash_file->file_header = FileHeader_Read(data);
  ue_crash_file->file = calloc(ue_crash_file->file_header->file_count, sizeof(FFile));
  for (int i = 0; i < ue_crash_file->file_header->file_count; i++)
  {
    ue_crash_file->file[i] = *File_Read(data);
  }
  return ue_crash_file;
}

void UECrashFile_Destroy(FUECrashFile *ue_crash_file)
{
  if (ue_crash_file)
  {
    FileHeader_Destroy(ue_crash_file->file_header);
    for (int i = 0; i < ue_crash_file->file_header->file_count; i++)
    {
      File_Destroy(&ue_crash_file->file[i]);
    }
    free(ue_crash_file);
  }
}

#endif
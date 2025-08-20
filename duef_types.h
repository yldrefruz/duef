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

// Function declarations
int32_t read_int32(uint8_t **data);
char read_char(uint8_t **data);
FAnsiCharStr *AnsiCharStr_Read(uint8_t **data);
void AnsiCharStr_Destroy(FAnsiCharStr *string);
FFileHeader *FileHeader_Read(uint8_t **data);
void FileHeader_Destroy(FFileHeader *header);
FFile *File_Read(uint8_t **data);
void File_Destroy(FFile *file);
void File_DestroyContents(FFile *file);
FUECrashFile *UECrashFile_Read(uint8_t **data);
void UECrashFile_Destroy(FUECrashFile *ue_crash_file);

#endif
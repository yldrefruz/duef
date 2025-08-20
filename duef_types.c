#include "duef_types.h"

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
  FAnsiCharStr *string = malloc(sizeof(FAnsiCharStr));
  if (!string)
  {
    return NULL;
  }

  string->length = read_int32(data);
  string->content = malloc(string->length + 1);
  if (!string->content)
  {
    free(string);
    return NULL;
  }

  for (int i = 0; i < string->length; i++)
  {
    string->content[i] = read_char(data);
  }
  string->content[string->length] = '\0';

  return string;
}

void AnsiCharStr_Destroy(FAnsiCharStr *string)
{
  if (string)
  {
    if (string->content)
    {
      free(string->content);
    }
    free(string);
  }
}

FFileHeader *FileHeader_Read(uint8_t **data)
{
  FFileHeader *header = malloc(sizeof(FFileHeader));
  if (!header)
  {
    return NULL;
  }

  for (int i = 0; i < 3; i++)
  {
    header->version[i] = read_char(data);
  }

  header->directory_name = AnsiCharStr_Read(data);
  header->file_name = AnsiCharStr_Read(data);
  header->uncompressed_size = read_int32(data);
  header->file_count = read_int32(data);

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
  FFile *file = malloc(sizeof(FFile));
  if (!file)
  {
    return NULL;
  }

  file->current_file_index = read_int32(data);
  file->file_name = AnsiCharStr_Read(data);
  file->file_size = read_int32(data);

  file->file_data = malloc(file->file_size);
  if (!file->file_data)
  {
    AnsiCharStr_Destroy(file->file_name);
    free(file);
    return NULL;
  }

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
    File_DestroyContents(file);
    free(file);
  }
}

void File_DestroyContents(FFile *file)
{
  if (file)
  {
    AnsiCharStr_Destroy(file->file_name);
    if (file->file_data)
    {
      free(file->file_data);
      file->file_data = NULL;
    }
  }
}

FUECrashFile *UECrashFile_Read(uint8_t **data)
{
  FUECrashFile *crash_file = malloc(sizeof(FUECrashFile));
  if (!crash_file)
  {
    return NULL;
  }

  crash_file->file_header = FileHeader_Read(data);
  if (!crash_file->file_header)
  {
    free(crash_file);
    return NULL;
  }

  crash_file->file = malloc(sizeof(FFile) * crash_file->file_header->file_count);
  if (!crash_file->file)
  {
    FileHeader_Destroy(crash_file->file_header);
    free(crash_file);
    return NULL;
  }

  for (int i = 0; i < crash_file->file_header->file_count; i++)
  {
    FFile *file = File_Read(data);
    if (file)
    {
      crash_file->file[i] = *file;
      free(file);
    }
  }

  return crash_file;
}

void UECrashFile_Destroy(FUECrashFile *ue_crash_file)
{
  if (ue_crash_file)
  {
    int file_count = ue_crash_file->file_header->file_count;
    FileHeader_Destroy(ue_crash_file->file_header);
    for (int i = 0; i < file_count; i++)
    {
      File_DestroyContents(&ue_crash_file->file[i]);
    }
    free(ue_crash_file->file);
    free(ue_crash_file);
  }
}
#pragma once

#include "km_types.h"

// TODO get rid of this
#define PATH_MAX_LENGTH 512

struct ReadFileResult
{
    uint64 size;
    void* data;
};

char* GetAppPath();
void CatStrings(const char* str1, const char* str2, char* dest, uint64 destLen);
// Reads entire contents of file into memory.
// Returns a ReadFileResult containing the file data and size.
// free(..) must be called on the file data when done.
ReadFileResult ReadFile(const char* path);
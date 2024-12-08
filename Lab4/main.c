#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <aclapi.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_TIME_QUANTUM 4
#define MAX_BURST_TIME 10
#define MAX_PRIORITY 5
#define AGING_FACTOR 1


void get_file_attributes(const char* filename) {
    DWORD attributes = GetFileAttributes(filename);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        printf("Error getting file attributes: %lu\n", GetLastError());
        return;
    }

    printf("File Attributes for %s:\n", filename);

    if (attributes & FILE_ATTRIBUTE_HIDDEN) printf("- Hidden\n");
    if (attributes & FILE_ATTRIBUTE_READONLY) printf("- Read-only\n");
    if (attributes & FILE_ATTRIBUTE_SYSTEM) printf("- System\n");
    if (attributes & FILE_ATTRIBUTE_ARCHIVE) printf("- Archive\n");

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file: %lu\n", GetLastError());
        return;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        printf("Error getting file size: %lu\n", GetLastError());
        CloseHandle(hFile);
        return;
    }

    FILETIME creationTime, lastAccessTime, lastWriteTime;
    if (!GetFileTime(hFile, &creationTime, &lastAccessTime, &lastWriteTime)) {
        printf("Error getting file times: %lu\n", GetLastError());
        CloseHandle(hFile);
        return;
    }

    SYSTEMTIME stUTC;
    printf("File size: %lld bytes\n", fileSize.QuadPart);

    FileTimeToSystemTime(&creationTime, &stUTC);
    printf("Creation Time: %02d/%02d/%04d %02d:%02d:%02d\n", stUTC.wDay, stUTC.wMonth, stUTC.wYear, stUTC.wHour, stUTC.wMinute, stUTC.wSecond);

    CloseHandle(hFile);
}


void buffered_read(const char* filename) {
    clock_t start = clock();
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file (buffered)");
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    fread(buffer, 1, size, file);
    fclose(file);
    free(buffer);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Buffered read time: %f seconds\n", time_spent);
}

void unbuffered_read(const char* filename) {
    clock_t start = clock();
    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file (unbuffered): %lu\n", GetLastError());
        return;
    }

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    char* buffer = (char*)malloc(fileSize.QuadPart);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        CloseHandle(hFile);
        return;
    }

    DWORD bytesRead;
    ReadFile(hFile, buffer, fileSize.QuadPart, &bytesRead, NULL);
    CloseHandle(hFile);
    free(buffer);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Unbuffered read time: %f seconds\n", time_spent);
}


void async_io(const char* filename) {
    printf("Async IO (Simplified Example):\n");

    HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Error opening file (async): %lu\n", GetLastError());
        return;
    }

    OVERLAPPED overlapped = {0};
    DWORD bytesRead;
    char buffer[1024];

    if (ReadFileEx(hFile, buffer, sizeof(buffer), &overlapped, NULL)) {
        printf("Async read operation started.\n");
    } else {
        printf("Error initiating async read: %lu\n", GetLastError());
        CloseHandle(hFile);
        return;
    }

    if (GetOverlappedResult(hFile, &overlapped, &bytesRead, TRUE)) {
        printf("Async read complete. Bytes read: %lu\n", bytesRead);
    } else {
        printf("Error in async read: %lu\n", GetLastError());
    }

    CloseHandle(hFile);
}


int main() {
    char filename[] = "Lab4.txt";
    char large_filename[] = "Lab4_big.txt";


    get_file_attributes(filename);
    buffered_read(filename);
    unbuffered_read(filename);
    async_io(filename);


    printf("\nOperations on your large file (%s):\n", large_filename);

    get_file_attributes(large_filename);
    buffered_read(large_filename);
    unbuffered_read(large_filename);



    return 0;
}
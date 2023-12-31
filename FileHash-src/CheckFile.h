/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define AuthorMagicNumberValue 0x524554534F465653
#define HeaderMagicNumberValue 0x48534148
#define FileExtensionDefault TEXT(".checkfile")

#if defined(ENV64BIT)
    #define CurrentPlatformValue IMAGE_FILE_MACHINE_AMD64
#elif defined (ENV32BIT)
    #define CurrentPlatformValue IMAGE_FILE_MACHINE_I386
#else
    #error "Must define either ENV32BIT or ENV64BIT"
#endif

#define CheckFileOperationCreate 1
#define CheckFileOperationRead 2

#define BufferReadSizeDefault 65536


typedef struct _HashedFileInfoData
{
    DWORD RecordSize;
    DWORD FileSizeHigh;
    DWORD FileSizeLow;
    BYTE  data[0];
} THashedFileInfoData, *PHashedFileInfoData;

typedef struct _CheckFileInfoHeader
{
    DWORD HeaderMagicNumber;
    DWORD HeaderSize;
    WORD  Platform;
    TAlgorithms  Algorithms;
    DWORD FilesTotalCount;
    DWORD FileSizeHigh;
    DWORD FileSizeLow;
} TCheckFileInfoHeader, *PCheckFileInfoHeader;

typedef struct _CheckFileHeader
{
    DWORDLONG AuthorMagicNumber;
    TCheckFileInfoHeader InfoHeader;
} TCheckFileHeader, *PCheckFileHeader;

typedef struct _CheckFileObj
{
    HANDLE hFile;
    LPTSTR FileName;
    LPTSTR FileNameFull;
    BOOL CanDelete;
    DWORD Operation;
    PBYTE ReadBuffer;
    DWORD ReadBufferSize;
    DWORD HashBlockSize;
    TCheckFileHeader Header;
} TCheckFileObj, *PCheckFileObj;


BOOL CheckFileCreate(LPCTSTR FileName, CONST BOOL Overwrite, TCAlgorithms Algorithms, PCheckFileObj FObj);
BOOL CheckFileOpen(LPCTSTR FileName, PCheckFileObj FObj);
BOOL CheckFileClose(PCheckFileObj FObj);
VOID CheckFileFreeBuffers(PCheckFileObj FObj);
BOOL CheckFileWrite
(
    PCheckFileObj FObj,
    LPCTSTR FileName,
    CONST DWORD FileSizeHigh,
    CONST DWORD FileSizeLow,
    PBYTE HashBlock,
    CONST DWORD HashBlockSize
);
BOOL CheckFileRead
(
    PCheckFileObj FObj,
    LPVOID FileName,
    PDWORD FileSizeHigh,
    PDWORD FileSizeLow,
    LPVOID HashBlock,
    PBOOL EOFile
);
BOOL ReadFileEOF(HANDLE HandleFile, LPVOID Buffer, DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, PBOOL EOFile);

/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    See CheckFile.c for details

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

#define CheckFileOperationUndefined 0
#define CheckFileOperationCreate 1
#define CheckFileOperationRead 2

#define BufferReadSizeDefault 2048


typedef struct _HashedFileInfoData
{
    DWORD RecordSize;
    DWORD FileSizeHigh;
    DWORD FileSizeLow;
    BYTE  data[0];
} THashedFileInfoData, *PHashedFileInfoData;
typedef CONST THashedFileInfoData TCHashedFileInfoData, *PCHashedFileInfoData;

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
typedef CONST TCheckFileInfoHeader TCCheckFileInfoHeader, *PCCheckFileInfoHeader;

typedef struct _CheckFileHeader
{
    DWORDLONG AuthorMagicNumber;
    TCheckFileInfoHeader InfoHeader;
} TCheckFileHeader, *PCheckFileHeader;
typedef CONST TCheckFileHeader TCCheckFileHeader, *PCCheckFileHeader;

typedef struct _CheckFileObj
{
    BOOL   IsOpen;
    HANDLE hFile;
    LPTSTR FileName;
    LPTSTR FileNameFull;
    BOOL  CanDelete;
    DWORD Operation;
    PBYTE ReadBuffer;
    DWORD ReadBufferSize;
    DWORD HashBlockSize;
    DWORD RecordFixedPartSize;
    TCheckFileHeader Header;
} TCheckFileObj, *PCheckFileObj;
typedef CONST TCheckFileObj TCCheckFileObj, *PCCheckFileObj;


BOOL CheckFileCreate(LPCTSTR FileName, CONST BOOL Overwrite, TCAlgorithms Algorithms, PCheckFileObj FObj);
BOOL CheckFileOpen(LPCTSTR FileName, PCheckFileObj FObj);
BOOL CheckFileClose(PCheckFileObj FObj);
BOOL CheckFileWrite
(
    PCheckFileObj FObj,
    LPCWSTR HashedFileName,
    CONST DWORD HashedFileSizeHigh,
    CONST DWORD HashedFileSizeLow,
    CONST PBYTE HashBlock
);
BOOL CheckFileRead
(
    PCheckFileObj FObj,
    LPWSTR* HashedFileName,
    PDWORD  HashedFileSizeHigh,
    PDWORD  HashedFileSizeLow,
    PBYTE*  HashBlock,
    PBOOL   EOFile
);

static VOID CheckFileFreeBuffers(PCheckFileObj FObj);
static BOOL CheckFileCloseForced(PCheckFileObj FObj);

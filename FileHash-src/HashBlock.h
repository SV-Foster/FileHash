/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    See HashBlock.c for details

***/

#pragma once


#define HashAlgoMD2    0x0001
#define HashAlgoMD4    0x0002
#define HashAlgoMD5    0x0004
#define HashAlgoSHA1   0x0008
#define HashAlgoSHA256 0x0010
#define HashAlgoSHA384 0x0020
#define HashAlgoSHA512 0x0040

#define HashAlgoAll \
	HashAlgoMD2 | HashAlgoMD4 | HashAlgoMD5 | HashAlgoSHA1 | HashAlgoSHA256 | HashAlgoSHA384 | HashAlgoSHA512

#define HashAlgoCount 7

typedef WORD TAlgorithms;
typedef CONST TAlgorithms TCAlgorithms;
typedef TAlgorithms* PAlgorithms;
typedef CONST TAlgorithms* PCAlgorithms;

typedef struct _HashAlgorithmsData
{
	TCAlgorithms Algorithm;
	ALG_ID Win32APICode;
	DWORD DigestSize;
	TCHAR Name[7];
} THashAlgorithmsData, *PHashAlgorithmsData;
typedef CONST THashAlgorithmsData TCHashAlgorithmsData, *PCHashAlgorithmsData;


DWORD HashBlockSizeGet(TCAlgorithms Algorithms);
BOOL  HashBlockAlgoDataGet(TCAlgorithms Algorithms, PCHashAlgorithmsData* Buffer, DWORD BufferSizeBytes);

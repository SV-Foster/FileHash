/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

    Rev 1, FEB 2024
       Added IncludeHidden option

***/

#pragma once


// file read for hashing buffer size
#define BufferIOSizeBytes 524288
#define ExtendedPathMaxChar 32767
#define ExtendedPathMaxCharTerm 32768
#define PathListArrayElements 1024
#define AlgorithmsDefault HashAlgoSHA256


typedef enum _OperatingMode
{
	OperatingModeHelp,
	OperatingModeCreate,
	OperatingModeVerify,
	OperatingModeList
} TOperatingMode;

typedef struct _GlobalOptions
{
	TOperatingMode OperatingMode;
	TAlgorithms Algorithms;
	LPTSTR CheckFileName;
	LPTSTR PathListArray[PathListArrayElements];
	BOOL IsRecursive;
	BOOL CheckFileOverwrite;
	BOOL NoCopyrightLogo;
	BOOL IncludeHidden;
} TGlobalOptions, *PGlobalOptions;

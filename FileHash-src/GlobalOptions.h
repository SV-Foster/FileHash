/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


// file read for hashing buffer size
#define BufferIOSizeBytes 524288
#define ExtendedPathMaxChar 32767
#define PathListArrayElements 1024
#define AlgorithmsDefault HashAlgoSHA256;


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
} TGlobalOptions, *PGlobalOptions;

/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
	See CommandLineInterface.c for details

***/

#pragma once


#define UserAbortionCheckInterval 333

// Files were hashed without error
#define ExitCodeOK EXIT_SUCCESS
// No files were found to hash
#define ExitCodeErrorNoFiles 1
// The user pressed CTRL+C to terminate the program
#define ExitCodeErrorUserStop 2
// File(s) with wrong hash found during the ceck
#define ExitCodeWrongHash 3
// Initialization error occurred. There is not enough memory or disk space, or you entered 
// an invalid drive name or invalid syntax on the command line
#define ExitCodeErrorInit 4
//	Disk read/write error occurred
#define ExitCodeErrorIO 5

#if defined(ENV64BIT)
	#define ArchString TEXT("x64 (x86-64)")
#elif defined (ENV32BIT)
	#define ArchString TEXT("x86 (IA-32)")
#else
	#error "Must define either ENV32BIT or ENV64BIT"
#endif


BOOL  CLIWorkModeGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo);
BOOL  CLISwitchesGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo);
BOOL  CLIPathsGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo);
VOID  CLILogoPrint();
VOID  CLIHelpPrint();
VOID  CLIFilePrintBasicData(LPCTSTR FileName, CONST DWORD FileSizeHigh, CONST DWORD FileSizeLow);
VOID  CLIFilePrintHashData(PBYTE HashBlock, TCAlgorithms Algorithms);
VOID  CLIFilePrintCreateTotal(CONST UINT MsgID, CONST DWORD FilesTotalCount, CONST DWORD SizeHigh, CONST DWORD SizeLow, LPCTSTR FileName);
VOID  CLIFilePrintCheckTotal
(
	CONST DWORD FilesChecked,
	CONST DWORD FilesTotalCount,
	CONST DWORD SizeHigh,
	CONST DWORD SizeLow,
	CONST DWORD FilesWErrors,
	LPCTSTR FileName
);
VOID  CLIProgramStartOperationPrint(CONST TOperatingMode Mode);
VOID  CLIProgramExitSummaryPrint(int Result);
VOID  CLIWriteLN();
VOID  CLIHashPrintByByte(PBYTE HashBlock, CONST DWORD Length);
BOOL  CLISetModeUTF16();
BOOL  WINAPI CLIConsoleHandler(DWORD signal);
BOOL  CLIIsAbortedByUser();
BOOL  CLIIsAbortedByUserDirect();
VOID  CLIConsoleEventsInit();

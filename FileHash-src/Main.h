/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    See Main.c for details

***/

#pragma once


DWORD ModeHelp();
DWORD ModeCreate(PGlobalOptions GlobalOptions);
DWORD ModeVerify(PGlobalOptions GlobalOptions);
DWORD ModeList(PGlobalOptions GlobalOptions);
BOOL  ProcessPathString(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, CONST BOOL IsRecursive, CONST BOOL SkipHidden);
BOOL  ProcessFile(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, LPBY_HANDLE_FILE_INFORMATION ffd, CONST SIZE_T PathCut);
BOOL  ProcessFolder(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, CONST BOOL IsRecursive, CONST BOOL SkipHidden, CONST SIZE_T PathCut);
BOOL  FileHashAlgoBunch(LPCTSTR PathString, PBYTE HashBuffer, TCAlgorithms Algorithms);
BOOL  FileHashAlgoSingle(LPCTSTR PathString, PBYTE HashBuffer, DWORD BufferLength, ALG_ID AlgAPICode);
BOOL  IsFileSkip(LPCTSTR PathString, LPCTSTR PathCheckFile, PBOOL Skip);

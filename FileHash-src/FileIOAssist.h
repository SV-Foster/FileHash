/***

Copyright SV Foster, 2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage

Revision History:
    See FileIOAssist.c for details

***/

#pragma once



BOOL ReadFileEOF(HANDLE HandleFile, LPVOID Buffer, DWORD NumberOfBytesToRead, PBOOL EOFile);
BOOL FileGetAttributes(LPCTSTR PathString, LPBY_HANDLE_FILE_INFORMATION FileInformation);

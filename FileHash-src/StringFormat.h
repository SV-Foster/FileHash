/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
	See StringFormat.c for details

***/

#pragma once

// Max length of ULONGLONG in decimal as string is 20 + 1 NULL char - 18446744073709551615\0
#define ULONGLONGMAXLENGTH 21
#define MAX_LCL_STRING  256


BOOL  StringFmtU64WithGroupingHLGet(LPTSTR Str, CONST DWORD StrLength, CONST DWORD SizeHigh, CONST DWORD SizeLow, CONST LCID lcid);
UINT  StringFmtGroupingStrToUint(LPCTSTR Grouping);
BOOL  StringFmtDefaultNumberFmtGet(CONST LCID lcid, LPNUMBERFMT NumberFmt, int DecimalSep, int ThousandSep);

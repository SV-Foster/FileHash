/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
	See PathWorks.c for details

***/

#pragma once


VOID  PathStringSlashLegacyNormalyze(LPTSTR PathString);
VOID  PathStringSlashDoubleRemove(LPTSTR PathString);
VOID  FindFirstFilePathNormalyze(LPTSTR PathString, CONST DWORD LengthMax);
VOID  PathStringSlashTrailingIclude(LPTSTR PathString, CONST DWORD LengthMax);
BOOL  PathStringSlashFindLast(LPCTSTR PathString, PSIZE_T Position);
VOID  PathStringFileNameRemove(LPTSTR PathString);
BOOL  PathStringHasWildCards(LPCTSTR PathString);

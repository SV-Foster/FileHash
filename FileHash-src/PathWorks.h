/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


VOID  PathStringSlashLegacyNormalyze(LPTSTR PathString);
VOID  PathStringSlashDoubleRemove(LPTSTR PathString);
VOID  FindFirstFilePathNormalyze(LPTSTR PathString, CONST DWORD LengthMax);
VOID  PathStringSlashTrailingIclude(LPTSTR PathString, CONST DWORD LengthMax);
BOOL  PathStringSlashFindLast(LPCTSTR PathString, PSIZE_T Position);
VOID  PathStringFileNameRemove(LPTSTR PathString);
BOOL  PathStringHasWildCards(LPCTSTR PathString);

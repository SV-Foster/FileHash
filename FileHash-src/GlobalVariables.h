/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
	See GlobalVariables.c for details

***/

#pragma once


typedef struct _TGlobalVariables
{
	// for ctrl-c support
	CRITICAL_SECTION BreakCrSec;
	BOOL ProcessBreak;
} TGlobalVariables, *PGlobalVariables;

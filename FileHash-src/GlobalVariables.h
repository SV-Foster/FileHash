/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


typedef struct _TGlobalVariables
{
	// for ctrl-c support
	CRITICAL_SECTION BreakCrSec;
	BOOL ProcessBreak;
} TGlobalVariables, *PGlobalVariables;

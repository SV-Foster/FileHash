/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

    Rev 1, FEB 2024
       Code refactoring

***/

#include <Windows.h>
#include "SharedHeaders.h"
#include "HashBlock.h"
#include "HashBlockAlgDB.h"


// get size in bytes of the hashblock by algorithms requested
DWORD HashBlockSizeGet(TCAlgorithms Algorithms)
{
	DWORD Result = 0;


	for (SIZE_T i = 0; i < ARRAYSIZE(HashAlgDB); ++i)
		if (Algorithms & HashAlgDB[i].Algorithm)
			Result += HashAlgDB[i].DigestSize;

	return Result;
}

// fill a buffer w/null-terminated array of pointers to the actual data about algorithms requested
BOOL HashBlockAlgoDataGet(TCAlgorithms Algorithms, PCHashAlgorithmsData* Buffer, DWORD BufferSizeBytes)
{
	SIZE_T p = 0;


	// check we can write the null-terminator at least
	if (BufferSizeBytes < sizeof(PCHashAlgorithmsData))
		return FALSE;

	for (SIZE_T i = 0; i < ARRAYSIZE(HashAlgDB); ++i)
	{
		if (Algorithms & HashAlgDB[i].Algorithm)
		{
			if (BufferSizeBytes < (sizeof(PCHashAlgorithmsData)*(p+2)))
				return FALSE;

			// save the pointer to the memory where actual data is located
			Buffer[p] = &HashAlgDB[i];
			++p;
		}
	}
	// write the null-terminator to the end of the array
	Buffer[p] = NULL;

	return TRUE;
}

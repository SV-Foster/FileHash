/***

Copyright SV Foster, 2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage

Revision History:
    Rev 0, DEC 2023
       First revision

***/

#include <Windows.h>
#include "SharedHeaders.h"
#include "FileIOAssist.h"


// read data from the file
// returns TRUE if requested amount of bytes is read, FALSE in case of EOF, I/O error or data left in the file is too short
// EOFile is set to TRUE, if end of file is reached w/o read errors
BOOL ReadFileEOF(HANDLE HandleFile, LPVOID Buffer, DWORD NumberOfBytesToRead, PBOOL EOFile)
{
	DWORD NumberOfBytesRead;
	if (EOFile)
		*EOFile = FALSE;


	if (!ReadFile(HandleFile, Buffer, NumberOfBytesToRead, &NumberOfBytesRead, NULL))
		return FALSE; // I/O error

	// check for EOF
	// If lpOverlapped is NULL, then when a synchronous read operation reaches the end of a file, ReadFile returns TRUE 
	// and sets *lpNumberOfBytesRead to zero
	if ((NumberOfBytesToRead != 0) && (NumberOfBytesRead == 0))
	{
		if (EOFile)
			*EOFile = TRUE;

		return FALSE; // EOF
	}

	// check all data was read
	if (NumberOfBytesToRead != NumberOfBytesRead)
		return FALSE; // data left in the file is too short


	return TRUE;
}

// get attributes of a file or a folder
BOOL FileGetAttributes(LPCTSTR PathString, LPBY_HANDLE_FILE_INFORMATION FileInformation)
{
	DWORD Result = TRUE;
	HANDLE h;


	// In rare cases or on a heavily loaded system, file attribute information on NTFS file systems 
	// may not be current at the time FindFirstFile is called, so using GetFileInformationByHandle instead
	h = CreateFile(PathString, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (h == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);

	if (!GetFileInformationByHandle(h, FileInformation))
		ExitFunction(FALSE);


function_end:
	if (h != INVALID_HANDLE_VALUE)
		CloseHandle(h);

	return Result;
}

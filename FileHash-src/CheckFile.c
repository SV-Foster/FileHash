/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

    Rev 1, FEB 2024
       Read error checks improved
       Potential problems w/non-unicode file names and paths inside the .checkfile fixed
       Improved .checkfile create/open/close code
       Code refactoring

***/

#include <Windows.h>
#include <tchar.h>
#include <assert.h>
#include "SharedHeaders.h"
#include "HashBlock.h"
#include "GlobalOptions.h"
#include "FileIOAssist.h"
#include "CheckFile.h"


// create new .checkfile
BOOL CheckFileCreate(LPCTSTR FileName, CONST BOOL Overwrite, TCAlgorithms Algorithms, PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	DWORD CreationDisposition = CREATE_NEW;
	if (Overwrite)
		CreationDisposition = CREATE_ALWAYS;
	

	// init the checkfile object
	ZeroMemory(FObj, sizeof(TCheckFileObj));
	FObj->Operation = CheckFileOperationCreate;
	FObj->hFile = INVALID_HANDLE_VALUE;
	// init header of the checkfile
	FObj->Header.AuthorMagicNumber = AuthorMagicNumberValue;
	FObj->Header.InfoHeader.HeaderMagicNumber = HeaderMagicNumberValue;
	FObj->Header.InfoHeader.HeaderSize = sizeof(TCheckFileInfoHeader);
	FObj->Header.InfoHeader.Platform = CurrentPlatformValue;
	FObj->Header.InfoHeader.Algorithms = Algorithms;
	// copy provided file path
	SIZE_T Len = _tcslen(FileName);
	FObj->FileName = malloc((Len + 1) * sizeof(TCHAR));
	if (!FObj->FileName)
		ExitFunction(FALSE);
	_tcsncpy_s(FObj->FileName, Len + 1, FileName, Len);
	// get full path of this file path from Windows
	FObj->FileNameFull = malloc(ExtendedPathMaxCharTerm * sizeof(TCHAR));
	if (!FObj->FileNameFull)
		ExitFunction(FALSE);
	if (!GetFullPathName(FileName, ExtendedPathMaxCharTerm, FObj->FileNameFull, NULL))
		ExitFunction(FALSE);

	// calculate some metainformation for convenience
	FObj->HashBlockSize = HashBlockSizeGet(Algorithms);
	FObj->RecordFixedPartSize = sizeof(THashedFileInfoData) + FObj->HashBlockSize;	

	// open the file and write a header
	FObj->hFile = CreateFile
	(
		FileName,
		GENERIC_WRITE | DELETE,
		FILE_SHARE_READ,
		NULL,
		CreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (FObj->hFile == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);
	
	FObj->CanDelete = TRUE;

	// set the file pointer ahead, later here will be the header
	// If the function returns a value other than INVALID_SET_FILE_POINTER, the call to SetFilePointer has succeeded
	if (SetFilePointer(FObj->hFile, sizeof(FObj->Header), NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		ExitFunction(FALSE);

	FObj->IsOpen = TRUE;


function_end:
	if (!Result)
		CheckFileCloseForced(FObj);

	return Result;
}

// open an existing checkfile
BOOL CheckFileOpen(LPCTSTR FileName, PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	TAlgorithms BufferAlgoUnknown = HashAlgoAll;
	BufferAlgoUnknown = ~BufferAlgoUnknown;


	// init the checkfile object
	ZeroMemory(FObj, sizeof(TCheckFileObj));
	FObj->Operation = CheckFileOperationRead;

	// open the file and read the header
	FObj->hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FObj->hFile == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);

	if (!ReadFileEOF(FObj->hFile, &FObj->Header, sizeof(FObj->Header), NULL))
		ExitFunction(FALSE);

	// check the header
	if (FObj->Header.AuthorMagicNumber != AuthorMagicNumberValue)
		ExitFunction(FALSE);

	if (FObj->Header.InfoHeader.HeaderMagicNumber != HeaderMagicNumberValue)
		ExitFunction(FALSE);

	if (FObj->Header.InfoHeader.HeaderSize != sizeof(TCheckFileInfoHeader))
		ExitFunction(FALSE);

	if ((FObj->Header.InfoHeader.Platform != IMAGE_FILE_MACHINE_I386) && (FObj->Header.InfoHeader.Platform != IMAGE_FILE_MACHINE_AMD64))
		ExitFunction(FALSE);
		
	if ((FObj->Header.InfoHeader.Algorithms & BufferAlgoUnknown) > 0)
		ExitFunction(FALSE);

	// init read buffer
	FObj->ReadBuffer = malloc(BufferReadSizeDefault);
	FObj->ReadBufferSize = BufferReadSizeDefault;

	if (FObj->ReadBuffer == NULL)
		ExitFunction(FALSE);

	// calculate some metainformation for convenience
	FObj->HashBlockSize = HashBlockSizeGet(FObj->Header.InfoHeader.Algorithms);
	FObj->RecordFixedPartSize = sizeof(THashedFileInfoData) + FObj->HashBlockSize;
	assert(FObj->ReadBufferSize > FObj->RecordFixedPartSize); // check BufferReadSizeDefault constant

	FObj->IsOpen = TRUE;


function_end:
	if (!Result)
		CheckFileCloseForced(FObj);

	return Result;
}

// close the checkfile
BOOL CheckFileClose(PCheckFileObj FObj)
{
	if (!FObj->IsOpen)
		return FALSE;

	return CheckFileCloseForced(FObj);
}

BOOL CheckFileCloseForced(PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	DWORD NumberOfBytesWritten; // this WriteFile parameter can be NULL only when the lpOverlapped parameter is not NULL


	// save final version of the header
	if (FObj->Operation == CheckFileOperationCreate)
	{
		if (SetFilePointer(FObj->hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			ExitFunction(FALSE);

		if (!WriteFile(FObj->hFile, &FObj->Header, sizeof(FObj->Header), &NumberOfBytesWritten, NULL))
			ExitFunction(FALSE);
	}


function_end:
	// If the application is running under a debugger, the function will throw an exception if it receives
	// either a handle value that is not valid or a pseudo-handle value
	CloseHandle(FObj->hFile);

	// delete .checkfile, if empty
	if (!FObj->Header.InfoHeader.FilesTotalCount)
		if (FObj->CanDelete)
			DeleteFile(FObj->FileName);

	CheckFileFreeBuffers(FObj);
	FObj->IsOpen = FALSE;

	return Result;
}

// free all allocated memory on the heap
VOID CheckFileFreeBuffers(PCheckFileObj FObj)
{
	free(FObj->ReadBuffer);
	FObj->ReadBuffer = NULL;

	free(FObj->FileName);
	FObj->FileName = NULL;

	free(FObj->FileNameFull);
	FObj->FileNameFull = NULL;
}

// adds new record to the checkfile
// returns TRUE if data is saved, FALSE otherwise
BOOL CheckFileWrite
(
	PCheckFileObj FObj,
	LPCWSTR HashedFileName,
	CONST DWORD HashedFileSizeHigh,
	CONST DWORD HashedFileSizeLow,
	CONST PBYTE PHashBlock
)
{
	DWORD NumberOfBytesWritten; // this WriteFile parameter can be NULL only when the lpOverlapped parameter is not NULL
	THashedFileInfoData hfid;
	CONST SIZE_T StringLength = wcslen(HashedFileName) + 1;
	CONST DWORD StringBytes = (DWORD)(StringLength * sizeof(WCHAR));
	ULARGE_INTEGER BufferSA, BufferSB;


	// simple data validity check
	if (StringLength > ExtendedPathMaxCharTerm)
		return FALSE;

	// increase the "files count" in the header
	FObj->Header.InfoHeader.FilesTotalCount += 1;
	// increase the "total files size" in the header
	BufferSA.HighPart = HashedFileSizeHigh;
	BufferSA.LowPart = HashedFileSizeLow;
	BufferSB.HighPart = FObj->Header.InfoHeader.FileSizeHigh;
	BufferSB.LowPart = FObj->Header.InfoHeader.FileSizeLow;
	BufferSA.QuadPart += BufferSB.QuadPart;
	FObj->Header.InfoHeader.FileSizeHigh = BufferSA.HighPart;
	FObj->Header.InfoHeader.FileSizeLow = BufferSA.LowPart;

	// prepare the record header, fill data and
	// calculate the record size in the file
	hfid.RecordSize = FObj->RecordFixedPartSize + StringBytes;
	hfid.FileSizeHigh = HashedFileSizeHigh;
	hfid.FileSizeLow = HashedFileSizeLow;

	// save the header
	if (!WriteFile(FObj->hFile, &hfid, sizeof(hfid), &NumberOfBytesWritten, NULL))
		return FALSE;
	// save data
	if (!WriteFile(FObj->hFile, PHashBlock, FObj->HashBlockSize, &NumberOfBytesWritten, NULL))
		return FALSE;
	if (!WriteFile(FObj->hFile, HashedFileName, StringBytes, &NumberOfBytesWritten, NULL))
		return FALSE;


	return TRUE;
}

// read next file record from the checkfile
// returns TRUE if there is more data to read, FALSE if end of file is reached
// EOFile is set to TRUE, if end of file is reached w/o read errors
BOOL CheckFileRead
(
	PCheckFileObj FObj,
	LPWSTR* HashedFileName,
	PDWORD  HashedFileSizeHigh,
	PDWORD  HashedFileSizeLow,
	PBYTE*  HashBlock,
	PBOOL   EOFile
)
{
	*EOFile = FALSE;
	PWCHAR LastChar;
	PCHashedFileInfoData Phfid = (PCHashedFileInfoData)FObj->ReadBuffer;


	// read fixed size data, header + hashes	
	if (!ReadFileEOF(FObj->hFile, FObj->ReadBuffer, FObj->RecordFixedPartSize, EOFile))
		return FALSE;

	// basic check that the total size of the file record is correct
	if (Phfid->RecordSize > (FObj->RecordFixedPartSize + (ExtendedPathMaxCharTerm * sizeof(WCHAR))))
		return FALSE;

	// if readbuffer is too small to fit this record, increase it
	if (FObj->ReadBufferSize < Phfid->RecordSize)
	{
		PBYTE AllocTemp = realloc(FObj->ReadBuffer, Phfid->RecordSize);
		if (AllocTemp == NULL)
		{
			// If there is not enough memory, the old memory block is not freed and null pointer is returned
			free(FObj->ReadBuffer);
			FObj->ReadBuffer = NULL;
			return FALSE;
		}

		// realloc moves data, just update pointers
		FObj->ReadBuffer = AllocTemp;
		Phfid = (PCHashedFileInfoData)FObj->ReadBuffer;
		FObj->ReadBufferSize = Phfid->RecordSize;
	}

	// and read the file name
	if (!ReadFileEOF(FObj->hFile, FObj->ReadBuffer + FObj->RecordFixedPartSize, Phfid->RecordSize - FObj->RecordFixedPartSize, NULL))
		return FALSE;

	// check the last char of file name is the null-terminator
	LastChar = (PWCHAR)(FObj->ReadBuffer + Phfid->RecordSize - sizeof(WCHAR));
	if (*LastChar)
		return FALSE;

	// set pointers	for return values
	*HashedFileSizeLow  = Phfid->FileSizeLow;
	*HashedFileSizeHigh = Phfid->FileSizeHigh;
	*HashBlock          = (PBYTE)Phfid->data;
	*HashedFileName     = (LPWSTR)(FObj->ReadBuffer + FObj->RecordFixedPartSize);


	return TRUE;
}

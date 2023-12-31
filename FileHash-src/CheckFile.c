/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <Windows.h>
#include <tchar.h>
#include "SharedHeaders.h"
#include "HashBlock.h"
#include "GlobalOptions.h"
#include "CheckFile.h"


// new checkfile creation
BOOL CheckFileCreate(LPCTSTR FileName, CONST BOOL Overwrite, TCAlgorithms Algorithms, PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	DWORD NumberOfBytesWritten;
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
	FObj->FileNameFull = malloc(ExtendedPathMaxChar * sizeof(TCHAR));
	if (!FObj->FileNameFull)
		ExitFunction(FALSE);
	if (!GetFullPathName(FileName, ExtendedPathMaxChar, FObj->FileNameFull, NULL))
		ExitFunction(FALSE);

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

	if (!WriteFile(FObj->hFile, &FObj->Header, sizeof(FObj->Header), &NumberOfBytesWritten, NULL))
		ExitFunction(FALSE);


function_end:
	if (!Result)
		CheckFileClose(FObj);

	return Result;
}

// open an existing checkfile
BOOL CheckFileOpen(LPCTSTR FileName, PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	DWORD NumberOfBytesRead;
	TAlgorithms buff = HashAlgoAll;
	BOOL EOFile;


	// init the checkfile object
	ZeroMemory(FObj, sizeof(TCheckFileObj));
	FObj->Operation = CheckFileOperationRead;

	// open the file and read the header
	FObj->hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FObj->hFile == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);

	if (!ReadFileEOF(FObj->hFile, &FObj->Header, sizeof(FObj->Header), &NumberOfBytesRead, &EOFile))
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
	
	buff = ~buff;
	if ((FObj->Header.InfoHeader.Algorithms & buff) > 0)
		ExitFunction(FALSE);

	// init read buffer
	FObj->ReadBuffer = malloc(BufferReadSizeDefault);
	FObj->ReadBufferSize = BufferReadSizeDefault;

	if (FObj->ReadBuffer == NULL)
		ExitFunction(FALSE);

	// get some metainformation for convenience
	FObj->HashBlockSize = HashBlockSizeGet(FObj->Header.InfoHeader.Algorithms);


function_end:
	if (!Result)
		CheckFileClose(FObj);

	return Result;
}

// close the checkfile
BOOL CheckFileClose(PCheckFileObj FObj)
{
	BOOL Result = TRUE;
	DWORD NumberOfBytesWritten;


	if (FObj->Operation == CheckFileOperationCreate)
	{
		if (SetFilePointer(FObj->hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			ExitFunction(FALSE);

		if (!WriteFile(FObj->hFile, &FObj->Header, sizeof(FObj->Header), &NumberOfBytesWritten, NULL))
			ExitFunction(FALSE);
	}


function_end:
	CloseHandle(FObj->hFile);
	if (!FObj->Header.InfoHeader.FilesTotalCount)
		if (FObj->CanDelete)
			DeleteFile(FObj->FileName);

	CheckFileFreeBuffers(FObj);

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

// add new record to the checkfile
BOOL CheckFileWrite
(
	PCheckFileObj FObj, 
	LPCTSTR FileName,
	CONST DWORD FileSizeHigh,
	CONST DWORD FileSizeLow,
	PBYTE PHashBlock,
	CONST DWORD HashBlockSize
)
{
	DWORD NumberOfBytesWritten;
	THashedFileInfoData fid;
	SIZE_T StringLength = _tcslen(FileName) + 1;
	DWORD StringBytes;
	ULARGE_INTEGER BufferSA, BufferSB;


	// simple data validity check
	if (StringLength > ExtendedPathMaxChar)
		return FALSE;

	StringBytes = (DWORD)(StringLength * sizeof(TCHAR));

	// increase the "files count" in the header
	FObj->Header.InfoHeader.FilesTotalCount += 1;
	// increase the "total files size" in the header
	BufferSA.HighPart = FileSizeHigh;
	BufferSA.LowPart = FileSizeLow;
	BufferSB.HighPart = FObj->Header.InfoHeader.FileSizeHigh;
	BufferSB.LowPart = FObj->Header.InfoHeader.FileSizeLow;
	BufferSA.QuadPart += BufferSB.QuadPart;
	FObj->Header.InfoHeader.FileSizeHigh = BufferSA.HighPart;
	FObj->Header.InfoHeader.FileSizeLow = BufferSA.LowPart;

	// prepare the record header, fill data and
	// calculate the record size in the file
	fid.RecordSize =
		sizeof(fid) +
		HashBlockSize +
		StringBytes;
	fid.FileSizeHigh = FileSizeHigh;
	fid.FileSizeLow = FileSizeLow;

	// save the header
	if (!WriteFile(FObj->hFile, &fid, sizeof(fid), &NumberOfBytesWritten, NULL))
		return FALSE;
	// save data
	if (!WriteFile(FObj->hFile, PHashBlock, HashBlockSize, &NumberOfBytesWritten, NULL))
		return FALSE;
	if (!WriteFile(FObj->hFile, FileName, StringBytes, &NumberOfBytesWritten, NULL))
		return FALSE;

	return TRUE;
}

// read a record from the checkfile
BOOL CheckFileRead
(
	PCheckFileObj FObj,
	LPVOID FileName,
	PDWORD FileSizeHigh,
	PDWORD FileSizeLow,
	LPVOID HashBlock,
	PBOOL EOFile
)
{
	*EOFile = FALSE;
	DWORD NumberOfBytesRead;
	DWORD RecordSize;
	LONG Dist;
	PBYTE AllocTemp;
	LPTSTR BufferPFileName;
	PBYTE BufferPHashBlock;
	PTCHAR LastChar;
	PHashedFileInfoData fid;


	// read current record size
	if (!ReadFileEOF(FObj->hFile, &RecordSize, sizeof(RecordSize), &NumberOfBytesRead, EOFile))
		return FALSE;

	// basic check the size is correct and the file format is OK
	if (RecordSize > ((ExtendedPathMaxChar * sizeof(TCHAR)) + FObj->HashBlockSize + sizeof(fid)))
		return FALSE;

	// if readbuffer is too small for this record, increase it
	if (FObj->ReadBufferSize < RecordSize)
	{
		AllocTemp = realloc(FObj->ReadBuffer, RecordSize);
		if (AllocTemp == NULL)
		{
			// If there is not enough memory, the old memory block is not freed and null pointer is returned
			free(FObj->ReadBuffer);
			FObj->ReadBuffer = NULL;
			return FALSE;
		}

		FObj->ReadBuffer = AllocTemp;
		FObj->ReadBufferSize = RecordSize;
	}

	// set the file pointer a little back
	Dist = sizeof(RecordSize);
	Dist = 0 - Dist;
	if (SetFilePointer(FObj->hFile, Dist, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		return FALSE;

	// and read the whole header
	if (!ReadFile(FObj->hFile, FObj->ReadBuffer, RecordSize, &NumberOfBytesRead, NULL))
		return FALSE;
	if (!NumberOfBytesRead)
		return FALSE;

	// check the last char of file name is the null-terminator
	fid = (PHashedFileInfoData)FObj->ReadBuffer;	
	LastChar = (PTCHAR)(FObj->ReadBuffer + RecordSize - sizeof(TCHAR));
	if (*LastChar)
		return FALSE;

	// set pointers	for return values
	*FileSizeLow = fid->FileSizeLow;
	*FileSizeHigh = fid->FileSizeHigh;
	BufferPHashBlock = (PBYTE)fid->data;
	_memccpy(HashBlock, &BufferPHashBlock, 1, sizeof(BufferPHashBlock));
	BufferPFileName = (LPTSTR)(fid->data + FObj->HashBlockSize);
	_memccpy(FileName, &BufferPFileName, 1, sizeof(BufferPFileName));

	return TRUE;
}

BOOL ReadFileEOF(HANDLE HandleFile, LPVOID Buffer, DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, PBOOL EOFile)
{
	*EOFile = FALSE;
	
	if (!ReadFile(HandleFile, Buffer, NumberOfBytesToRead, NumberOfBytesRead, NULL))
		return FALSE;

	// check for EOF
	// If lpOverlapped is NULL, then when a synchronous read operation reaches the end of a file, ReadFile returns TRUE 
	// and sets *lpNumberOfBytesRead to zero
	if ((NumberOfBytesToRead != 0) && (*NumberOfBytesRead == 0))
	{
		*EOFile = TRUE;
		return FALSE;
	}

	return TRUE;
}

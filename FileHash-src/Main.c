/***

Copyright SV Foster, 2023-2024. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

    Rev 1, FEB 2024
       Added IncludeHidden option support
       Code refactoring

***/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "SharedHeaders.h"
#include "HashBlock.h"
#include "GlobalVariables.h"
#include "GlobalOptions.h"
#include "CommandLineInterface.h"
#include "CheckFile.h"
#include "PathWorks.h"
#include "resource.h"
#include "LanguageRes.h"
#include "FileIOAssist.h"
#include "Main.h"

#pragma warning( disable : 6255 )


DWORD _tmain(DWORD argc, LPCTSTR argv[], LPCTSTR envp[])
{
	DWORD Result = ExitCodeOK;
	TGlobalOptions GlobalOptions = { 0 };


	CLISetModeUTF16();
	SetErrorMode(SEM_FAILCRITICALERRORS); // Don't popup on floppy query and etc.
	CLIWriteLN();

	// collect parameters
	if (!CLIWorkModeGet(argc, argv, &GlobalOptions))
		return ExitCodeErrorInit;
	if (GlobalOptions.OperatingMode == OperatingModeHelp)
		return ModeHelp();
	if (!CLISwitchesGet(argc, argv, &GlobalOptions))
		return ExitCodeErrorInit;
	if (!CLIPathsGet(argc, argv, &GlobalOptions))
		return ExitCodeErrorInit;

	if (!GlobalOptions.NoCopyrightLogo)
		CLILogoPrint();

	CLIProgramStartOperationPrint(GlobalOptions.OperatingMode);
	CLIConsoleEventsInit(); // ctrl-c works now

	switch (GlobalOptions.OperatingMode)
	{
	case OperatingModeCreate:
		Result = ModeCreate(&GlobalOptions);
		break;

	case OperatingModeList:
		Result = ModeList(&GlobalOptions);
		break;

	case OperatingModeVerify:
		Result = ModeVerify(&GlobalOptions);
		break;

	default:
		Result = ExitCodeErrorInit;
	}

	if (CLIIsAbortedByUserDirect())
		Result = ExitCodeErrorUserStop;
	CLIProgramExitSummaryPrint(Result);
	return Result;
}

DWORD ModeHelp()
{
	CLILogoPrint();
	CLIHelpPrint();

	return ExitCodeOK;
}

DWORD ModeCreate(PGlobalOptions GlobalOptions)
{
	DWORD Result = ExitCodeOK;
	LPTSTR BufferPath;
	TCheckFileObj CheckFileObj = { 0 };
	SIZE_T i = 0;


	// main buffer for file path
	BufferPath = malloc(ExtendedPathMaxCharTerm * sizeof(TCHAR));
	if (!BufferPath)
		ExitFunction(ExitCodeErrorInit);

	// create new checkfile, write the header into it
	if (!CheckFileCreate(GlobalOptions->CheckFileName, GlobalOptions->CheckFileOverwrite, GlobalOptions->Algorithms, &CheckFileObj))
	{
		_tprintf_s(LangGet(UIMSG_104_CHECKFILE_CREATE_ERROR));
		ExitFunction(ExitCodeErrorIO);
	}

	// for every path provided by the user...
	while (GlobalOptions->PathListArray[i])
	{
		// copy path to the buffer and process it
		_tcscpy_s(BufferPath, ExtendedPathMaxCharTerm, GlobalOptions->PathListArray[i]);
		if (!ProcessPathString(BufferPath, &CheckFileObj, GlobalOptions->Algorithms, GlobalOptions->IsRecursive, !GlobalOptions->IncludeHidden))
		{
			_tprintf_s(LangGet(UIMSG_110_CANT_PROCESS_FILE), BufferPath);
			ExitFunction(ExitCodeErrorIO);
		}

		++i;
	}
	
	// done, check we've hashed some files
	if (CheckFileObj.Header.InfoHeader.FilesTotalCount)
		CLIFilePrintCreateTotal
		(
			UIMSG_109_TOTAL_PROCESSED,
			CheckFileObj.Header.InfoHeader.FilesTotalCount,
			CheckFileObj.Header.InfoHeader.FileSizeHigh,
			CheckFileObj.Header.InfoHeader.FileSizeLow,
			GlobalOptions->CheckFileName
		);
	else
		Result = ExitCodeErrorNoFiles; // if no files was hashed, return the specific error


function_end:
	if (!CheckFileClose(&CheckFileObj))
		if (Result == ExitCodeOK)
		{
			// report i/o error on checkfile close, only if this is the first error
			_tprintf_s(LangGet(UIMSG_105_CHECKFILE_FINISH_ERROR));
			Result = ExitCodeErrorIO;
		}
	free(BufferPath);

	return Result;
}

DWORD ModeVerify(PGlobalOptions GlobalOptions)
{
	DWORD Result = ExitCodeOK;
	LPTSTR BufferPath;
	TCheckFileObj CheckFileObj = { 0 };
	LPTSTR PFileName;
	DWORD FileSizeHigh;
	DWORD FileSizeLow;
	PBYTE BufferHashRead;
	BOOL EOFile;
	BY_HANDLE_FILE_INFORMATION FileInfo;	
	PBYTE BufferHash;
	DWORD FilesWErrors = 0;
	DWORD FilesChecked = 0;


	// main buffer for file path
	BufferPath = malloc(ExtendedPathMaxCharTerm * sizeof(TCHAR));
	if (!BufferPath)
		ExitFunction(ExitCodeErrorInit);

	// open and check file format/headers
	if (!CheckFileOpen(GlobalOptions->CheckFileName, &CheckFileObj))
	{
		_tprintf_s(LangGet(UIMSG_111_CHECKFILE_OPEN_ERROR));
		ExitFunction(ExitCodeErrorInit);
	}

	// get buffer for calculated hash
	BufferHash = _alloca(CheckFileObj.HashBlockSize);

	while (CheckFileRead(&CheckFileObj, &PFileName, &FileSizeHigh, &FileSizeLow, &BufferHashRead, &EOFile))
	{
		// read every record from the checkfile
		++FilesChecked;

		// copy file path from the checkfile, add user's provided path and normalyze the whole path string
		_tcscpy_s(BufferPath, ExtendedPathMaxCharTerm, GlobalOptions->PathListArray[0]);
		PathStringSlashLegacyNormalyze(BufferPath);
		PathStringSlashDoubleRemove(BufferPath);
		PathStringSlashTrailingIclude(BufferPath, ExtendedPathMaxCharTerm);
		_tcscat_s(BufferPath, ExtendedPathMaxCharTerm, PFileName);

		// print file name and size
		CLIFilePrintBasicData(PFileName, FileSizeHigh, FileSizeLow);
		CLIFilePrintHashData(BufferHashRead, CheckFileObj.Header.InfoHeader.Algorithms);

		// get attributes of the file and check the size matches
		if (!FileGetAttributes(BufferPath, &FileInfo))
		{
			_tprintf_s(LangGet(UIMSG_115_CHECK_FILE_NOT_FOUND));
			++FilesWErrors;
			continue;
		}

		if ((FileSizeHigh != FileInfo.nFileSizeHigh) || (FileSizeLow != FileInfo.nFileSizeLow))
		{
			_tprintf_s(LangGet(UIMSG_116_CHECK_FILE_WRONG_SIZE));
			++FilesWErrors;
			continue;
		}

		// hash file on the drive
		if (!FileHashAlgoBunch(BufferPath, BufferHash, CheckFileObj.Header.InfoHeader.Algorithms))
			ExitFunction(ExitCodeErrorIO);

		// compare hash(es) from checkfile and calculated one
		if (memcmp(BufferHashRead, BufferHash, CheckFileObj.HashBlockSize))
		{
			// print an error if hashes don't match
			_tprintf_s(LangGet(UIMSG_120_CHECK_FILE_WRONG_HASH));
			CLIFilePrintHashData(BufferHash, CheckFileObj.Header.InfoHeader.Algorithms);
			CLIWriteLN();
			++FilesWErrors;
			continue;
		}

		_tprintf_s(LangGet(UIMSG_114_CHECKFILE_OK));
	}

	// CheckFileRead() returned w/EOF = TRUE
	if (EOFile)
	{
		CLIFilePrintCheckTotal
		(
			FilesChecked, 
			CheckFileObj.Header.InfoHeader.FilesTotalCount, 
			CheckFileObj.Header.InfoHeader.FileSizeHigh,
			CheckFileObj.Header.InfoHeader.FileSizeLow,
			FilesWErrors,
			GlobalOptions->CheckFileName
		);

		// set return code to report the specific error
		if (FilesWErrors)
			ExitFunction(ExitCodeWrongHash);

		ExitFunction(ExitCodeOK);
	}

	_tprintf_s(LangGet(UIMSG_113_CANT_READ_CHECKFILE));
	ExitFunction(ExitCodeErrorIO);


function_end:
	CheckFileClose(&CheckFileObj);
	free(BufferPath);

	return Result;
}

DWORD ModeList(PGlobalOptions GlobalOptions)
{
	DWORD Result = ExitCodeOK;
	TCheckFileObj CheckFileObj = { 0 };
	LPTSTR PFileName;
	DWORD FileSizeHigh;
	DWORD FileSizeLow;
	PBYTE PHashBlock;
	BOOL EOFile;
	

	// open and check file format/headers
	if (!CheckFileOpen(GlobalOptions->CheckFileName, &CheckFileObj))
	{
		_tprintf_s(LangGet(UIMSG_111_CHECKFILE_OPEN_ERROR));
		ExitFunction(ExitCodeErrorInit);
	}

	while (CheckFileRead(&CheckFileObj, &PFileName, &FileSizeHigh, &FileSizeLow, &PHashBlock, &EOFile))
	{
		// print every record from the checkfile
		CLIFilePrintBasicData(PFileName, FileSizeHigh, FileSizeLow);
		CLIFilePrintHashData(PHashBlock, CheckFileObj.Header.InfoHeader.Algorithms);
		CLIWriteLN();

		// did user press ctrl-c?
		if (CLIIsAbortedByUser())
			break;
	}
	
	// an error while reading the file, CheckFileRead() returned w/EOF = FALSE
	if (!EOFile)
	{
		_tprintf_s(LangGet(UIMSG_113_CANT_READ_CHECKFILE));
		ExitFunction(ExitCodeErrorIO);
	}

	// print data from the header as summary
	CLIFilePrintCreateTotal
	(
		UIMSG_112_TOTAL_LISTED,
		CheckFileObj.Header.InfoHeader.FilesTotalCount,
		CheckFileObj.Header.InfoHeader.FileSizeHigh,
		CheckFileObj.Header.InfoHeader.FileSizeLow,
		GlobalOptions->CheckFileName
	);


function_end:
	CheckFileClose(&CheckFileObj);

	return Result; 
}

// process the path string from the user and call function requiert
BOOL ProcessPathString(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, CONST BOOL IsRecursive, CONST BOOL SkipHidden)
{
	BY_HANDLE_FILE_INFORMATION hfi;
	SIZE_T pos;


	// normalyze path
	PathStringSlashLegacyNormalyze(PathString);
	PathStringSlashDoubleRemove(PathString);
	if (PathStringSlashFindLast(PathString, &pos))
		++pos;
	else
		pos = 0;

	// if has wildcards, it is a folder
	if (PathStringHasWildCards(PathString))
		return ProcessFolder(PathString, CheckFileObj, Algorithms, IsRecursive, SkipHidden, pos);

	if (!FileGetAttributes(PathString, &hfi))
		return FALSE;

	// is it hidden?
	if (hfi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		if (SkipHidden)
			return FALSE;

	// it it a folder?
	if (hfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		PathStringSlashTrailingIclude(PathString, ExtendedPathMaxCharTerm);
		pos = _tcslen(PathString);
		return ProcessFolder(PathString, CheckFileObj, Algorithms, IsRecursive, SkipHidden, pos);
	}
	
	// it is a file
	return ProcessFile(PathString, CheckFileObj, Algorithms, &hfi, pos);
}

BOOL ProcessFile(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, LPBY_HANDLE_FILE_INFORMATION ffd, CONST SIZE_T PathCut)
{
	PBYTE BufferHash = _alloca(HashBlockSizeGet(Algorithms));
	LPTSTR PathRelative = &PathString[PathCut];
	BOOL FileSkip;


	CLIFilePrintBasicData(PathString, ffd->nFileSizeHigh, ffd->nFileSizeLow);
	
	// do we need to skip this file, 'cause this is the checkfile?
	if (!IsFileSkip(PathString, CheckFileObj->FileNameFull, &FileSkip))
		return FALSE;
	
	if (FileSkip)
	{
		_tprintf_s(LangGet(UIMSG_129_FILE_SKIPPED));
		return TRUE;
	}

	// hash the file
	if (!FileHashAlgoBunch(PathString, BufferHash, Algorithms))
		return FALSE;

	CLIFilePrintHashData(BufferHash, Algorithms);
	CLIWriteLN();

	if (!CheckFileWrite(CheckFileObj, PathRelative, ffd->nFileSizeHigh, ffd->nFileSizeLow, BufferHash))
		return FALSE;

	return TRUE;
}

BOOL ProcessFolder(LPTSTR PathString, PCheckFileObj CheckFileObj, TCAlgorithms Algorithms, CONST BOOL IsRecursive, CONST BOOL SkipHidden, CONST SIZE_T PathCut)
{    
	DWORD Result = TRUE;
	WIN32_FIND_DATAW ffd;
	BY_HANDLE_FILE_INFORMATION hfi;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	SIZE_T PosCurrent;

	
	// add slash and *.* to the end
	FindFirstFilePathNormalyze(PathString, ExtendedPathMaxCharTerm);

	// start the search
	hFind = FindFirstFile(PathString, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);

	do {
		if ((_tcscmp(ffd.cFileName, TEXT(".")) == 0) || (_tcscmp(ffd.cFileName, TEXT("..")) == 0))
			continue;

		// now remove the file name (or wildcards) at the end of the path
		PathStringFileNameRemove(PathString);
		// and add the found one
		_tcscat_s(PathString, ExtendedPathMaxCharTerm, ffd.cFileName);

		// make some checks on the path
		if (!FileGetAttributes(PathString, &hfi))
			ExitFunction(FALSE);

		if (hfi.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)
			continue;
		
		if (hfi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			if (SkipHidden)
				continue;

		if (hfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (IsRecursive)
			{
				// hash in subdirectries
				PosCurrent = _tcslen(PathString);
				if (!ProcessFolder(PathString, CheckFileObj, Algorithms, IsRecursive, SkipHidden, PathCut))
					ExitFunction(FALSE);
				PathString[PosCurrent] = '\0';
			};
			continue;
		}
		
		if (!ProcessFile(PathString, CheckFileObj, Algorithms, &hfi, PathCut))
			ExitFunction(FALSE);

	} while (FindNextFile(hFind, &ffd));

	// FindNextFile() returned 'cause of an error or there are no more files in the folder?
	if (GetLastError() != ERROR_NO_MORE_FILES)
		ExitFunction(FALSE);


function_end:
	if (hFind != INVALID_HANDLE_VALUE)
		FindClose(hFind);

	return Result;
}

// hash a file with all algorithms provided
BOOL FileHashAlgoBunch(LPCTSTR PathString, PBYTE HashBuffer, TCAlgorithms Algorithms)
{
	SIZE_T Position = 0;
	PCHashAlgorithmsData Arr[HashAlgoCount+1];


	if (!HashBlockAlgoDataGet(Algorithms, (PCHashAlgorithmsData*)&Arr, sizeof(PCHashAlgorithmsData) * (HashAlgoCount+1)))
		return FALSE;

	for (SIZE_T i = 0; Arr[i] != NULL; ++i)
	{
		if (!FileHashAlgoSingle(PathString, &HashBuffer[Position], Arr[i]->DigestSize, Arr[i]->Win32APICode))
			return FALSE;

		Position += Arr[i]->DigestSize;
	}

	return TRUE;
}

// hash a file with a single algorithm provided
BOOL FileHashAlgoSingle(LPCTSTR PathString, PBYTE HashBuffer, DWORD BufferLength, ALG_ID AlgAPICode)
{
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	PBYTE BufferData;
	DWORD NumberOfBytesRead;
	DWORD Result = TRUE;


	BufferData = malloc(BufferIOSizeBytes);
	if (!BufferData)
		return FALSE;

	hFile = CreateFile(PathString, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		ExitFunction(FALSE);

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, 0))
		ExitFunction(FALSE);

	if (!CryptCreateHash(hProv, AlgAPICode, 0, 0, &hHash))
		ExitFunction(FALSE);

	do {
		if (!ReadFile(hFile, BufferData, BufferIOSizeBytes, &NumberOfBytesRead, NULL))
			ExitFunction(FALSE);

		if (!CryptHashData(hHash, BufferData, NumberOfBytesRead, 0))
			ExitFunction(FALSE);

		if (CLIIsAbortedByUser())
			ExitFunction(FALSE);
	} while (NumberOfBytesRead > 0);

	if (!CryptGetHashParam(hHash, HP_HASHVAL, HashBuffer, &BufferLength, 0))
		ExitFunction(FALSE);


function_end:
	if (hHash)
		CryptDestroyHash(hHash);
	if (hProv)
		CryptReleaseContext(hProv, 0);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	free(BufferData);

	return Result;
}

// check if we need to skip the file
BOOL IsFileSkip(LPCTSTR PathString, LPCTSTR PathCheckFile, PBOOL Skip)
{
	BOOL Result = TRUE;
	LPTSTR FileCurrentFullPath = malloc(ExtendedPathMaxCharTerm * sizeof(TCHAR));


	if (!FileCurrentFullPath)
		ExitFunction(FALSE);

	if (!GetFullPathName(PathString, ExtendedPathMaxCharTerm, FileCurrentFullPath, NULL))
		ExitFunction(FALSE);

	*Skip = (_tcscmp(FileCurrentFullPath, PathCheckFile) == 0);


function_end:
	free(FileCurrentFullPath);

	return Result;
}

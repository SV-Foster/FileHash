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
#include <io.h>
#include <fcntl.h>
#include "SharedHeaders.h"
#include "HashBlock.h"
#include "GlobalVariables.h"
#include "GlobalOptions.h"
#include "PathWorks.h"
#include "StringFormat.h"
#include "LanguageRes.h"
#include "resource.h"
#include "CommandLineInterface.h"

#pragma comment(lib, "Version.lib")

#pragma warning( disable : 6255 )


extern TGlobalVariables GlobalVariables;


BOOL CLIWorkModeGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	glo->OperatingMode = OperatingModeHelp;

	if (argc <= 1)
	{
		return TRUE;
	}

	for (DWORD i = 0; i < argc; ++i)
	{
		if (_tcsicmp(argv[i], TEXT("/help")) == 0)
		{
			return TRUE;
		}

		if (_tcsicmp(argv[i], TEXT("/?")) == 0)
		{
			return TRUE;
		}
	}

	if (_tcsicmp(argv[1], TEXT("c")) == 0)
	{
		glo->OperatingMode = OperatingModeCreate;
		return TRUE;
	}

	if (_tcsicmp(argv[1], TEXT("v")) == 0)
	{
		glo->OperatingMode = OperatingModeVerify;
		return TRUE;
	}

	if (_tcsicmp(argv[1], TEXT("l")) == 0)
	{
		glo->OperatingMode = OperatingModeList;
		return TRUE;
	}

	CLILogoPrint();
	_tprintf_s(LangGet(UIMSG_101_NO_COMMAND));
	return FALSE;
}

BOOL CLISwitchesGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	BOOL UserAlgoSet = FALSE;

	for (DWORD i = 1; i < argc; ++i)
	{
		if (_tcsicmp(argv[i], TEXT("/NoLogo")) == 0)
		{
			glo->NoCopyrightLogo = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/Recursive")) == 0)
		{
			glo->IsRecursive = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/Overwrite")) == 0)
		{
			glo->CheckFileOverwrite = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/IncludeHidden")) == 0)
		{
			glo->IncludeHidden = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/MD2")) == 0)
		{
			glo->Algorithms |= HashAlgoMD2;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/MD4")) == 0)
		{
			glo->Algorithms |= HashAlgoMD4;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/MD5")) == 0)
		{
			glo->Algorithms |= HashAlgoMD5;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/SHA1")) == 0)
		{
			glo->Algorithms |= HashAlgoSHA1;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/SHA256")) == 0)
		{
			glo->Algorithms |= HashAlgoSHA256;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/SHA384")) == 0)
		{
			glo->Algorithms |= HashAlgoSHA384;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/SHA512")) == 0)
		{
			glo->Algorithms |= HashAlgoSHA512;
			UserAlgoSet = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/AlgoALL")) == 0)
		{
			glo->Algorithms = HashAlgoAll;
			UserAlgoSet = TRUE;
			continue;
		}

		if (argv[i][0] == TEXT('/'))
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_121_ERR_PARAMS_BAD_SW), argv[i]);
			return FALSE;
		}
	}

	if (!UserAlgoSet)
		glo->Algorithms = AlgorithmsDefault;

	return TRUE;
}

BOOL CLIPathsGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	SIZE_T j = 0;

	for (DWORD i = 2; i < argc; ++i)
	{
		if (argv[i][0] == TEXT('/'))
		{
			continue;
		}

		if (glo->CheckFileName == NULL)
		{
			glo->CheckFileName = (LPTSTR)argv[i];
			continue;
		}

		if (j >= PathListArrayElements)
		{
			_tprintf_s(LangGet(UIMSG_103_COMMAND_LINE_TOO_LONG));
			return FALSE;
		}
		glo->PathListArray[j] = (LPTSTR)argv[i];
		j++;
	}

	switch (glo->OperatingMode)
	{
	case OperatingModeCreate:
		if (glo->CheckFileName)
			if (PathStringHasWildCards(glo->CheckFileName))
			{
				_tprintf_s(LangGet(UIMSG_130_WRONG_PATH_WWC), glo->CheckFileName);
				return FALSE;
			}

		if ((glo->CheckFileName) && (glo->PathListArray[0]))
			return TRUE;
			
		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_102_NO_FILENAME_OR_PATH));
		break;

	case OperatingModeList:
		if (glo->PathListArray[0])
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_119_ERR_PARAMS_TOO_MANY_AGRS), glo->PathListArray[0]);
			return FALSE;
		}

		if (glo->CheckFileName)
			if (PathStringHasWildCards(glo->CheckFileName))
			{
				_tprintf_s(LangGet(UIMSG_130_WRONG_PATH_WWC), glo->CheckFileName);
				return FALSE;
			}

		if (glo->CheckFileName)
			return TRUE;

		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_102_NO_FILENAME_OR_PATH));
		break;

	case OperatingModeVerify:
		if (glo->PathListArray[1])
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_119_ERR_PARAMS_TOO_MANY_AGRS), glo->PathListArray[1]);
			return FALSE;
		}

		if (glo->CheckFileName)
			if (PathStringHasWildCards(glo->CheckFileName))
			{
				_tprintf_s(LangGet(UIMSG_130_WRONG_PATH_WWC), glo->CheckFileName);
				return FALSE;
			}

		if (glo->PathListArray[0])
			if (PathStringHasWildCards(glo->PathListArray[0]))
			{
				_tprintf_s(LangGet(UIMSG_130_WRONG_PATH_WWC), glo->PathListArray[0]);
				return FALSE;
			}

		if ((glo->CheckFileName) && (glo->PathListArray[0]))
			return TRUE;

		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_102_NO_FILENAME_OR_PATH));
		break;

	default:
		break;
	}

	return FALSE;
}

VOID CLILogoPrint()
{
	TCHAR FileName[MAX_PATH + 1];
	DWORD FileNameSize;
	DWORD Handle;
	DWORD FileVersionInfoSize;
	LPVOID BufferData;
	UINT len;
	LPVOID CopyrightString;
	LPVOID ProductNameString;
	LPVOID versionInfo;
	VS_FIXEDFILEINFO* fileInfo;


	FileNameSize = GetModuleFileName(NULL, (LPTSTR)&FileName, MAX_PATH);
	if (!FileNameSize)
		return;
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return;
	FileName[FileNameSize] = TEXT('\0');

	FileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)&FileName, &Handle);
	if (!FileVersionInfoSize)
		return;

	BufferData = _alloca(FileVersionInfoSize);

	if (!GetFileVersionInfo((LPTSTR)&FileName, 0, FileVersionInfoSize, BufferData))
		return;

	if (!VerQueryValue(BufferData, TEXT("\\"), &versionInfo, &len))
		return;

	fileInfo = (VS_FIXEDFILEINFO*)versionInfo;
	if (fileInfo->dwSignature != 0xfeef04bd)
		return;

	if (!VerQueryValue(BufferData, TEXT("\\StringFileInfo\\040904B0\\FileDescription"), &ProductNameString, &len))
		return;

	if (!VerQueryValue(BufferData, TEXT("\\StringFileInfo\\040904B0\\LegalCopyright"), &CopyrightString, &len))
		return;

	_tprintf_s(LangGet(UIMSG_118_LOGO_TEXT),
		ProductNameString,
		HIWORD(fileInfo->dwProductVersionMS),
		LOWORD(fileInfo->dwProductVersionMS),
		ArchString,
		CopyrightString
	);
}

VOID CLIHelpPrint()
{
	_tprintf_s(LangGet(UIMSG_117_HELP_TEXT));
}

VOID CLIFilePrintBasicData(LPCTSTR FileName, CONST DWORD FileSizeHigh, CONST DWORD FileSizeLow)
{
	TCHAR BufferStr[ULONGLONGMAXLENGTH*2];

	StringFmtU64WithGroupingHLGet((PWCHAR)&BufferStr, ULONGLONGMAXLENGTH*2, FileSizeHigh, FileSizeLow, LOCALE_USER_DEFAULT);

	_tprintf_s(LangGet(UIMSG_106_PROCESSING_FILE), FileName);
	_tprintf_s(LangGet(UIMSG_107_FILE_BYTES), &BufferStr);
}

VOID CLIFilePrintHashData(PBYTE HashBlock, TCAlgorithms Algorithms)
{
	SIZE_T Position = 0;
	PCHashAlgorithmsData Arr[HashAlgoCount+1];

	if (!HashBlockAlgoDataGet(Algorithms, (PCHashAlgorithmsData*)&Arr, sizeof(PCHashAlgorithmsData) * (HashAlgoCount+1)))
		return;

	for (SIZE_T i = 0; Arr[i] != NULL; ++i)
	{
		_tprintf_s(TEXT("%s: "), Arr[i]->Name);
		CLIHashPrintByByte(&HashBlock[Position], Arr[i]->DigestSize);
		CLIWriteLN();
		Position += Arr[i]->DigestSize;
	}
}

VOID CLIFilePrintCreateTotal(CONST UINT MsgID,CONST DWORD FilesTotalCount, CONST DWORD SizeHigh, CONST DWORD SizeLow, LPCTSTR FileName)
{
	TCHAR BufferStr[ULONGLONGMAXLENGTH*2];

	StringFmtU64WithGroupingHLGet((PWCHAR)&BufferStr, ULONGLONGMAXLENGTH*2, SizeHigh, SizeLow, LOCALE_USER_DEFAULT);

	_tprintf_s(LangGet(MsgID), FilesTotalCount, BufferStr, FileName);
}

VOID CLIFilePrintCheckTotal
(
	CONST DWORD FilesChecked,
	CONST DWORD FilesTotalCount,
	CONST DWORD SizeHigh,
	CONST DWORD SizeLow,
	CONST DWORD FilesWErrors,
	LPCTSTR FileName
)
{
	TCHAR BufferStr[ULONGLONGMAXLENGTH*2];

	StringFmtU64WithGroupingHLGet((PWCHAR)&BufferStr, ULONGLONGMAXLENGTH*2, SizeHigh, SizeLow, LOCALE_USER_DEFAULT);

	_tprintf_s(LangGet(UIMSG_122_CHECK_FILE_RESULTS), 
		FilesChecked,
		FilesTotalCount,
		&BufferStr,
		FileName,
		FilesWErrors
		);
}

VOID CLIProgramStartOperationPrint(CONST TOperatingMode Mode)
{
	switch (Mode)
	{
	case OperatingModeCreate:
		_tprintf_s(LangGet(UIMSG_131_MODESTART_CREATE));
		break;

	case OperatingModeList:
		_tprintf_s(LangGet(UIMSG_132_MODESTART_LIST));
		break;

	case OperatingModeVerify:
		_tprintf_s(LangGet(UIMSG_133_MODESTART_VERIFY));
		break;

	default:
		break;
	}
}

VOID CLIProgramExitSummaryPrint(int Result)
{
	switch (Result)
	{
	case ExitCodeOK:
		_tprintf_s(LangGet(UIMSG_108_OPERATION_SUCCESS));
		break;

	case ExitCodeErrorNoFiles:
		_tprintf_s(LangGet(UIMSG_123_EXIT_NO_FILE_FOUND), Result);
		break;

	case ExitCodeErrorUserStop:
		_tprintf_s(LangGet(UIMSG_124_EXIT_USER_STOP), Result);
		break;

	case ExitCodeWrongHash:
		_tprintf_s(LangGet(UIMSG_125_EXIT_WRONG_HASH), Result);
		break;

	case ExitCodeErrorInit:
		_tprintf_s(LangGet(UIMSG_126_EXIT_BAD_INIT), Result);
		break;

	case ExitCodeErrorIO:
		_tprintf_s(LangGet(UIMSG_127_EXIT_IOERR), Result);
		break;

	default:
		break;
	}
}

VOID CLIWriteLN()
{
	_tprintf_s(TEXT("\n"));
}

VOID CLIHashPrintByByte(PBYTE HashBlock, CONST DWORD Length)
{
	for (DWORD i = 0; i < Length; ++i)
		_tprintf_s(TEXT("%02hhX"), HashBlock[i]);
}

BOOL CLISetModeUTF16()
{
	if (_setmode(_fileno(stdout), _O_U16TEXT) == -1)
		return FALSE;

	if (_setmode(_fileno(stdin), _O_U16TEXT) == -1)
		return FALSE;

	if (_setmode(_fileno(stderr), _O_U16TEXT) == -1)
		return FALSE;

	return TRUE;
}

BOOL WINAPI CLIConsoleHandler(DWORD signal)
{
	switch (signal)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		// Windows creates a new thread in the process to execute the handler function
		EnterCriticalSection(&GlobalVariables.BreakCrSec);		
		GlobalVariables.ProcessBreak = TRUE;
		LeaveCriticalSection(&GlobalVariables.BreakCrSec);
		_tprintf_s(LangGet(UIMSG_128_CTRLC_PRESSED));

		return TRUE;

	default:
		return FALSE;
	}
}

// critical section call takes time, so check for abortion only once in UserAbortionCheckInterval to increase the perfomance
BOOL CLIIsAbortedByUser()
{
	static ULONGLONG CheckTimeLast = 0;
	ULONGLONG TimeCurrent = GetTickCount64();

	// timer overflow check
	if (TimeCurrent < CheckTimeLast)
		CheckTimeLast = 0;
	
	if (TimeCurrent - CheckTimeLast < UserAbortionCheckInterval)
		return FALSE;
	CheckTimeLast = TimeCurrent;
	
	return CLIIsAbortedByUserDirect();
}

BOOL CLIIsAbortedByUserDirect()
{
	BOOL Result;

	EnterCriticalSection(&GlobalVariables.BreakCrSec);
	Result = GlobalVariables.ProcessBreak;
	LeaveCriticalSection(&GlobalVariables.BreakCrSec);

	return Result;
}

VOID CLIConsoleEventsInit()
{
	GlobalVariables.ProcessBreak = FALSE;
	InitializeCriticalSection(&GlobalVariables.BreakCrSec);
	SetConsoleCtrlHandler(CLIConsoleHandler, TRUE);
}

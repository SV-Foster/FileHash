/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

***/

#include <windows.h>
#include <tchar.h>
#include "PathWorks.h"


// replace all legacy unix slashes to the normal ones
VOID PathStringSlashLegacyNormalyze(LPTSTR PathString)
{	
	LPTSTR s = PathString;
	while (*s != TEXT('\0'))
	{
		if (*s == TEXT('/'))
			*s = TEXT('\\');
		++s;
	}
}

/*
	D:\\Books\\\Code\Misc
	\\?\D:\\Books\\\Code\Misc
	\\server\\foldershared\\books

	--- to ---

	D:\Books\Code\Misc
	\\?\D:\Books\Code\Misc
	\\server\foldershared\books
*/
VOID PathStringSlashDoubleRemove(LPTSTR PathString)
{
	SIZE_T Length = _tcslen(PathString);
	if (Length < 2)
		return;

	LPTSTR s = PathString;
	LPTSTR b;

	/* skip first two slashes if \\?\ or \\server\ is specified */
	if (Length > 2)
	{
		if ((PathString[0] == TEXT('\\')) &&
			(PathString[1] == TEXT('\\')))
			s = PathString + 2;
	}

	// remove other doubles
	while (s[1] != TEXT('\0'))
	{
		if (s[0] == TEXT('\\') && s[1] == TEXT('\\'))
		{
			b = s;
			do {
				++b;
				b[0] = b[1];
			} while (*b != TEXT('\0'));

			continue;
		}

		++s;
	}
}

VOID FindFirstFilePathNormalyze(LPTSTR PathString, CONST DWORD LengthMax)
{
	/*
		wildcards are provided
		D:\Books\*.pdf
		D:\Books\Robert Martin Clean ????.epub
		..\*The Road Ah*.xps
	*/
	if (PathStringHasWildCards(PathString))
		return;

	/*
		w/trailing backslash, aka

		D:\
		D:\Books\
		\\?\D:\Books\
		..\Datasheets\
		System Architecture\
		.\System Architecture\
	*/
	if (_tcsstr(PathString, TEXT("\\")) != NULL)
		if (PathString[_tcslen(PathString) - 1] == TEXT('\\'))
		{
			_tcscat_s(PathString, LengthMax, TEXT("*"));
			return;
		}

	/*
		all other cases

		D:
		D:\Books
		\\?\D:\Books
		..\Datasheets
		System Architecture
		.\System Architecture
	*/
	_tcscat_s(PathString, LengthMax, TEXT("\\*"));
}

VOID PathStringSlashTrailingIclude(LPTSTR PathString, CONST DWORD LengthMax)
{
	SIZE_T Length = _tcslen(PathString);
	if (Length > 1)
		if (PathString[Length-1] == TEXT('\\'))
			return;

	_tcscat_s(PathString, LengthMax, TEXT("\\"));
}

BOOL PathStringSlashFindLast(LPCTSTR PathString, PSIZE_T Position)
{
	LPCTSTR StrEnd = PathString + _tcslen(PathString);
	while (StrEnd != PathString) {
		if (StrEnd[0] == TEXT('\\'))
		{
			*Position = StrEnd - PathString;
			return TRUE;
		}
		--StrEnd;
	}

	return FALSE;
}

VOID PathStringFileNameRemove(LPTSTR PathString)
{
	SIZE_T pos;
	LPTSTR Buffer;


	if (PathString == NULL)
		return;

	// ignore UNC and long path slashes
	if (_tcsstr(PathString, TEXT("\\\\")) == PathString)
	{
		// long path
		if (_tcsstr(PathString, TEXT("\\\\?\\")) == PathString)
		{
			PathString += 4;
			goto sub_end;
		}

		// local DOS device namespace
		if (_tcsstr(PathString, TEXT("\\\\.\\")) == PathString)
		{
			PathString += 4;
			goto sub_end;
		}

		// UNC
		// server name
		PathString += 2;
		Buffer = _tcsstr(PathString, TEXT("\\"));
		if (!Buffer)
			goto sub_end;
		PathString = Buffer;
		// share name
		Buffer = _tcsstr(PathString, TEXT("\\"));
		if (!Buffer)
			goto sub_end;
		PathString = Buffer;
	
	}
sub_end:

	// just a drive name. like D:
	if (_tcslen(PathString) == 2)
		if (_tcsstr(PathString, TEXT(":")) == (PathString + 1))
			return;

	// no slashes at all -- this is a file in the current folder
	if (!PathStringSlashFindLast(PathString, &pos))
	{
		PathString[0] = TEXT('\0');
		return;
	}

	/* 
	   slash at the end means this is path to a folder, aka
	   D:\ or C:\Windows\Temp\
	*/
	if (PathString[pos + 1] == TEXT('\0'))
		return;

	PathString[pos + 1] = TEXT('\0');
}

BOOL PathStringHasWildCards(LPCTSTR PathString)
{
	if ((_tcsstr(PathString, TEXT("*")) != NULL) || (_tcsstr(PathString, TEXT("?")) != NULL))
		return TRUE;

	return FALSE;
}

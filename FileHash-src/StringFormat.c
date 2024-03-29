/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
    Rev 0, DEC 2023
       First revision

***/

#include <Windows.h>
#include <tchar.h>
#include "StringFormat.h"


// get formatted string from an ULARGE_INTEGER
BOOL StringFmtU64WithGroupingHLGet(LPTSTR Str, CONST DWORD StrLength, CONST DWORD SizeHigh, CONST DWORD SizeLow, CONST LCID lcid)
{
	TCHAR BufferStr[ULONGLONGMAXLENGTH * 2];
	ULARGE_INTEGER Buffer;
	Buffer.HighPart = SizeHigh;
	Buffer.LowPart = SizeLow;
	NUMBERFMT fmt;
	TCHAR DecimalSep[MAX_LCL_STRING + 1];
	TCHAR ThousandSep[MAX_LCL_STRING + 1];
	fmt.lpDecimalSep = DecimalSep;
	fmt.lpThousandSep = ThousandSep;
	int CallResult;


	// get formatting options from Windows for current user
	if (!StringFmtDefaultNumberFmtGet(lcid, &fmt, ARRAYSIZE(DecimalSep), ARRAYSIZE(ThousandSep)))
		return FALSE;

	// set parameters desired:
	// no decimal digits
	fmt.NumDigits = 0;

	// get formatted string
	_stprintf_s(BufferStr, ULONGLONGMAXLENGTH * 2, TEXT("%llu"), Buffer.QuadPart);
	CallResult = GetNumberFormat
	(
		lcid,
		0,
		BufferStr,
		&fmt,
		Str,
		StrLength
	);

	return (CallResult > 0);
}

UINT StringFmtGroupingStrToUint(LPCTSTR Grouping)
{
	LPCTSTR Curr = Grouping;
	UINT Result = 0;
	LPTSTR LastProcessed;


	if (!Curr)
		return 0;

	for (;;)
	{
		Result *= 10;
		if (*Curr == TEXT('\0'))
			break;

		// Convert string to a long integer value
		Result += _tcstol(Curr, &LastProcessed, 10);
		if ((errno == ERANGE) || (errno == EINVAL))
			return 0;

		if (_tcscmp(LastProcessed, TEXT(";0")) == 0)
			break;

		Curr = LastProcessed + 1;
	}

	return Result;
}

BOOL StringFmtDefaultNumberFmtGet(CONST LCID lcid, LPNUMBERFMT NumberFmt, int DecimalSep, int ThousandSep)
{
	TCHAR Buffer[MAX_LCL_STRING + 1];


	if (!GetLocaleInfo(lcid, LOCALE_IDIGITS, Buffer, ARRAYSIZE(Buffer)))
		return FALSE;
	NumberFmt->NumDigits = _tcstol(Buffer, NULL, 10);

	if (!GetLocaleInfo(lcid, LOCALE_ILZERO, Buffer, ARRAYSIZE(Buffer)))
		return FALSE;
	NumberFmt->LeadingZero = _tcstol(Buffer, NULL, 10);

	if (!GetLocaleInfo(lcid, LOCALE_SGROUPING, Buffer, ARRAYSIZE(Buffer)))
		return FALSE;
	NumberFmt->Grouping = StringFmtGroupingStrToUint(Buffer);

	if (!GetLocaleInfo(lcid, LOCALE_SDECIMAL, NumberFmt->lpDecimalSep, DecimalSep))
		return FALSE;

	if (!GetLocaleInfo(lcid, LOCALE_STHOUSAND, NumberFmt->lpThousandSep, ThousandSep))
		return FALSE;

	if (!GetLocaleInfo(lcid, LOCALE_INEGNUMBER, Buffer, ARRAYSIZE(Buffer)))
		return FALSE;
	NumberFmt->NegativeOrder = _tcstol(Buffer, NULL, 10);

	return TRUE;
}

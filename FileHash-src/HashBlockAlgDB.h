/***

Copyright SV Foster, 2023. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:
	Rev 0, DEC 2023
	   First revision

***/

#pragma once


// Keep in mind that because the HashAlgDB array is defined directly in the header file, it will be included in every .c* file 
// where this header is included. This means that the size of the resulting object files may be larger than necessary
static TCHashAlgorithmsData HashAlgDB[HashAlgoCount] =
{
	{ HashAlgoMD2,	  CALG_MD2,     16, TEXT("MD2")   },
	{ HashAlgoMD4,	  CALG_MD4,     16, TEXT("MD4")   },
	{ HashAlgoMD5,	  CALG_MD5,     16, TEXT("MD5")   },
	{ HashAlgoSHA1,	  CALG_SHA1,    20, TEXT("SHA1")  },
	{ HashAlgoSHA256, CALG_SHA_256, 32, TEXT("SHA256")},
	{ HashAlgoSHA384, CALG_SHA_384,	48, TEXT("SHA384")},
	{ HashAlgoSHA512, CALG_SHA_512, 64, TEXT("SHA512")}
};

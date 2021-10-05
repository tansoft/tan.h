// RegExpDll.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include "RegExpDll.h"
#include <atlrx.h>

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return TRUE;
}

BOOL CheckRegExp(const char* szStr,const char* szExpr)
{
	CAtlRegExp<> reStr;
	CAtlREMatchContext<> mc;
	// five match groups: scheme, authority, path, query, fragment
	REParseError status = reStr.Parse(szExpr);
//		"({[^:/?#]+}:)?(//{[^/?#]*})?{[^?#]*}(?{[^#]*})?(#{.*})?" );
	if (REPARSE_ERROR_OK != status)
	{
		// Unexpected error.
		return FALSE;
	}
	if (!reStr.Match(szStr,	&mc))
	{
		// Unexpected error.
		return FALSE;
	}
	return TRUE;
	//for (UINT nGroupIndex = 0; nGroupIndex < mcUrl.m_uNumGroups;++nGroupIndex)
	//{
	//	const CAtlREMatchContext<>::RECHAR* szStart = 0;
	//	const CAtlREMatchContext<>::RECHAR* szEnd = 0;
	//	mcUrl.GetMatch(nGroupIndex, &szStart, &szEnd);

	//	ptrdiff_t nLength = szEnd - szStart;
	//	ATLTRACE("%d: \"%.*s\"\n", nGroupIndex, nLength, szStart);
	//}

}
#ifndef STRTOOLS2013_H
#define STRTOOLS2013_H
#ifdef _WIN32
#pragma once
#endif

#include "sdk/annotations.h"

#ifdef __cplusplus
// This is the preferred Min operator. Using the MIN macro can lead to unexpected
// side-effects or more expensive code.
template< class T >
T Min( T const &val1, T const &val2 )
{
	return val1 < val2 ? val1 : val2;
}

// This is the preferred Max operator. Using the MAX macro can lead to unexpected
// side-effects or more expensive code.
template< class T >
T Max( T const &val1, T const &val2 )
{
	return val1 > val2 ? val1 : val2;
}

#endif

int V_vsnprintfRet( OUT_Z_CAP(maxLenInCharacters) char *pDest, int maxLenInCharacters, PRINTF_FORMAT_STRING const char *pFormat, va_list params, bool *pbTruncated );
template <size_t maxLenInCharacters> int V_vsprintfRet_safe( OUT_Z_ARRAY char (&pDest)[maxLenInCharacters], PRINTF_FORMAT_STRING const char *pFormat, va_list params, bool *pbTruncated ) { return V_vsnprintfRet( pDest, maxLenInCharacters, pFormat, params, pbTruncated ); }

#endif
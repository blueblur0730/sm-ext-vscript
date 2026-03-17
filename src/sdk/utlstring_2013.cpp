//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "sdk/strtools_2013.h"
#include "sdk/utlstring_2013.h"
#include "tier1/strtools.h"
#include "tier1/utlvector.h"
#include <ctype.h>

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Helper: Find s substring
//-----------------------------------------------------------------------------
static ptrdiff_t IndexOf( const char *pstrToSearch, const char *pstrTarget )
{
	const char *pstrHit = Q_strstr( pstrToSearch, pstrTarget );
	if ( pstrHit == NULL )
	{
		return -1;	// Not found.
	}
	return ( pstrHit - pstrToSearch );
}

//-----------------------------------------------------------------------------
// Simple string class. 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Either allocates or reallocates memory to the length
//
// Allocated space for length characters.  It automatically adds space for the 
// nul and the cached length at the start of the memory block.  Will adjust
// m_pString and explicitly set the nul at the end before returning.
void *CUtlStringNew::AllocMemory( uint32 length )
{
	void *pMemoryBlock;
	if ( m_pString )
	{
		pMemoryBlock = realloc( m_pString, length + 1 );
	}
	else
	{
		pMemoryBlock = malloc( length + 1 );
	}
	m_pString = (char*)pMemoryBlock;
	m_pString[ length ] = 0;

	return pMemoryBlock;
}

//-----------------------------------------------------------------------------
void CUtlStringNew::SetDirect( const char *pValue, int nChars )
{
	if ( pValue && nChars > 0 )
	{
		if ( pValue == m_pString )
		{
			AssertMsg( nChars == Q_strlen(m_pString), "CUtlStringNew::SetDirect does not support resizing strings in place." );
			return; // Do nothing. Realloc in AllocMemory might move pValue's location resulting in a bad memcpy.
		}

		Assert( nChars <= Min<int>( strnlen(pValue, nChars) + 1, nChars ) );
		AllocMemory( nChars );
		Q_memcpy( m_pString, pValue, nChars );
	}
	else
	{
		Purge();
	}

}


void CUtlStringNew::Set( const char *pValue )
{
	int length = pValue ? V_strlen( pValue ) : 0;
	SetDirect( pValue, length );
}

// Sets the length (used to serialize into the buffer )
void CUtlStringNew::SetLength( int nLen )
{
	if ( nLen > 0 )
	{
#ifdef _DEBUG
		int prevLen = m_pString ? Length() : 0;
#endif
		AllocMemory( nLen );
#ifdef _DEBUG
		if ( nLen > prevLen )
		{
			V_memset( m_pString + prevLen, 0xEB, nLen - prevLen );
		}
#endif
	}
	else
	{
		Purge();
	}
}

const char *CUtlStringNew::Get( ) const
{
	if (!m_pString)
	{
		return "";
	}
	return m_pString;
}

char *CUtlStringNew::GetForModify()
{
	if ( !m_pString )
	{
		// In general, we optimise away small mallocs for empty strings
		// but if you ask for the non-const bytes, they must be writable
		// so we can't return "" here, like we do for the const version - jd
		void *pMemoryBlock = malloc( 1 );
		m_pString = (char *)pMemoryBlock;
		*m_pString = 0;
	}

	return m_pString;
}

char CUtlStringNew::operator[]( int i ) const
{
	if ( !m_pString )
		return '\0';

	if ( i >= Length() )
	{
		return '\0';
	}

	return m_pString[i];
}

void CUtlStringNew::Clear()
{
	Purge();
}

void CUtlStringNew::Purge()
{
    free( m_pString );
    m_pString = NULL;
}

bool CUtlStringNew::IsEqual_CaseSensitive( const char *src ) const
{
	if ( !src )
	{
		return (Length() == 0);
	}
	return ( V_strcmp( Get(), src ) == 0 );
}

bool CUtlStringNew::IsEqual_CaseInsensitive( const char *src ) const
{
	if ( !src )
	{
		return (Length() == 0);
	}
	return ( V_stricmp( Get(), src ) == 0 );
}


void CUtlStringNew::ToLower()
{
	if ( !m_pString )
	{
		return;
	}

	V_strlower( m_pString );
}

void CUtlStringNew::ToUpper()
{
	if ( !m_pString )
	{
		return;
	}

	V_strupr( m_pString );
}

CUtlStringNew &CUtlStringNew::operator=( const CUtlStringNew &src )
{
	SetDirect( src.Get(), src.Length() );
	return *this;
}

CUtlStringNew &CUtlStringNew::operator=( const char *src )
{
	Set( src );
	return *this;
}

bool CUtlStringNew::operator==( const CUtlStringNew &src ) const
{
	if ( IsEmpty() )
	{
		if ( src.IsEmpty() )
		{
			return true;
		}

		return false;
	}
	else
	{
		if ( src.IsEmpty() )
		{
			return false;
		}

		return Q_strcmp( m_pString, src.m_pString ) == 0;
	}
}

CUtlStringNew &CUtlStringNew::operator+=( const CUtlStringNew &rhs )
{
	const int lhsLength( Length() );
	const int rhsLength( rhs.Length() );

	if (!rhsLength)
	{
		return *this;
	}

	const int requestedLength( lhsLength + rhsLength );

	AllocMemory( requestedLength );
	Q_memcpy( m_pString + lhsLength, rhs.m_pString, rhsLength );

	return *this;
}

CUtlStringNew &CUtlStringNew::operator+=( const char *rhs )
{
	const int lhsLength( Length() );
	const int rhsLength( V_strlen( rhs ) );
	const int requestedLength( lhsLength + rhsLength );

	if (!requestedLength)
	{
		return *this;
	}

	AllocMemory( requestedLength );
	Q_memcpy( m_pString + lhsLength, rhs, rhsLength );

	return *this;
}

CUtlStringNew &CUtlStringNew::operator+=( char c )
{
	const int lhsLength( Length() );

	AllocMemory( lhsLength + 1 );
	m_pString[ lhsLength ] = c;

	return *this;
}

CUtlStringNew &CUtlStringNew::operator+=( int rhs )
{
	Assert( sizeof( rhs ) == 4 );

	char tmpBuf[ 12 ];	// Sufficient for a signed 32 bit integer [ -2147483648 to +2147483647 ]
	V_snprintf( tmpBuf, sizeof( tmpBuf ), "%d", rhs );
	tmpBuf[ sizeof( tmpBuf ) - 1 ] = '\0';

	return operator+=( tmpBuf );
}

CUtlStringNew &CUtlStringNew::operator+=( double rhs )
{
	char tmpBuf[ 256 ];	// How big can doubles be???  Dunno.
	V_snprintf( tmpBuf, sizeof( tmpBuf ), "%lg", rhs );
	tmpBuf[ sizeof( tmpBuf ) - 1 ] = '\0';

	return operator+=( tmpBuf );
}

bool CUtlStringNew::MatchesPattern( const CUtlStringNew &Pattern, int nFlags ) const
{
	const char *pszSource = String();
	const char *pszPattern = Pattern.String();
	bool	bExact = true;

	while( 1 )
	{
		if ( ( *pszPattern ) == 0 )
		{
			return ( (*pszSource ) == 0 );
		}

		if ( ( *pszPattern ) == '*' )
		{
			pszPattern++;

			if ( ( *pszPattern ) == 0 )
			{
				return true;
			}

			bExact = false;
			continue;
		}

		int nLength = 0;

		while( ( *pszPattern ) != '*' && ( *pszPattern ) != 0 )
		{
			nLength++;
			pszPattern++;
		}

		while( 1 )
		{
			const char *pszStartPattern = pszPattern - nLength;
			const char *pszSearch = pszSource;

			for( int i = 0; i < nLength; i++, pszSearch++, pszStartPattern++ )
			{
				if ( ( *pszSearch ) == 0 )
				{
					return false;
				}

				if ( ( *pszSearch ) != ( *pszStartPattern ) )
				{
					break;
				}
			}

			if ( pszSearch - pszSource == nLength )
			{
				break;
			}

			if ( bExact == true )
			{
				return false;
			}

			if ( ( nFlags & PATTERN_DIRECTORY ) != 0 )
			{
				if ( ( *pszPattern ) != '/' && ( *pszSource ) == '/' )
				{
					return false;
				}
			}

			pszSource++;
		}

		pszSource += nLength;
	}
}

/*
int CUtlStringNew::Format( const char *pFormat, ... )
{
	va_list marker;

	va_start( marker, pFormat );
	int len = FormatV( pFormat, marker );
	va_end( marker );

	return len;
}

//--------------------------------------------------------------------------------------------------
// This can be called from functions that take varargs.
//--------------------------------------------------------------------------------------------------

int CUtlStringNew::FormatV( const char *pFormat, va_list marker )
{
	char tmpBuf[ 4096 ];	//< Nice big 4k buffer, as much memory as my first computer had, a Radio Shack Color Computer

	//va_start( marker, pFormat );
	int len = V_vsprintf_safe( tmpBuf, pFormat, marker );
	//va_end( marker );
	Set( tmpBuf );
	return len;
}
*/
//-----------------------------------------------------------------------------
// Strips the trailing slash
//-----------------------------------------------------------------------------
void CUtlStringNew::StripTrailingSlash()
{
	if ( IsEmpty() )
		return;

	int nLastChar = Length() - 1;
	char c = m_pString[ nLastChar ];
	if ( c == '\\' || c == '/' )
	{
		SetLength( nLastChar );
	}
}

void CUtlStringNew::FixSlashes( char cSeparator/*=CORRECT_PATH_SEPARATOR*/ )
{
	if ( m_pString )
	{
		V_FixSlashes( m_pString, cSeparator );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Indicates if the target string exists in this instance.
//			The index is negative if the target string is not found, otherwise it is the index in the string.
//-----------------------------------------------------------------------------
ptrdiff_t CUtlStringNew::IndexOf( const char *pstrTarget ) const
{
	return ::IndexOf( String(), pstrTarget );
}

//-----------------------------------------------------------------------------
// Trim functions
//-----------------------------------------------------------------------------
void CUtlStringNew::TrimLeft( char cTarget )
{
	int nIndex = 0;

	if ( IsEmpty() )
	{
		return;
	}

	while( m_pString[nIndex] == cTarget )
	{
		++nIndex;
	}

	// We have some whitespace to remove
	if ( nIndex > 0 )
	{
		memcpy( m_pString, &m_pString[nIndex], Length() - nIndex );
		SetLength( Length() - nIndex );
	}
}


void CUtlStringNew::TrimLeft( const char *szTargets )
{
	int i;

	if ( IsEmpty() )
	{
		return;
	}

	for( i = 0; m_pString[i] != 0; i++ )
	{
		bool bWhitespace = false;

		for( int j = 0; szTargets[j] != 0; j++ )
		{
			if ( m_pString[i] == szTargets[j] )
			{
				bWhitespace = true;
				break;
			}
		}

		if ( !bWhitespace )
		{
			break;
		}
	}

	// We have some whitespace to remove
	if ( i > 0 )
	{
		memcpy( m_pString, &m_pString[i], Length() - i );
		SetLength( Length() - i );
	}
}


void CUtlStringNew::TrimRight( char cTarget )
{
	const int nLastCharIndex = Length() - 1;
	int nIndex = nLastCharIndex;

	while ( nIndex >= 0 && m_pString[nIndex] == cTarget )
	{
		--nIndex;
	}

	// We have some whitespace to remove
	if ( nIndex < nLastCharIndex )
	{
		m_pString[nIndex + 1] = 0;
		SetLength( nIndex + 2 );
	}
}


void CUtlStringNew::TrimRight( const char *szTargets )
{
	const int nLastCharIndex = Length() - 1;
	int i;

	for( i = nLastCharIndex; i > 0; i-- )
	{
		bool bWhitespace = false;

		for( int j = 0; szTargets[j] != 0; j++ )
		{
			if ( m_pString[i] == szTargets[j] )
			{
				bWhitespace = true;
				break;
			}
		}

		if ( !bWhitespace )
		{
			break;
		}
	}

	// We have some whitespace to remove
	if ( i < nLastCharIndex )
	{
		m_pString[i + 1] = 0;
		SetLength( i + 2 );
	}
}


void CUtlStringNew::Trim( char cTarget )
{
	TrimLeft( cTarget );
	TrimRight( cTarget );
}


void CUtlStringNew::Trim( const char *szTargets )
{
	TrimLeft( szTargets );
	TrimRight( szTargets );
}


CUtlStringNew CUtlStringNew::Slice( int32 nStart, int32 nEnd ) const
{
	int length = Length();
	if ( length == 0 )
	{
		return CUtlStringNew();
	}

	if ( nStart < 0 )
		nStart = length - (-nStart % length);
	else if ( nStart >= length )
		nStart = length;

	if ( nEnd == INT32_MAX )
		nEnd = length;
	else if ( nEnd < 0 )
		nEnd = length - (-nEnd % length);
	else if ( nEnd >= length )
		nEnd = length;
	
	if ( nStart >= nEnd )
		return CUtlStringNew();

	const char *pIn = String();

	CUtlStringNew ret;
	ret.SetDirect( pIn + nStart, nEnd - nStart );
	return ret;
}

// Grab a substring starting from the left or the right side.
CUtlStringNew CUtlStringNew::Left( int32 nChars ) const
{
	return Slice( 0, nChars );
}

CUtlStringNew CUtlStringNew::Right( int32 nChars ) const
{
	return Slice( -nChars );
}

CUtlStringNew CUtlStringNew::Replace( char cFrom, char cTo ) const
{
	if (!m_pString)
	{
		return CUtlStringNew();
	}

	CUtlStringNew ret = *this;
	int len = ret.Length();
	for ( int i=0; i < len; i++ )
	{
		if ( ret.m_pString[i] == cFrom )
			ret.m_pString[i] = cTo;
	}

	return ret;
}

CUtlStringNew CUtlStringNew::Replace( const char *pchFrom, const char *pchTo, bool bCaseSensitive /*= false*/ ) const
{
	if ( !pchTo )
	{
		return Remove( pchFrom, bCaseSensitive );
	}

	int nTextToReplaceLength = pchFrom ? V_strlen( pchFrom ) : 0;
	CUtlStringNew outputString;
	const char *pSrc = Get();
	if ( pSrc )
	{
		while ( *pSrc )
		{
			char const *pNextOccurrence = bCaseSensitive ? V_strstr( pSrc, pchFrom ) : V_stristr( pSrc, pchFrom );
			if ( !pNextOccurrence )
			{
				// append remaining string
				outputString += pSrc;
				break;
			}

			int nNumCharsToCopy = pNextOccurrence - pSrc;
			if ( nNumCharsToCopy )
			{
				// append up to the undesired substring
				CUtlStringNew temp = pSrc;
				temp = temp.Left( nNumCharsToCopy );
				outputString += temp;
			}

			// Append the replacement
			outputString += pchTo;

			// skip past undesired substring
			pSrc = pNextOccurrence + nTextToReplaceLength;
		}
	}

	return outputString;
}

void CUtlStringNew::RemoveDotSlashes(char separator)
{
	V_RemoveDotSlashes(GetForModify(), separator);
}

// Get a string with the specified substring removed
CUtlStringNew CUtlStringNew::Remove( char const *pTextToRemove, bool bCaseSensitive ) const
{
	int nTextToTemoveLength = pTextToRemove ? V_strlen( pTextToRemove ) : 0;
	CUtlStringNew outputString;
	const char *pSrc = m_pString;
	if ( pSrc )
	{
		while ( *pSrc )
		{
			char const *pNextOccurrence = bCaseSensitive ? V_strstr( pSrc, pTextToRemove ) : V_stristr( pSrc, pTextToRemove );
			if ( !pNextOccurrence )
			{
				// append remaining string
				outputString += pSrc;
				break;
			}

			int nNumCharsToCopy = pNextOccurrence - pSrc;
			if ( nNumCharsToCopy )
			{
				// append up to the undesired substring
				outputString.Append( pSrc, nNumCharsToCopy );
			}

			// skip past undesired substring
			pSrc = pNextOccurrence + nTextToTemoveLength;
		}
	}

	return outputString;
}

CUtlStringNew CUtlStringNew::AbsPath( const char *pStartingDir ) const
{
	char szNew[MAX_PATH];
	V_MakeAbsolutePath( szNew, sizeof( szNew ), this->String(), pStartingDir );
	return CUtlStringNew( szNew );
}

CUtlStringNew CUtlStringNew::UnqualifiedFilename() const
{
	const char *pFilename = V_UnqualifiedFileName( this->String() );
	return CUtlStringNew( pFilename );
}

CUtlStringNew CUtlStringNew::DirName() const
{
	CUtlStringNew ret( this->String() );
	V_StripLastDir( (char*)ret.Get(), ret.Length() + 1 );
	V_StripTrailingSlash( (char*)ret.Get() );
	return ret;
}

CUtlStringNew CUtlStringNew::StripExtension() const
{
	char szTemp[MAX_PATH];
	V_StripExtension( String(), szTemp, sizeof( szTemp ) );
	return CUtlStringNew( szTemp );
}

CUtlStringNew CUtlStringNew::StripFilename() const
{
	const char *pFilename = V_UnqualifiedFileName( Get() ); // NOTE: returns 'Get()' on failure, never NULL
	int nCharsToCopy = pFilename - Get();
	CUtlStringNew result;
	result.SetDirect( Get(), nCharsToCopy );
	result.StripTrailingSlash();
	return result;
}

CUtlStringNew CUtlStringNew::GetBaseFilename() const
{
	char szTemp[MAX_PATH];
	V_FileBase( String(), szTemp, sizeof( szTemp ) );
	return CUtlStringNew( szTemp );
}

CUtlStringNew CUtlStringNew::GetExtension() const
{
	char szTemp[MAX_PATH];
	V_ExtractFileExtension( String(), szTemp, sizeof( szTemp ) );
	return CUtlStringNew( szTemp );
}


CUtlStringNew CUtlStringNew::PathJoin( const char *pStr1, const char *pStr2 )
{
	char szPath[MAX_PATH];
	V_ComposeFileName( pStr1, pStr2, szPath, sizeof( szPath ) );
	return CUtlStringNew( szPath );
}

CUtlStringNew CUtlStringNew::operator+( const char *pOther ) const
{
	CUtlStringNew s = *this;
	s += pOther;
	return s;
}

CUtlStringNew CUtlStringNew::operator+( const CUtlStringNew &other ) const
{
	CUtlStringNew s = *this;
	s += other;
	return s;
}

CUtlStringNew CUtlStringNew::operator+( int rhs ) const
{
	CUtlStringNew ret = *this;
	ret += rhs;
	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: concatenate the provided string to our current content
//-----------------------------------------------------------------------------
void CUtlStringNew::Append( const char *pchAddition )
{
	(*this) += pchAddition;
}

void CUtlStringNew::Append( const char *pchAddition, int nChars )
{
	nChars = Min<int>( nChars, V_strlen(	pchAddition ) );

	const int lhsLength( Length() );
	const int rhsLength( nChars );
	const int requestedLength( lhsLength + rhsLength );

	AllocMemory( requestedLength );
	const int allocatedLength( requestedLength );
	const int copyLength( allocatedLength - lhsLength < rhsLength ? allocatedLength - lhsLength : rhsLength );
	memcpy( GetForModify() + lhsLength, pchAddition, copyLength );
	m_pString[ allocatedLength ] = '\0';
}

// Shared static empty string.
const CUtlStringNew &CUtlStringNew::GetEmptyString()
{
	static const CUtlStringNew s_emptyString;

	return s_emptyString;
}
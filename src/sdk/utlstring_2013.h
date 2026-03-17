//====== Copyright Valve Corporation, All rights reserved. =======

#ifndef UTLSTRING_H
#define UTLSTRING_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/utlmemory.h"
#include "tier1/strtools.h"
#include "limits.h"

// Matched with the memdbgoff at end of header
#include "memdbgon.h"

#if defined( OSX )
#ifndef wcsdup
// The mem override tools may provide a copy of this if active, otherwise it is not available in OS X's libc due to
// being introduced in POSIX-20008
inline wchar_t *wcsdup(const wchar_t *pString)
{
	wchar_t *pMemory;

	if (!pString)
		return NULL;

	size_t len = (wcslen(pString) + 1);
	if ((pMemory = (wchar_t *)malloc(len * sizeof(wchar_t))) != NULL)
	{
		return wcscpy( pMemory, pString );
	}

	return NULL;
}
#endif

inline size_t strnlen(const char *s, size_t n)
{
	const char *p = (const char *)memchr(s, 0, n);
	return (p ? p - s : n);
}

#endif

#define MOVE_CONSTRUCTOR_SUPPORT

// The islower/isdigit/etc. functions all expect a parameter that is either
// 0-0xFF or EOF. It is easy to violate this constraint simply by passing
// 'char' to these functions instead of unsigned char.
// The V_ functions handle the char/unsigned char mismatch by taking a
// char parameter and casting it to unsigned char so that chars with the
// sign bit set will be zero extended instead of sign extended.
// Not that EOF cannot be passed to these functions.
//
// These functions could also be used for optimizations if locale
// considerations make some of the CRT functions slow.
//#undef isdigit // In case this is implemented as a macro
//#define isdigit use_V_isdigit_instead_of_isdigit
inline bool V_isalpha(char c) { return isalpha( (unsigned char)c ) != 0; }
//#undef isalpha
//#define isalpha use_V_isalpha_instead_of_isalpha
inline bool V_isalnum(char c) { return isalnum( (unsigned char)c ) != 0; }
//#undef isalnum
//#define isalnum use_V_isalnum_instead_of_isalnum
inline bool V_isprint(char c) { return isprint( (unsigned char)c ) != 0; }
//#undef isprint
//#define isprint use_V_isprint_instead_of_isprint
inline bool V_isxdigit(char c) { return isxdigit( (unsigned char)c ) != 0; }
//#undef isxdigit
//#define isxdigit use_V_isxdigit_instead_of_isxdigit
inline bool V_ispunct(char c) { return ispunct( (unsigned char)c ) != 0; }
//#undef ispunct
//#define ispunct use_V_ispunct_instead_of_ispunct
inline bool V_isgraph(char c) { return isgraph( (unsigned char)c ) != 0; }
//#undef isgraph
//#define isgraph use_V_isgraph_instead_of_isgraph
inline bool V_isupper(char c) { return isupper( (unsigned char)c ) != 0; }
//#undef isupper
//#define isupper use_V_isupper_instead_of_isupper
inline bool V_islower(char c) { return islower( (unsigned char)c ) != 0; }
//#undef islower
//#define islower use_V_islower_instead_of_islower
inline bool V_iscntrl(char c) { return iscntrl( (unsigned char)c ) != 0; }
//#undef iscntrl
//#define iscntrl use_V_iscntrl_instead_of_iscntrl
inline bool V_isspace(char c) { return isspace( (unsigned char)c ) != 0; }
//#undef isspace
//#define isspace use_V_isspace_instead_of_isspace


//-----------------------------------------------------------------------------
// Simple string class. 
// NOTE: This is *not* optimal! Use in tools, but not runtime code
//-----------------------------------------------------------------------------
class CUtlStringNew
{
public:
	typedef enum
	{
		PATTERN_NONE		= 0x00000000,
		PATTERN_DIRECTORY	= 0x00000001
	} TUtlStringPattern;

public:
	CUtlStringNew();
	CUtlStringNew( const char *pString );
	CUtlStringNew( const char *pString, int length );
	CUtlStringNew( const CUtlStringNew& string );

#ifdef MOVE_CONSTRUCTOR_SUPPORT
	// Support moving of CUtlStringNew objects. Long live C++11
	// This move constructor will get called when appropriate, such as when
	// returning objects from functions, or otherwise copying from temporaries
	// which are about to be destroyed. It can also be explicitly invoked with
	// std::move().
	// Move constructor:
	CUtlStringNew( CUtlStringNew&& rhs )
	{
		// Move the string pointer from the source to this -- be sure to
		// zero out the source to avoid double frees.
		m_pString = rhs.m_pString;
		rhs.m_pString = 0;
	}
	// Move assignment operator:
	CUtlStringNew& operator=( CUtlStringNew&& rhs )
	{
		// Move the string pointer from the source to this -- be sure to
		// zero out the source to avoid double frees.  SetPtr will free our existing
		// value if needed.
		SetPtr( rhs.m_pString );
		rhs.m_pString = 0;
		return *this;
	}

	void SetPtr( char *pszPtr )
	{
		if ( pszPtr == m_pString )
			return;

		Purge();
		m_pString = pszPtr;
	}
#endif

	~CUtlStringNew();

	const char	*Get( ) const;
	void		Set( const char *pValue );
	operator const char*() const;

	// Set directly and don't look for a null terminator in pValue.
	// nChars does not include the nul and this will only copy
	// at most nChars (even if pValue is longer).  If nChars
	// is >strlen(pValue) it will copy past the end, don't do it
	// Does nothing if pValue == String()
	void		SetDirect( const char *pValue, int nChars );

	char *Access() { return GetForModify(); }
	// for compatibility switching items from UtlSymbol
	const char  *String() const { return Get(); }

	// Returns strlen
	int			Length() const;
	// IsEmpty() is more efficient than Length() == 0
	bool		IsEmpty() const;

	// Sets the length (used to serialize into the buffer )
	// Note: If nLen != 0, then this adds an extra byte for a null-terminator.	
	void		SetLength( int nLen );
	char		*GetForModify();
	void		Clear();
	void		Purge();

	// Case Change
	void		ToLower();
	void		ToUpper();
	void		Append( const char *pAddition, int nChars );

	void		Append( const char *pchAddition );
	void		Append( const char chAddition ) { char temp[2] = { chAddition, 0 }; Append( temp ); }
	// Strips the trailing slash
	void		StripTrailingSlash();
	void		FixSlashes( char cSeparator = CORRECT_PATH_SEPARATOR );

	ptrdiff_t IndexOf( const char *pstrTarget ) const;

	// Trim whitespace
	void		TrimLeft( char cTarget );
	void		TrimLeft( const char *szTargets = "\t\r\n " );
	void		TrimRight( char cTarget );
	void		TrimRight( const char *szTargets = "\t\r\n " );
	void		Trim( char cTarget );
	void		Trim( const char *szTargets = "\t\r\n " );

	bool		IsEqual_CaseSensitive( const char *src ) const;
	bool		IsEqual_CaseInsensitive( const char *src ) const;

	void RemoveDotSlashes(char separator = CORRECT_PATH_SEPARATOR);

	CUtlStringNew AbsPath(const char *pStartingDir, bool bLowercaseName) const
	{
		CUtlStringNew result = AbsPath(pStartingDir);
		if (bLowercaseName)
		{
			result.ToLower();
		}
		return result;
	}

	CUtlStringNew &operator=( const CUtlStringNew &src );
	CUtlStringNew &operator=( const char *src );

	// Test for equality
	bool operator==( const CUtlStringNew &src ) const;
	bool operator!=( const CUtlStringNew &src ) const { return !operator==( src ); }

	CUtlStringNew &operator+=( const CUtlStringNew &rhs );
	CUtlStringNew &operator+=( const char *rhs );
	CUtlStringNew &operator+=( char c );
	CUtlStringNew &operator+=( int rhs );
	CUtlStringNew &operator+=( double rhs );

	CUtlStringNew operator+( const char *pOther ) const;
	CUtlStringNew operator+( const CUtlStringNew &other ) const;
	CUtlStringNew operator+( int rhs ) const;

	bool MatchesPattern( const CUtlStringNew &Pattern, int nFlags = 0 ) const;		// case SENSITIVE, use * for wildcard in pattern string

	char operator[]( int i ) const;

	// is valid?
	bool IsValid() const { return m_pString != NULL; }

#if ! defined(SWIG)
	// Don't let SWIG see the PRINTF_FORMAT_STRING attribute or it will complain.
	//int Format( const char *pFormat, ... )  FMTFUNCTION( 2, 3 );
	//int FormatV( const char *pFormat, va_list marker );
#else
	//int Format( const char *pFormat, ... );
	//int FormatV( const char *pFormat, va_list marker );
#endif

	void Truncate( int nChars );

	// Defining AltArgumentType_t hints that associative container classes should
	// also implement Find/Insert/Remove functions that take const char* params.
	typedef const char *AltArgumentType_t;

	// Get a copy of part of the string.
	// If you only specify nStart, it'll go from nStart to the end.
	// You can use negative numbers and it'll wrap around to the start.
	CUtlStringNew Slice( int32 nStart=0, int32 nEnd=INT_MAX ) const;

	// Get a substring starting from the left or the right side.
	CUtlStringNew Left( int32 nChars ) const;
	CUtlStringNew Right( int32 nChars ) const;

	// Get a string with all instances of one character replaced with another.
	CUtlStringNew Replace( char cFrom, char cTo ) const;

	// Replace all instances of specified string with another.
	CUtlStringNew Replace( const char *pszFrom, const char *pszTo, bool bCaseSensitive = false ) const;

	// Get a string with the specified substring removed
	CUtlStringNew Remove( char const *pTextToRemove, bool bCaseSensitive = false ) const;

	// Get this string as an absolute path (calls right through to V_MakeAbsolutePath).
	CUtlStringNew AbsPath( const char *pStartingDir=NULL ) const;	

	// Gets the filename (everything except the path.. c:\a\b\c\somefile.txt -> somefile.txt).
	CUtlStringNew UnqualifiedFilename() const;
	
	// Gets a string with one directory removed. Uses V_StripLastDir but strips the last slash also!
	CUtlStringNew DirName() const;

	// Get a string with the extension removed (with V_StripExtension).
	CUtlStringNew StripExtension() const;

	// Get a string with the filename removed (uses V_UnqualifiedFileName and also strips the last slash)
	CUtlStringNew StripFilename() const;

	// Get a string with the base filename (with V_FileBase).
	CUtlStringNew GetBaseFilename() const;

	// Get a string with the file extension (with V_FileBase).
	CUtlStringNew GetExtension() const;

	// Works like V_ComposeFileName.
	static CUtlStringNew PathJoin( const char *pStr1, const char *pStr2 );

	// These can be used for utlvector sorts.
	static int __cdecl SortCaseInsensitive( const CUtlStringNew *pString1, const CUtlStringNew *pString2 );
	static int __cdecl SortCaseSensitive( const CUtlStringNew *pString1, const CUtlStringNew *pString2 );

	// Empty string for those times when you need to return an empty string and
	// either don't want to pay the construction cost, or are returning a
	// const CUtlStringNew& and cannot just return "".
	static const CUtlStringNew &GetEmptyString();

private:
	// INTERNALS
	// AllocMemory allocates enough space for length characters plus a terminating zero.
	// Previous characters are preserved, the buffer is null-terminated, but new characters
	// are not touched.
	void *AllocMemory( uint32 length );

	// If m_pString is not NULL, it points to the start of the string, and the memory allocation.
	char *m_pString;
};

//	// If these are not defined, CUtlConstString as rhs will auto-convert
//	// to const char* and do logical operations on the raw pointers. Ugh.
//	inline friend bool operator<( const T *lhs, const CUtlConstStringBase &rhs ) { return rhs.Compare( lhs ) > 0; }
//	inline friend bool operator==( const T *lhs, const CUtlConstStringBase &rhs ) { return rhs.Compare( lhs ) == 0; }
//	inline friend bool operator!=( const T *lhs, const CUtlConstStringBase &rhs ) { return rhs.Compare( lhs ) != 0; }

inline bool operator==( const char *pString, const CUtlStringNew &utlString )
{
	return utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator!=( const char *pString, const CUtlStringNew &utlString )
{	
	return !utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator==( const CUtlStringNew &utlString, const char *pString )
{
	return utlString.IsEqual_CaseSensitive( pString );
}

inline bool operator!=( const CUtlStringNew &utlString, const char *pString )
{
	return !utlString.IsEqual_CaseSensitive( pString );
}



//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline CUtlStringNew::CUtlStringNew()
: m_pString( NULL )
{
}

inline CUtlStringNew::CUtlStringNew( const char *pString )
: m_pString( NULL )
{
	Set( pString );
}

inline CUtlStringNew::CUtlStringNew( const char *pString, int length )
: m_pString( NULL )
{
	SetDirect( pString, length );
}

inline CUtlStringNew::CUtlStringNew( const CUtlStringNew& string )
: m_pString( NULL )
{
	Set( string.Get() );
}

inline CUtlStringNew::~CUtlStringNew()
{
	Purge();
}

inline int CUtlStringNew::Length() const
{
	if (m_pString)
	{
		return V_strlen( m_pString );
	}
	return 0;
}

inline bool CUtlStringNew::IsEmpty() const
{
	return !m_pString || m_pString[0] == 0;
}

//-----------------------------------------------------------------------------
// Purpose: Truncates the string to the specified number of characters
//-----------------------------------------------------------------------------
inline void CUtlStringNew::Truncate( int nChars )
{
	if ( !m_pString )
		return;

	int nLen = V_strlen( m_pString );
	if ( nLen <= nChars )
		return;

	m_pString[nChars] = '\0';
}

inline int __cdecl CUtlStringNew::SortCaseInsensitive( const CUtlStringNew *pString1, const CUtlStringNew *pString2 )
{
	return V_stricmp( pString1->String(), pString2->String() );
}

inline int __cdecl CUtlStringNew::SortCaseSensitive( const CUtlStringNew *pString1, const CUtlStringNew *pString2 )
{
	return V_strcmp( pString1->String(), pString2->String() );
}

// Converts to c-strings
inline CUtlStringNew::operator const char*() const
{
	return Get();
}

#endif // UTLSTRING_2013_H
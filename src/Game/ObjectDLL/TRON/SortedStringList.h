// ----------------------------------------------------------------------- //
//
// MODULE  : SortedStringList.h
//
// PURPOSE : A plugin's string list for DEdit, sorted by name
//
// RETAIL: Tron 2.0 only, inline (no RTTI, no strings: the names of the
//         class, its members and its functions are ours). Written from the
//         plugins that build one on the stack: SecrezzerPlugin's (retail
//         Object.lto 0x10044570) and AINodeExitLevelByRezOutPlugin's
//         (0x1026e190) and AINodeUseObjectPlugin's (0x100a49b0). Its out-of-line STL code is retail's 0x10047230
//         (push_back), 0x10047760 (for_each), 0x100478f0 (Sort) and
//         0x10047980 (find).
//
// ----------------------------------------------------------------------- //

#ifndef __SORTEDSTRINGLIST_H__
#define __SORTEDSTRINGLIST_H__

#include <vector>
#include <string>
#include <algorithm>

class CSortedStringList
{
	public :

		typedef std::vector<std::string> StringArray;

		CSortedStringList( uint32 cMaxStrings, uint32 cMaxStringLength )
			: m_bSorted( false ),
			  m_cMaxStrings( cMaxStrings ),
			  m_cMaxStringLength( cMaxStringLength )
		{
		}

		// Add a string (not an empty one), cut to the longest string DEdit
		// takes. The strings
		// in m_FirstNames go first, in the order they are added.
		void AddString( const char* pszString )
		{
			if( !pszString || !pszString[0] )
				return;

			std::string sString;
			if( strlen( pszString ) >= m_cMaxStringLength )
				sString.assign( pszString, pszString + m_cMaxStringLength - 1 );
			else
				sString.assign( pszString, pszString + strlen( pszString ));

			if( m_FirstNames.end( ) != std::find( m_FirstNames.begin( ), m_FirstNames.end( ), sString ))
			{
				m_FirstStrings.push_back( sString );
			}
			else
			{
				m_Strings.push_back( sString );
				m_bSorted = false;
			}
		}

		void Sort( )
		{
			if( !m_bSorted )
			{
				std::sort( m_Strings.begin( ), m_Strings.end( ));
				m_bSorted = true;
			}
		}

		// Copy the strings to DEdit's list: the first strings, then the
		// others sorted.
		void PopulateStringList( char** aszStrings, uint32* pcStrings )
		{
			if( aszStrings && *aszStrings && pcStrings )
			{
				std::for_each( m_FirstStrings.begin( ), m_FirstStrings.end( ), CopyString( aszStrings, pcStrings, m_cMaxStrings ));
				Sort( );
				std::for_each( m_Strings.begin( ), m_Strings.end( ), CopyString( aszStrings, pcStrings, m_cMaxStrings ));
			}
		}

	protected :

		// Copies one string to the next free entry of DEdit's list.
		struct CopyString
		{
			CopyString( char** aszStrings, uint32* pcStrings, uint32 cMaxStrings )
				: m_aszStrings( aszStrings ),
				  m_pcStrings( pcStrings ),
				  m_cMaxStrings( cMaxStrings )
			{
			}

			void operator()( const std::string& sString )
			{
				if( *m_pcStrings + 1 < m_cMaxStrings )
				{
					strcpy( m_aszStrings[(*m_pcStrings)++], sString.c_str( ));
				}
			}

			char**	m_aszStrings;
			uint32*	m_pcStrings;
			uint32	m_cMaxStrings;
		};

		StringArray	m_Strings;			// +0x00 sorted
		StringArray	m_FirstStrings;		// +0x0c not sorted
		StringArray	m_FirstNames;		// +0x18 the strings that go first
		bool		m_bSorted;			// +0x24
		uint32		m_cMaxStrings;		// +0x28
		uint32		m_cMaxStringLength;	// +0x2c
};

#endif // __SORTEDSTRINGLIST_H__

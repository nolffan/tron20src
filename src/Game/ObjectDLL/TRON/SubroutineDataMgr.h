// ----------------------------------------------------------------------- //
//
// MODULE  : SubroutineDataMgr.h
//
// PURPOSE : Tron's table of subroutine chances, by subroutine level and
//			 difficulty
//
// RETAIL: Tron 2.0's CSubroutineDataMgr (RTTI; vtable 0x10294f78, Init at
// 0x100cf5d0, g_pSubroutineDataMgr at 0x10376da0), not in any source
// release. Only what the archive bin and the core dump use is declared
// here: the class is not written yet. Its Init sizes a table of
// m_nNumA * m_nNumLevels * m_nNumDifficulties entries (+0x2c..+0x38) of
// two floats (the vector at +0xfc). Member and function names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SUBROUTINEDATAMGR_H__
#define __SUBROUTINEDATAMGR_H__

#include <vector>

class CSubroutineDataMgr;
extern CSubroutineDataMgr* g_pSubroutineDataMgr;

// Which entry of the table (the names are ours)
struct SubroutineDataKey
{
	int		nTable;
	int		nLevel;
	int		nDifficulty;
};

// An entry of the table (the names are ours)
struct SubroutineChance
{
	float	fChanceA;		// The core dump's first virus scan roll
	float	fChanceB;		// The core dump's pickup roll
};

typedef std::vector<SubroutineChance> SubroutineChanceList;

class CSubroutineDataMgr
{
	public :

		virtual ~CSubroutineDataMgr();

		// RETAIL: inline (emitted at 0x1000f9d0): the entry's index, 0 when
		// out of the table
		int GetIndex(const SubroutineDataKey& key) const
		{
			int nIndex = ((key.nTable * m_nNumLevels) + key.nLevel) * m_nNumDifficulties + key.nDifficulty;
			if (nIndex < 0 || nIndex >= m_nNumEntries)
				return 0;
			return nIndex;
		}

		const SubroutineChance& GetChance(int nIndex) const { return m_lstChances.at(nIndex); }

	protected :

		uint8					m_aTronUnknown04[0x28];
		int						m_nNumTables;			// +0x2c
		int						m_nNumLevels;			// +0x30
		int						m_nNumDifficulties;		// +0x34
		int						m_nNumEntries;			// +0x38
		uint8					m_aTronUnknown3c[0xc0];

	public :

		SubroutineChanceList	m_lstChances;			// +0xfc
};

#endif // __SUBROUTINEDATAMGR_H__

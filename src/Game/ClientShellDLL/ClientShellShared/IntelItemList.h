// ----------------------------------------------------------------------- //
//
// MODULE  : IntelItemList.h
//
// PURPOSE : Definition of list of collected intel items
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTEL_LIST_H
#define __INTEL_LIST_H

#pragma warning( disable : 4786 )
#include <vector>

#include "ltbasedefs.h"

// RETAIL: Tron's item (0x8c bytes; debug_new's constructor, retail
// CShell.dll 0x100acb40; CIntelItemList::Save/Load, 0x1013dc60/0x1013dd40;
// CTronPlayerStats::UpdateIntel, 0x100aaac0). It holds build notes, help
// files and e-mails. The names after nPopupId are ours
struct INTEL_ITEM
{
	INTEL_ITEM() {nTextId = 0; nType = 0; nPopupId = 0; bIsIntel = false; bRead = false; nMissionNum = 0; nLevelNum = 0; szName[0] = 0;}

	uint32	nTextId;
	uint8	nType;			// +0x4: 1 is a build note (CMenuBuildNotes)
	uint8	nPopupId;		// +0x5
	bool	bIsIntel;		// +0x6
	bool	bRead;			// +0x7: CMenuHelp sets it when it shows the item
	uint8	nMissionNum;	// +0x8
	uint8	nLevelNum;		// +0x9
	char	szName[128];	// +0xa

};
typedef std::vector<INTEL_ITEM*> IntelArray;



class CIntelItemList
{
public:
	CIntelItemList(uint8 baseSize = 25) {	m_IntelArray.reserve(baseSize);}
	~CIntelItemList() { Clear(); }


	void Clear();

	LTBOOL IsValid(uint16 nIndex) {return (nIndex < m_IntelArray.size());}

	uint16 GetIndex(uint32 id);

	LTBOOL Add(INTEL_ITEM *pItem);
	LTBOOL Remove(uint32 id);

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	uint16		GetCount() {return m_IntelArray.size();}
	INTEL_ITEM*	Get(uint16 nIndex);

private:
	IntelArray::iterator Find(uint32 id);
	IntelArray m_IntelArray;

};

#endif
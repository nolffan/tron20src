// ----------------------------------------------------------------------- //
//
// MODULE  : IONodeMgr.h
//
// PURPOSE : The pings: the characters (I/O nodes) that call the player,
//           read from a bute file, and the pings waiting to be answered
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x1003f590..0x1003feeb, vtable 0x101e7288). The class name is retail's
// (its RTTI); the struct and member names are ours. The player stats keep
// one (TRONPlayerStats.h: the 0x1138-byte object); CHUDPings shows its
// pings.
//
// ----------------------------------------------------------------------- //

#ifndef __IONODE_MGR_H__
#define __IONODE_MGR_H__

#include "GameButeMgr.h"
#include "iltdrawprim.h"

// RETAIL: a bute file's Character<n> (0x14 bytes)
struct IONODECHARACTER
{
	char *		szName;			// +0x00 Name
	int			nNameId;		// +0x04 NameID
	char *		szPortrait;		// +0x08 Portrait
	uint8		bIsCharacter;	// +0x0c IsCharacter: announced in the chat
	LTFLOAT		fStayTime;		// +0x10 StayTime
};

// RETAIL: a ping (0x18 bytes)
struct IONODEPING
{
	char *		szName;			// +0x00 its character's
	char *		szDisplayName;	// +0x04 its character's NameID string
	uint32		nTronUnknown8;	// +0x08
	HTEXTURE	hPortrait;		// +0x0c
	LTFLOAT		fStayTime;		// +0x10
	bool		bNew;			// +0x14
};

class CIONodeMgr : public CGameButeMgr
{
public:
	CIONodeMgr();									// RETAIL: 0x1003f590
	virtual ~CIONodeMgr() { Term(); }				// RETAIL: 0x1003f660

	LTBOOL		Init(const char* szAttributeFile);	// RETAIL: 0x1003f760

	bool		AddPing(const char* szName);		// RETAIL: 0x1003f9d0
	void		ClearPings();						// RETAIL: 0x1003fc70
	bool		RemovePing(const char* szName);		// RETAIL: 0x1003fd00
	IONODEPING*	GetPing(int nIndex);				// RETAIL: 0x1003fdd0
	int			GetNumPings() { return m_Pings.size(); }

	void		Save(ILTMessage_Write* pMsg);		// RETAIL: 0x1003fe00
	void		Load(ILTMessage_Read* pMsg);		// RETAIL: 0x1003fe70

private:
	typedef std::vector<IONODECHARACTER*> CharacterArray;
	typedef std::vector<IONODEPING*> PingArray;

	CharacterArray	m_Characters;		// +0x111c
	PingArray		m_Pings;			// +0x1128
	LTFLOAT			m_fNextPingSound;	// +0x1134: no new ping sound before
};

#endif // __IONODE_MGR_H__

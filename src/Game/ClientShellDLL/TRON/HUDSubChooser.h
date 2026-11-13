// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSubChooser.h
//
// PURPOSE : HUDItem showing the utility subroutines the player can use,
//           each an icon on a background (another for the bad ones)
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x1003b200..0x1003be11, its vtable 0x101e7194). The class name is
// retail's (its RTTI); the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SUBCHOOSER_H
#define __HUD_SUBCHOOSER_H

#include "HUDItem.h"

class CUIFormattedPolyString;
struct TronSubroutine;

#define HUDSUBCHOOSER_MAX	14

class CHUDSubChooser : public CHUDItem
{
public:
	CHUDSubChooser();
	~CHUDSubChooser();

	LTBOOL		Init();
	void		Term();

	// RETAIL: the commonest slot 3 (retail 0x10023440)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
				return true;
		}
		return false;
	}

	void		Render();
	void		Update();

private:
	void		Arrange();

	bool					m_bShow;							// +0x10
	LTFLOAT					m_fScale;							// +0x14
	uint32					m_nScreenWidth;						// +0x18
	LT_POLYFT4				m_Icon[HUDSUBCHOOSER_MAX];			// +0x1c
	LT_POLYFT4				m_Back[HUDSUBCHOOSER_MAX];			// +0x4b4
	LT_POLYFT4				m_BadBack[HUDSUBCHOOSER_MAX];		// +0x94c
	HTEXTURE				m_hBackTex;							// +0xde4
	HTEXTURE				m_hBadBackTex;						// +0xde8
	HTEXTURE				m_hIconTex[HUDSUBCHOOSER_MAX];		// +0xdec
	TronSubroutine*			m_pSubs[HUDSUBCHOOSER_MAX];			// +0xe24
	int						m_nNumSubs;							// +0xe5c
	CUIFormattedPolyString*	m_pText[HUDSUBCHOOSER_MAX];			// +0xe60
	bool					m_bAvailable[HUDSUBCHOOSER_MAX];	// +0xe98
	bool					m_bBad[HUDSUBCHOOSER_MAX];			// +0xea6
	int						m_nTronUnknownEB4;					// +0xeb4 (cleared only)
	int						m_nFontSize;						// +0xeb8
	uint32					m_nTextColor;						// +0xebc
	LTIntPt					m_BoxSize;							// +0xec0
	LTIntPt					m_CursorSize;						// +0xec8
	int						m_nSpacing;							// +0xed0
};

#endif

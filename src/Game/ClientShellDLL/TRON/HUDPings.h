// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPings.h
//
// PURPOSE : HUDItem listing the pings: the characters calling the player,
//           each an icon and a name that blink, then fade
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x100310c0..0x10031a9e, its vtable 0x101e7018). The class name is
// retail's (its RTTI); the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PINGS_H
#define __HUD_PINGS_H

#include "HUDItem.h"

class CUIFormattedPolyString;

#define HUDPINGS_MAX	8

// RETAIL: one shown ping, 0x7c bytes (the name is ours)
struct HUDPING
{
	HTEXTURE				hIcon;			// +0x00: the caller's portrait (none: no ping)
	LT_POLYFT4				Poly;			// +0x04
	CUIFormattedPolyString*	pName;			// +0x58
	char					szName[16];		// +0x5c
	int						nBlinkCount;	// +0x6c
	LTFLOAT					fBlinkTime;		// +0x70
	bool					bExpires;		// +0x74
	LTFLOAT					fLifeTime;		// +0x78
};

class CHUDPings : public CHUDItem
{
public:
	CHUDPings();

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

	void		UpdateLayout();

private:
	void		Arrange();

	LTFLOAT		m_fScale;						// +0x10
	int			m_nNumPings;					// +0x14
	HUDPING		m_Pings[HUDPINGS_MAX];			// +0x18
	LTIntPt		m_IconSize;						// +0x3f8
	LTIntPt		m_StartPos;						// +0x400
	int			m_nSpacing;						// +0x408
	int			m_nFontNum;						// +0x40c
	int			m_nFontSize;					// +0x410
	int			m_nBlinkCount;					// +0x414
	LTFLOAT		m_fBlinkTime;					// +0x418
};

#endif

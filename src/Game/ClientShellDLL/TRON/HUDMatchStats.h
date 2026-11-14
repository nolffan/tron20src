// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMatchStats.h
//
// PURPOSE : HUDItem showing the local player's disc match
//
// RETAIL: Tron 2.0's, retail-only: retail CShell.dll's CHUDMatchStats
// (vtable 0x101e6fb0; its TU 0x1002fe20..0x10030ac0), a member of
// CTronHUDMgr. The two teams of the disc match the local player is in,
// the match's time left, and a circle per round each team won. Retail has
// no CHUDWeapons: this item stands where Build 69's was. Member names are
// ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_MATCHSTATS_H
#define __HUD_MATCHSTATS_H

#include "HUDItem.h"

class CHUDMatchStats : public CHUDItem
{
public:
	CHUDMatchStats();							// RETAIL: 0x1002fe90

	LTBOOL		Init();							// RETAIL: 0x1002ff40
	void		Term();							// RETAIL: 0x10030040

	// RETAIL: folded with CHUDVersion's (0x10023440)
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

	void		Render();						// RETAIL: 0x100300b0
	void		Update();						// RETAIL: 0x10030380
	void		UpdateLayout();					// RETAIL: 0x10030810

	enum Constants
	{
		kMaxRounds = 3,
	};

protected:

	uint8		m_nRounds;						// +0x10 rounds to win (0 when not in a match)
	uint8		m_nWins[2];						// +0x11 per team
	uint32		m_nTronUnknown14;				// +0x14 (cleared, not used)
	float		m_fScale;						// +0x18
	int			m_nCenterX;						// +0x1c
	int			m_nTronUnknown20;				// +0x20 (set, not used)
	int			m_nTop;							// +0x24
	HTEXTURE	m_hEmpty;						// +0x28
	HTEXTURE	m_hFull;						// +0x2c
	LT_POLYFT4	m_Circles[kMaxRounds * 2];		// +0x30 a round's circle per team
	CUIFormattedPolyString*	m_pTeam[2];			// +0x228
	CUIFormattedPolyString*	m_pTime;			// +0x230
};

#endif

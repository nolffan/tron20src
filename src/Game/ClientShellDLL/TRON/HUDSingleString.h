// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSingleString.h
//
// PURPOSE : HUDItem showing the one string the server asks for
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003ad60..
// 0x1003b1d0, vtable 0x101e7160). The class name is retail's (its RTTI);
// the member names are ours. In multiplayer games it centres the string
// whose id the interface manager holds (CTronInterfaceMgr +0x28b18) at the
// layout's "SingleString" position.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SINGLESTRING_H
#define __HUD_SINGLESTRING_H

#include "HUDItem.h"

class CHUDSingleString : public CHUDItem
{
public:
	CHUDSingleString();

	void		Term();

	// RETAIL: every game type (0x1003ade0)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeCooperative:
			case eGameTypeDeathmatch:
			case eGameTypeDiscArena:
			case eGameTypeDiscTournament:
			case eGameTypeMultiplayerLightcycles:
				return true;
		}
		return false;
	}

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	int				m_nStringID;	// +0x10
	float			m_fScale;		// +0x14
	CUIPolyString*	m_pString;		// +0x18
	LTIntPt			m_BasePos;		// +0x1c
	LTVector		m_vColor;		// +0x24
};

#endif

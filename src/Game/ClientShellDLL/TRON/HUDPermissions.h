// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPermissions.h
//
// PURPOSE : HUDItem to display permission sets
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PERMISSIONS_H
#define __HUD_PERMISSIONS_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
class CHUDPermissions : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
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

	CHUDPermissions();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	// RETAIL: Tron's layout (retail 0x10030b30..0x10031086): the pieces
	// the player has flash when new; build 69's missing pieces are gone
	float		m_fScale;				// +0x10
	uint8		m_nPermissions;			// +0x14
	uint8		m_nNewPermissions;		// +0x15: those flashing
	uint8		m_nFlashCount;			// +0x16
	float		m_fFlashTime;			// +0x18

	LT_POLYFT4	m_Base;					// +0x1c
	LT_POLYFT4	m_Have[8];				// +0x70
	HTEXTURE	m_hBaseTex;				// +0x310
	HTEXTURE	m_hHaveTex[8];			// +0x314
};

#endif
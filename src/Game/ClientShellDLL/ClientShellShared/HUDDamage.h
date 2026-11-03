// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamage.h
//
// PURPOSE : HUDItem to display player damage icons
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_DAMAGE_H
#define __HUD_DAMAGE_H

#include "HUDItem.h"


class CHUDDamage : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDDamage();
	~CHUDDamage();

    LTBOOL      Init();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
    LTIntPt		m_BasePos;
	uint16 		m_nIconHt;

	LTBOOL		m_bDraw;


	LTPoly_GT4	*m_Poly;
	HTEXTURE	*m_hIcon;			//  icon
	
};

#endif
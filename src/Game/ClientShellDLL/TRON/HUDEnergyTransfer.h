// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEnergyTransfer.h
//
// PURPOSE : HUDItem to display an energy transfer in progress
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_ENERGYTRANSFER_H
#define __HUD_ENERGYTRANSFER_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD energy transfer display
//**
//** RETAIL: Tron 2.0's, written from retail CShell.dll (0x10029350..
//** 0x10029910, vtable 0x101e6dd0): a background meter and up to three
//** triangles filling it as the transfer goes on. Build 69's arc version
//** is gone. The member names are ours.
//******************************************************************************************
class CHUDEnergyTransfer : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDEnergyTransfer();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	// RETAIL: 0x10029610, 0x100296d0
	void		BuildFill(LT_POLYFT3* pPoly, float fStartAngle, float fEndAngle);
	void		GetSquarePoint(float fAngle, float* pX, float* pY);

	float		m_fScale;		// +0x10
	float		m_fCenterX;		// +0x14
	float		m_fCenterY;		// +0x18
	bool		m_bShow;		// +0x1c
	HTEXTURE	m_hBack;		// +0x20
	HTEXTURE	m_hFill;		// +0x24
	LT_POLYFT4	m_Back;			// +0x28
	LT_POLYFT3	m_Fill[3];		// +0x7c
	int			m_nFill;		// +0x13c
};

#endif
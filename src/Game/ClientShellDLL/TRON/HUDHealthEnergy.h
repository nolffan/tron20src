// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealthEnergy.h
//
// PURPOSE : HUDItem to display countdown timer
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_HEALTHENERGY_H
#define __HUD_HEALTHENERGY_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD countdown timer display
//******************************************************************************************
class CHUDHealthEnergy : public CHUDItem
{
public:
	// RETAIL: eGameTypeSingle, Cooperative, Deathmatch and the disc games (retail's commonest slot 3)
	virtual bool	IsUsedInGameType(GameType eGameType) { return eGameTypeSingle == eGameType || (eGameType > eGameTypeSingleLightCycleCustom && eGameType <= eGameTypeDiscTournament); }

	CHUDHealthEnergy();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	// RETAIL: 0x1002b390 (folded with CHUDProgress's): the texture rect of
	// a 256x256 texture
	void		SetUVWH(LT_POLYFT4 * pPrim, LTRect rect);

	// RETAIL: Tron's layout (constructor 0x10029980, 0x460 bytes). The names
	// are ours.
	float		m_fScale;				// +0x10

	// local copies, to see if something has changed
	uint32		m_nHealth;				// +0x14
	uint32		m_nMaxHealth;			// +0x18
	float		m_fEnergy;				// +0x1c
	float		m_fMaxEnergy;			// +0x20
	float		m_fReservedEnergy;		// +0x24 (the subroutines' energy)

	uint32		m_nHealthWidth;			// +0x28: the bars at full
	uint32		m_nEnergyWidth;			// +0x2c
	uint32		m_nBarHeight;			// +0x30

	bool		m_bHealthBlink;			// +0x34
	float		m_fHealthBlinkTime;		// +0x38
	float		m_fLayoutHealthBlinkTime;	// +0x3c

	HTEXTURE	m_hHUDTex;				// +0x40
	HTEXTURE	m_hEnergyTex;			// +0x44
	HTEXTURE	m_hPlasmaTex;			// +0x48

	LT_POLYFT4	m_Energy;				// +0x4c
	LT_POLYFT4	m_HealthBG;				// +0xa0
	LT_POLYFT4	m_HealthBar;			// +0xf4
	LT_POLYFT4	m_ReservedBar;			// +0x148
	LT_POLYFT4	m_EnergyBar;			// +0x19c
	LT_POLYFT4	m_HealthBlinkBar;		// +0x1f0
	LT_POLYFT4	m_EnergyCap;			// +0x244
	LT_POLYFT4	m_EnergyTransfer;		// +0x298
	LT_POLYFT4	m_Plasma[4];			// +0x2ec

	float		m_fPlasmaTime;			// +0x43c
	CUIPolyString * m_pHealthStr;		// +0x440
	CUIPolyString * m_pEnergyStr;		// +0x444
	LTFloatPt	m_HealthTextPos;		// +0x448
	LTFloatPt	m_EnergyTextPos;		// +0x450
	float		m_fFlickerTime;			// +0x458
	bool		m_bFlicker;				// +0x45c
};

#endif
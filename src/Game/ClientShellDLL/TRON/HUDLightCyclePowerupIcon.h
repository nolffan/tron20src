// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCyclePowerupIcon.h
//
// PURPOSE : HUDItem showing the icon of the light cycle's power-up
//
// RETAIL: Tron 2.0's, in no source release; written from retail CShell.dll
// (0x1002dae0..0x1002dea2, its vtable 0x101e6f08). The class name is
// retail's (its RTTI); the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_LIGHTCYCLE_POWERUPICON_H
#define __HUD_LIGHTCYCLE_POWERUPICON_H

#include "HUDItem.h"

class CHUDLightCyclePowerupIcon : public CHUDItem
{
public:
	CHUDLightCyclePowerupIcon();

	LTBOOL		Init();

	// RETAIL: the single player and light cycle games (retail 0x1002ea20,
	// folded with the roster's and the speedometer's)
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingle:
			case eGameTypeSingleLightCycleCircuit:
			case eGameTypeSingleLightCycleCustom:
			case eGameTypeMultiplayerLightcycles:
				return true;
		}
		return false;
	}

	void		Render();
	void		Update();

	void		UpdateLayout();

private:
	void		UpdatePowerup();

	bool			m_bShow;	// +0x10
	HTEXTURE		m_hIcon;	// +0x14
	LT_POLYFT4		m_Poly;		// +0x18
};

#endif

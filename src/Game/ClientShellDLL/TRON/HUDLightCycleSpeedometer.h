// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleSpeedometer.h
//
// PURPOSE : HUDItem showing the speed of the player's light cycle
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002f770..
// 0x1002fe15; its vtable 0x101e6f7c). The class name is retail's (RTTI);
// the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_LIGHTCYCLE_SPEEDOMETER_H
#define __HUD_LIGHTCYCLE_SPEEDOMETER_H

#include "HUDItem.h"
#include "TexturedFrame.h"

class CHUDLightCycleSpeedometer : public CHUDItem
{
public:
	CHUDLightCycleSpeedometer();
	virtual ~CHUDLightCycleSpeedometer() {}

	LTBOOL		Init();

	// RETAIL: the light cycle games and single player (retail 0x1002ea20,
	// CHUDLightCycleRoster's)
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
	void		UpdateText();

	CTexturedFrame			m_Frame;			// +0x10
	CUIFormattedPolyString*	m_pText;			// +0x428
	bool					m_bTronUnknown42C;	// +0x42c: cleared only
};

#endif

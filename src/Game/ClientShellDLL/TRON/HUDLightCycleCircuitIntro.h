// ----------------------------------------------------------------------- //
//
// MODULE  : HUDLightCycleCircuitIntro.h
//
// PURPOSE : HUDItem showing a light cycle circuit's name, speed, trail
//           length, wave and lives when a wave starts
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002c180..
// 0x1002d2a0; its vtable 0x101e6e98). The class name is retail's (RTTI);
// the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_LIGHTCYCLE_CIRCUIT_INTRO_H
#define __HUD_LIGHTCYCLE_CIRCUIT_INTRO_H

#include "HUDItem.h"
#include "TexturedFrame.h"

class CHUDLightCycleCircuitIntro : public CHUDItem
{
public:
	CHUDLightCycleCircuitIntro();
	virtual ~CHUDLightCycleCircuitIntro() {}

	LTBOOL		Init();
	void		Term();

	// RETAIL: the single player light cycle circuit only (retail 0x1002d540,
	// CHUDLightCycleLives')
	virtual bool	IsUsedInGameType(GameType eGameType)
	{
		switch (eGameType)
		{
			case eGameTypeSingleLightCycleCircuit:
				return true;
		}
		return false;
	}

	void		Render();
	void		Update();

	void		UpdateLayout();

	// RETAIL: 0x1002cdf0: show it for the layout's time from now
	// (CTronPlayerMgr's 0x100a7030)
	void		Start();

private:
	void		TermStrings();
	void		DestroyString(CUIFormattedPolyString** ppStr);
	void		UpdateText();

	float					m_fStartTime;		// +0x10
	float					m_fDuration;		// +0x14
	float					m_fPauseTime;		// +0x18: not set by retail's code
	bool					m_bStarted;			// +0x1c
	CTexturedFrame			m_Frame;			// +0x20
	CUIFormattedPolyString*	m_pName;			// +0x438
	CUIFormattedPolyString*	m_pSpeedLabel;		// +0x43c
	CUIFormattedPolyString*	m_pTrailLabel;		// +0x440
	CUIFormattedPolyString*	m_pWaveLabel;		// +0x444
	CUIFormattedPolyString*	m_pLivesLabel;		// +0x448
	CUIFormattedPolyString*	m_pSpeed;			// +0x44c
	CUIFormattedPolyString*	m_pTrail;			// +0x450
	CUIFormattedPolyString*	m_pWave;			// +0x454
	CUIFormattedPolyString*	m_pLives;			// +0x458
};

extern CHUDLightCycleCircuitIntro* g_pLightCycleCircuitIntro;

#endif

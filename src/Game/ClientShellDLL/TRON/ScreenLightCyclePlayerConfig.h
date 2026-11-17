// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCyclePlayerConfig.h
//
// PURPOSE : Tron's single player light cycle screen: the player's cycle
//			 (its shade and its style), its model and its statistics
//
// RETAIL: Tron's (RTTI CScreenLightCyclePlayerConfig, vtable 0x101e7f30,
// 40 slots, sizeof 0xb04; retail CShell.dll 0x1006bf80..0x1006de20).
// Written from the retail code: the member and function names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENLIGHTCYCLEPLAYERCONFIG_H__
#define __SCREENLIGHTCYCLEPLAYERCONFIG_H__

#include "BaseScreenLightCycle.h"

class CLTGUIButton;
class CLTGUISlider;

class CScreenLightCyclePlayerConfig : public CBaseScreenLightCycle
{
public:
	CScreenLightCyclePlayerConfig();
	virtual ~CScreenLightCyclePlayerConfig();

	// Build the screen
	virtual LTBOOL	Build();
	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual void	OnFocus(LTBOOL bFocus);
	virtual void	ScreenDimsChanged();

	virtual void	LoadSettings();
	virtual void	SaveSettings();
	virtual CLTGUITextCtrl*	GetTab() { return m_pPlayerConfigTab; }

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Shows the selected cycle: its names, its model and its statistics
	// (retail 0x1006cd10)
	void	UpdateCycle();

	// The smallest and the largest value of each statistic over all the
	// cycles (retail 0x1006dad0)
	void	UpdateStatRanges();

	uint8			m_nShade;			// +0xa28
	CLTGUITextCtrl*	m_pShadeText;		// +0xa2c
	CLTGUIButton*	m_pShadeLeft;		// +0xa30
	CLTGUIButton*	m_pShadeRight;		// +0xa34
	int				m_nStyle;			// +0xa38: 0 regular, 1 super
	CLTGUITextCtrl*	m_pStyleText;		// +0xa3c
	CLTGUIButton*	m_pStyleLeft;		// +0xa40
	CLTGUIButton*	m_pStyleRight;		// +0xa44
	CLTGUITextCtrl*	m_pLabel;			// +0xa48
	CLTGUITextCtrl*	m_pStatsLabel;		// +0xa4c
	uint32			m_xTronUnknownA50;	// +0xa50: only the constructor clears it
	HTEXTURE		m_hBackground;		// +0xa54
	LT_POLYFT4		m_BackgroundPoly;	// +0xa58
	LTFLOAT			m_fRotation;		// +0xaac: the model's angle (degrees)
	LTFLOAT			m_fLastTime;		// +0xab0

	// +0xab4: each statistic's name and bar
	CLTGUITextCtrl*	m_pTurboSpeedText;	// +0xab4
	CLTGUISlider*	m_pTurboSpeed;		// +0xab8
	CLTGUITextCtrl*	m_pMaxSpeedText;	// +0xabc
	CLTGUISlider*	m_pMaxSpeed;		// +0xac0
	CLTGUITextCtrl*	m_pMinSpeedText;	// +0xac4
	CLTGUISlider*	m_pMinSpeed;		// +0xac8
	CLTGUITextCtrl*	m_pTurboAccelText;	// +0xacc
	CLTGUISlider*	m_pTurboAccel;		// +0xad0
	CLTGUITextCtrl*	m_pTurnLossText;	// +0xad4
	CLTGUISlider*	m_pTurnLoss;		// +0xad8

	// +0xadc: each statistic's range
	LTFLOAT			m_fMinTurboSpeed;	// +0xadc
	LTFLOAT			m_fMaxTurboSpeed;	// +0xae0
	LTFLOAT			m_fMinMaxSpeed;		// +0xae4
	LTFLOAT			m_fMaxMaxSpeed;		// +0xae8
	LTFLOAT			m_fMinMinSpeed;		// +0xaec
	LTFLOAT			m_fMaxMinSpeed;		// +0xaf0
	LTFLOAT			m_fMinTurboAccel;	// +0xaf4
	LTFLOAT			m_fMaxTurboAccel;	// +0xaf8
	LTFLOAT			m_fMinTurnLoss;		// +0xafc
	LTFLOAT			m_fMaxTurnLoss;		// +0xb00
};

#endif // __SCREENLIGHTCYCLEPLAYERCONFIG_H__

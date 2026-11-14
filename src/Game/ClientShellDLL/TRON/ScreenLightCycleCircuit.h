// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleCircuit.h
//
// PURPOSE : Tron's light cycle circuit screen: the circuits, their lives,
//			 waves, cycle restrictions and image, and launching one
//
// RETAIL: Tron's (RTTI CScreenLightCycleCircuit, vtable 0x101e7d44,
// sizeof 0xac8; retail CShell.dll 0x10065b10..0x10068420). Written from the
// retail code: the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENLIGHTCYCLECIRCUIT_H__
#define __SCREENLIGHTCYCLECIRCUIT_H__

#include "BaseScreenLightCycle.h"

class CircuitButeMgr;
struct Circuit;

// RETAIL: the launch command of the light cycle screens (ours: 0x2c)
const uint32 kCmdLCLaunch = 0x2C;

class CScreenLightCycleCircuit : public CBaseScreenLightCycle
{
public:
	CScreenLightCycleCircuit();

	// Builds the screen
	virtual LTBOOL	Build();

	virtual void	ScreenDimsChanged();
	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual void	OnSelectionChange();
	virtual void	OnFocus(LTBOOL bFocus);
	virtual uint16	SetSelection(uint16 select, LTBOOL bFindSelectable = LTFALSE);

	virtual void	LoadSettings();
	virtual void	SaveSettings();
	virtual CLTGUITextCtrl*	GetTab() { return m_pCircuitTab; }
	virtual LTRect	GetBodyRect();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual LTBOOL	OnUp();
	virtual LTBOOL	OnDown();

	// Shows a circuit's lives, waves, restrictions and image (retail 0x10066640)
	void	SelectCircuit(uint32 nCircuit);
	// Starts the selected circuit (retail 0x100669a0)
	void	LaunchCircuit();
	// Fills the list with the circuits (retail 0x10066d90)
	LTBOOL	FillCircuitList();

	// RETAIL: 0x10067650: the circuits (it does not use the screen)
	CircuitButeMgr*	GetCircuitMgr();
	// RETAIL: 0x10067660: always false
	bool	IsCircuitLocked(const Circuit *pCircuit);
	// Whether the profile has completed a circuit (retail 0x10067670)
	bool	IsCircuitCompleted(const Circuit *pCircuit);
	// Whether the profile's cycle may race the selected circuit (retail 0x100676e0)
	bool	IsCycleAllowed();
	// Says the profile's cycle may not race it (retail 0x10067870)
	void	ShowCycleNotAllowed();

	uint32				m_nTronUnknownA28;		// +0xa28: not used by the code we know
	CLTGUIListCtrl*		m_pCircuitList;			// +0xa2c
	CLTGUITextCtrl*		m_pLives;				// +0xa30
	CLTGUITextCtrl*		m_pWaves;				// +0xa34
	CLTGUIButton*		m_pLaunch;				// +0xa38
	CLTGUITextCtrl*		m_pSuperRestriction;	// +0xa3c
	CLTGUITextCtrl*		m_pRegularRestriction;	// +0xa40
	StringArray			m_CircuitNames;			// +0xa44
	int					m_nListWidth;			// +0xa50
	HTEXTURE			m_hImage;				// +0xa54
	LT_POLYFT4			m_ImagePoly;			// +0xa58
	uint32				m_nTronUnknownAAC;		// +0xaac: not used by the code we know
	std::string			m_sTronUnknownAB0;		// +0xab0: not used by the code we know
	uint32				m_nTronUnknownABC;		// +0xabc: not used by the code we know
	uint32				m_nCompletedColor;		// +0xac0
	uint32				m_nUncompletedColor;	// +0xac4
};

#endif // __SCREENLIGHTCYCLECIRCUIT_H__

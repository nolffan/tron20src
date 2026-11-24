// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleCustomMaps.h
//
// PURPOSE : Tron's single player light cycle screen: the maps of a custom
//			 game, and launching it
//
// RETAIL: Tron's (RTTI CScreenLightCycleCustomMaps, vtable 0x101e7e8c,
// sizeof 0xa5c; retail CShell.dll 0x10069890..0x1006bf80). Written from the
// retail code, which is CScreenHostMaps' on the light cycle screens' base:
// the member names are CScreenHostMaps', the others ours.
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENLIGHTCYCLECUSTOMMAPS_H__
#define __SCREENLIGHTCYCLECUSTOMMAPS_H__

#include "BaseScreenLightCycle.h"
#include "TRON/CaselessString.h"

class CScreenLightCycleCustomMaps : public CBaseScreenLightCycle
{
public:
	CScreenLightCycleCustomMaps();
	virtual ~CScreenLightCycleCustomMaps();

	// Build the screen
	virtual LTBOOL	Build();
	virtual void	Escape();

	virtual void	OnFocus(LTBOOL bFocus);
	virtual uint16	SetSelection(uint16 select, LTBOOL bFindSelectable = LTFALSE);

	virtual void	LoadSettings();
	virtual void	SaveSettings();
	virtual CLTGUITextCtrl*	GetTab() { return m_pCustomMapTab; }
	virtual LTRect	GetBodyRect();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Whether the profile has unlocked its cycle (retail 0x1006a410), the
	// message when not (0x1006a4b0) and launching the game (0x1006a660)
	bool	IsCycleUnlocked();
	void	ShowCycleLocked();
	void	LaunchGame();

	// The maps of the light cycle folder the profile has unlocked, sorted
	// (retail 0x1006a980, 0x1006adb0)
	LTBOOL	FillAvailList();
	void	FindMaps(const CaselessString &sDir, bool (*pfnIsLocked)(std::string sMap));

	void	UpdateButtons();					// retail 0x1006b040
	void	AddMapToList(uint32 nMap);			// retail 0x1006b0e0

	CLTGUITextCtrl*	m_pAdd;					// +0xa28
	CLTGUITextCtrl*	m_pRemove;				// +0xa2c
	CLTGUITextCtrl*	m_pAddAll;				// +0xa30
	CLTGUITextCtrl*	m_pRemoveAll;			// +0xa34
	CLTGUITextCtrl*	m_pMoveUp;				// +0xa38
	CLTGUITextCtrl*	m_pMoveDown;			// +0xa3c
	CLTGUIListCtrl*	m_pAvailMissions;		// +0xa40
	CLTGUIListCtrl*	m_pSelMissions;			// +0xa44
	StringArray		m_saMaps;				// +0xa48: the folder's maps (no extension)
	int				m_nAvailWidth;			// +0xa54
	int				m_nSelWidth;			// +0xa58
};

#endif // __SCREENLIGHTCYCLECUSTOMMAPS_H__

// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenLightCycle.h
//
// PURPOSE : Base of Tron's single player light cycle screens: the four
//			 tabs (circuit, player config, custom maps, custom game), the
//			 player's name and cycle, and the list of saved game settings
//
// RETAIL: Tron's (RTTI CBaseScreenLightCycle, vtable 0x101e551c, 40 slots,
// sizeof 0xa28; retail CShell.dll 0x10008230..0x1000a6f0). Written from the
// retail code: the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __BASESCREENLIGHTCYCLE_H__
#define __BASESCREENLIGHTCYCLE_H__

#include "BaseScreen.h"
#include "ScreenMgr.h"
#include "LightLine.h"
#include "LTGUIListItemCtrl.h"
#include <map>

struct LIGHTCYCLECOLOR;

// RETAIL: Tron's command IDs (the light cycle screens'; the names are ours)
const uint32 kCmdLCPlayerConfigTab	= 0x435;
const uint32 kCmdLCCircuitTab		= 0x436;
const uint32 kCmdLCCustomGameTab	= 0x437;
const uint32 kCmdLCCustomMapTab		= 0x438;
const uint32 kCmdLCNewSettings		= 0x439;
const uint32 kCmdLCRenameSettings	= 0x43A;
const uint32 kCmdLCDeleteSettings	= 0x43B;
const uint32 kCmdLCSelectSettings	= 0x43C;

// RETAIL: the edit box callbacks of the new and rename commands are
// CBaseScreenHost's (0x10002f40, 0x10002f90: they send CMD_OK with 0x439
// and 0x43a to the current screen); the names are ours
void NewSettingsCallBack(LTBOOL bReturn, void *pData);
void RenameSettingsCallBack(LTBOOL bReturn, void *pData);

class CBaseScreenLightCycle : public CBaseScreen
{
public:
	CBaseScreenLightCycle();
	virtual ~CBaseScreenLightCycle();

	// Builds the screen
	virtual LTBOOL	Build();

	virtual LTBOOL	Render(HSURFACE hDestSurf);

	// This is called when the screen gets or loses focus
	virtual void	OnFocus(LTBOOL bFocus);

	virtual LTBOOL	HandleKeyDown(int key, int rep);

	virtual void	ScreenDimsChanged();

	// RETAIL: Tron's new virtuals (slots 36..39; the names are ours):
	// reads the profile's light cycle settings into the screen, writes them
	// back, the screen's own tab and the rectangle its tab frames
	virtual void	LoadSettings();
	virtual void	SaveSettings();
	virtual CLTGUITextCtrl*	GetTab() = 0;
	virtual LTRect	GetBodyRect();

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual LTBOOL	OnUp();
	virtual LTBOOL	OnDown();
	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// The list of saved settings and its commands (retail 0x10008d10)
	void	CreateSettingsList();

	// Frames the body and the selected tab (retail 0x10008ec0)
	void	SetTabFrame(CLTGUICtrl *pTab);

	// Goes to the next or the previous tab's screen (retail 0x10009930)
	LTBOOL	CycleTab(LTBOOL bNext);

	// The message box results (CMD_OK; retail 0x10009a80)
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	// As CreateTextItem, a list item (retail 0x10009ed0)
	CLTGUIListItemCtrl*	CreateListItem(std::wstring sString, uint32 commandID, int helpID,
							LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	// Whether a tab is selected (retail 0x1000a0f0)
	bool	IsTabSelected();

	// Fills the list with the names of the saved settings (retail 0x1000a170)
	void	UpdateSettingsList();

	// The profile's cycle color, its style and its shade (retail 0x1000a600)
	bool	GetProfileCycleColor(LIGHTCYCLECOLOR **ppColor, uint8 *pnStyle, uint8 *pnShade);

	// Whether the profile has not unlocked a cycle color (retail 0x1000a660)
	bool	IsCycleColorLocked(const char *pszColor);

	CLTGUITextCtrl*		m_pCircuitTab;		// +0x790
	CLTGUITextCtrl*		m_pPlayerConfigTab;	// +0x794
	CLTGUITextCtrl*		m_pCustomGameTab;	// +0x798
	CLTGUITextCtrl*		m_pCustomMapTab;	// +0x79c
	CLTGUITextCtrl*		m_pNewSettings;		// +0x7a0
	CLTGUITextCtrl*		m_pRenameSettings;	// +0x7a4
	CLTGUITextCtrl*		m_pDeleteSettings;	// +0x7a8
	CString				m_sNewName;			// +0x7ac: the new settings' default name
	CLTGUIListCtrl*		m_pSettingsList;	// +0x7b0
	CLTGUITextCtrl*		m_pSettingsTitle;	// +0x7b4
	std::string			m_sPlayerName;		// +0x7b8
	CLTGUIColumnCtrl*	m_pName;			// +0x7c4
	CLTGUIColumnCtrl*	m_pCycleName;		// +0x7c8
	bool				m_bChangingFocus;	// +0x7cc: the list's commands are ignored while set
	LT_POLYF4			m_TabPoly[2];		// +0x7d0: the body and the selected tab
	CLightLine			m_TabLines[8];		// +0x838
	uint32				m_nTabColor;		// +0xa18
	// +0xa1c: each tab's screen: the screens of the previous and the next tab
	std::map<eScreenID, std::pair<int,int> >	m_TabOrder;
};

#endif // __BASESCREENLIGHTCYCLE_H__

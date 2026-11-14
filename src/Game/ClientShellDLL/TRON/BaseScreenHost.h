// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenHost.h
//
// PURPOSE : Base of Tron's host screens: the connection, game and maps
//			 tabs, the list of saved option sets and launching the server
//
// RETAIL: Tron's (RTTI CBaseScreenHost, vtable 0x101e53ac, sizeof 0xa14;
// retail CShell.dll 0x10002eb0..0x10004cd0). Written from the retail code:
// the member and function names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __BASESCREENHOST_H__
#define __BASESCREENHOST_H__

#include "BaseScreen.h"
#include "LightLine.h"

class CLTGUIListItemCtrl;

// RETAIL: Tron's command IDs (the host screens')
const uint32 kCmdHostLaunch			= 0x2C;
const uint32 kCmdHostSelectOptions	= 0x434;
const uint32 kCmdHostConnectionTab	= 0x435;
const uint32 kCmdHostGameTab		= 0x436;
const uint32 kCmdHostMapsTab		= 0x437;
const uint32 kCmdHostNewOptions		= 0x438;
const uint32 kCmdHostRenameOptions	= 0x439;
const uint32 kCmdHostDeleteOptions	= 0x43A;

// RETAIL: the option sets' edit box callbacks (0x10002f40, 0x10002f90:
// CMD_OK with 0x439, 0x43a to the current screen); the light cycle screens
// use them too (CBaseScreenLightCycle::OnCommand)
void RenameOptionsCallBack(LTBOOL bReturn, void *pData);
void DeleteOptionsCallBack(LTBOOL bReturn, void *pData);

class CBaseScreenHost : public CBaseScreen
{
public:
	CBaseScreenHost();
	virtual ~CBaseScreenHost();

	// Builds the screen
	virtual LTBOOL	Build();

	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual bool	UpdateInterfaceSFX();

	// This is called when the screen gets or loses focus
	virtual void	OnFocus(LTBOOL bFocus);

	virtual LTBOOL	HandleKeyDown(int key, int rep);

	// RETAIL: Tron's new virtuals (slots 36..38): the launch confirmation's
	// answer (0x100044c0, launched on the next update), the current option
	// set shown in the controls (0x10003f10; the screens add theirs) and the
	// controls saved into it (pure: each screen's own)
	virtual void	SetLaunchPending(uint32 bLaunch);
	virtual void	LoadOptions();
	virtual void	SaveOptions() = 0;

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// Frames the body and the selected tab (retail 0x100038f0)
	void	SetTabFrame(CLTGUICtrl *pTab);

public:
	// Asks before launching a light cycle game on the internet, then starts
	// the server (retail 0x10004330, 0x10004420)
	void	Launch();
	LTBOOL	StartServer();

protected:
	// The new, renamed or deleted option set (retail 0x100044d0): the edit
	// boxes' and the delete question's answers
	void	HandleEditResult(uint32 dwParam1, uint32 dwParam2);

	// A list item that sends nSelectCommandID when chosen (retail 0x10004880)
	CLTGUIListItemCtrl*	CreateListItem(std::wstring sString, uint32 nSelectCommandID, int helpID,
							LTIntPt pos = kDefaultPos, LTBOOL bFixed = LTFALSE, int nFont = -1);

	// Fills the list with the option sets' names (retail 0x10004aa0)
	void	RebuildList();

	CLTGUITextCtrl*	m_pConnectionTab;	// +0x790
	CLTGUITextCtrl*	m_pGameTab;			// +0x794
	CLTGUITextCtrl*	m_pMapsTab;			// +0x798
	CLTGUIButton*	m_pLaunch;			// +0x79c
	CLTGUITextCtrl*	m_pOptionsName;		// +0x7a0
	CLTGUITextCtrl*	m_pOptionsTitle;	// +0x7a4
	CLTGUITextCtrl*	m_pGameType;		// +0x7a8
	uint32			m_bLaunchPending;	// +0x7ac
	uint32			m_nTabColor;		// +0x7b0
	LT_POLYF4		m_TabPoly[2];		// +0x7b4: the body and the selected tab
	CLightLine		m_TabLines[8];		// +0x81c
	CLTGUITextCtrl*	m_pNewOptions;		// +0x9fc
	CLTGUITextCtrl*	m_pRenameOptions;	// +0xa00
	CLTGUITextCtrl*	m_pDeleteOptions;	// +0xa04
	CLTGUIListCtrl*	m_pOptionsList;		// +0xa08
	CString			m_sNewName;			// +0xa0c: a new set's default name
	bool			m_bFocusing;		// +0xa10: set while OnFocus fills the list
};

#endif // __BASESCREENHOST_H__

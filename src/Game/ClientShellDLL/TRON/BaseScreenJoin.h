// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenJoin.h
//
// PURPOSE : Base of Tron's join screens: the server list and server info
//			 tabs, the GameSpy logo and joining the chosen server
//
// RETAIL: Tron's (RTTI CBaseScreenJoin, vtable 0x101e5478, sizeof 0xa74;
// retail CShell.dll 0x100065c0..0x10008230). Written from the retail code:
// the member names are ours.
//
// ----------------------------------------------------------------------- //

#ifndef __BASESCREENJOIN_H__
#define __BASESCREENJOIN_H__

#include "BaseScreen.h"
#include "LightLine.h"
#include "../../Libs/GameSpy/GameSpySDK/serverbrowsing/sb_serverbrowsing.h"

// RETAIL: Tron's command IDs (the join screens')
const uint32 kCmdJoinServer			= 0x1F;
const uint32 kCmdEditPassword		= 0x22;
const uint32 kCmdServerListTab		= 0x434;
const uint32 kCmdServerInfoTab		= 0x435;

// RETAIL: the GameSpy server browser the join screens share (retail
// 0x10236c88, whether it has been made 0x10236c8c) and the server chosen
// to join (0x10236c94)
extern ServerBrowser	g_hServerBrowser;
extern bool				g_bServerBrowser;
extern SBServer			g_hJoinServer;

class CBaseScreenJoin : public CBaseScreen
{
public:
	CBaseScreenJoin();
	virtual ~CBaseScreenJoin();

	// Builds the screen
	virtual LTBOOL	Build();

	// RETAIL: an override that only calls CBaseScreen's (0x100067c0, emitted
	// with the vtable)
	virtual void	Escape() { CBaseScreen::Escape(); }

	virtual LTBOOL	Render(HSURFACE hDestSurf);
	virtual bool	UpdateInterfaceSFX();

	// This is called when the screen gets or loses focus
	virtual void	OnFocus(LTBOOL bFocus);

	virtual LTBOOL	HandleKeyDown(int key, int rep);

	// RETAIL: Tron's, a new virtual (slot 36, 0x10007970): the message box
	// results (CMD_OK)
	virtual void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

protected:
	virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual LTBOOL	OnLeft();
	virtual LTBOOL	OnRight();

	// Frames the body and the selected tab (retail 0x10006d40)
	void	SetTabFrame(CLTGUICtrl *pTab);

	// Joins g_hJoinServer (retail 0x100072b0), asking first for a light
	// cycle game (0x100065c0's callback) and for the password (0x100073c0)
	void	JoinServer();
public:
	void	CheckPassword();
protected:
	// Connects to g_hJoinServer once the password is known (0x100074e0,
	// 0x10007510)
	void	DoJoin();
	LTBOOL	ConnectToServer();

	// Joins the server at an "ip:port" address (retail 0x100077e0)
	void	JoinAddress(std::string sAddress);
	bool	ParseAddress(const std::string &sAddress, char *pszIP, uint16 *pnPort);
	SBServer FindServer(const char *pszIP, uint16 nPort);

	// RETAIL: 0x100be620 (identical code, folded with CHUDMessage's)
	void	SetRenderState();

	CLTGUITextCtrl*	m_pServerListTab;	// +0x790
	CLTGUITextCtrl*	m_pServerInfoTab;	// +0x794
	LTBOOL			m_bInternet;		// +0x798: (ours) gates the light cycle warning and the update check; set by the server list
	LTBOOL			m_bJoinPending;		// +0x79c
	std::string		m_sPassword;		// +0x7a0
	uint32			m_nTabColor;		// +0x7ac
	LT_POLYF4		m_TabPoly[2];		// +0x7b0: the body and the selected tab
	CLightLine		m_TabLines[8];		// +0x818
	char			m_szSecretKey[12];	// +0x9f8: GameSpy's key for Tron
	LT_POLYFT4		m_GameSpyPoly;		// +0xa04
	HTEXTURE		m_hGameSpyTex;		// +0xa58
	bool			m_bCheckingForUpdate;	// +0xa5c
	float			m_fUpdateTimeout;	// +0xa60
	bool			m_bMandatoryUpdate;	// +0xa64
	std::string		m_sUpdateURL;		// +0xa68
};

#endif // __BASESCREENJOIN_H__

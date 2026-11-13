// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenJoin.cpp
//
// PURPOSE : Base of Tron's join screens: the server list and server info
//			 tabs, the GameSpy logo and joining the chosen server
//
// RETAIL: written from retail CShell.dll 0x100065c0..0x10008230 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScreenJoin.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "GameClientShell.h"
#include "ltquaduvutils.h"
#include "../../Libs/GameSpy/GameSpySDK/ghttp/ghttp.h"

// RETAIL: 0x10236c88, 0x10236c8c, 0x10236c94
ServerBrowser	g_hServerBrowser = LTNULL;
bool			g_bServerBrowser = false;
SBServer		g_hJoinServer = LTNULL;

namespace
{
	// RETAIL: 0x102108f4: the light cycle warning is shown once
	bool	s_bWarnLightCycles = true;

	// RETAIL: the default port of a server address without one (27888)
	const uint16 kDefaultPort = 0x6CF0;

	// RETAIL: 0x100065c0
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		eScreenID eCurrent = g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID();
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(eCurrent);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdEditPassword);
	}

	// RETAIL: 0x10006610: the warning before joining a light cycle game
	void LightCycleWarningCallBack(LTBOOL bReturn, void *pData)
	{
		if (bReturn)
		{
			eScreenID eCurrent = g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID();
			CBaseScreenJoin *pThisScreen = (CBaseScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(eCurrent);
			if (pThisScreen)
			{
				s_bWarnLightCycles = false;
				pThisScreen->CheckPassword();
			}
		}
	}
}

// RETAIL: 0x10006660
CBaseScreenJoin::CBaseScreenJoin()
{
	m_pServerListTab = LTNULL;
	m_pServerInfoTab = LTNULL;
	m_bJoinPending = LTFALSE;
	m_bInternet = LTFALSE;
	m_bCheckingForUpdate = false;
	m_nTabColor = 0x80000080;

	// GameSpy's secret key for Tron 2.0
	m_szSecretKey[0] = 't';
	m_szSecretKey[1] = '9';
	m_szSecretKey[2] = 'D';
	m_szSecretKey[3] = '3';
	m_szSecretKey[4] = 'v';
	m_szSecretKey[5] = 'B';
	m_szSecretKey[6] = '\0';

	m_hGameSpyTex = LTNULL;
}

// RETAIL: 0x100067f0
CBaseScreenJoin::~CBaseScreenJoin()
{
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Term();
	}
}

// RETAIL: 0x10006920
LTBOOL CBaseScreenJoin::Build()
{
	CreateTitle(0x1F);

	LTRect rcTab = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN_SERVERLIST,"TabRect");
	int nTabWidth = (rcTab.right - rcTab.left) / 2;

	LTVector vColor = g_pLayoutMgr->GetScreenCustomVector(SCREEN_ID_HOST,"TabColor");
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;
	uint8 nA = (uint8)(g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_HOST,"TabAlpha") * 255.0f);
	m_nTabColor = SET_ARGB(nA,nR,nG,nB);

	m_pServerListTab = AddTextItem(0xF64,kCmdServerListTab,0,LTIntPt(rcTab.left,rcTab.top));
	m_pServerListTab->SetFixedWidth(nTabWidth);
	m_pServerInfoTab = AddTextItem(0xF65,kCmdServerInfoTab,0,LTIntPt(rcTab.left + nTabWidth,rcTab.top));
	m_pServerInfoTab->SetFixedWidth(nTabWidth);

	// RETAIL: read and not used
	LTIntPt joinPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_SERVERLIST,"JoinPos");
	uint8 nFont = g_pLayoutMgr->GetBackFont();
	uint8 nFontSize = g_pLayoutMgr->GetBackSize();

	LTIntPt launchPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_SERVERLIST,"LaunchPos");
	CLTGUIButton *pLaunch = debug_new(CLTGUIButton);

	char szTex[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_JOIN_SERVERLIST,"LaunchTex",szTex,sizeof(szTex));
	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_JOIN_SERVERLIST,"LaunchTexH",szTex,sizeof(szTex));
	HTEXTURE hHot = g_pInterfaceResMgr->GetTexture(szTex);

	pLaunch->Create(kCmdJoinServer,0x20D,hNormal,hHot);
	pLaunch->SetFont(g_pInterfaceResMgr->GetFont(nFontSize),nFontSize);
	pLaunch->SetText(LoadTempWString(0x1F7),LTTRUE);
	pLaunch->SetBasePos(launchPos);
	pLaunch->SetScale(g_pInterfaceResMgr->GetXRatio());
	pLaunch->SetCommandHandler(this);
	CUIFont *pFont = g_pInterfaceResMgr->GetFont(nFontSize);
	pLaunch->SetFont(pFont,nFontSize);
	pLaunch->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_controlArray.push_back(pLaunch);

	m_hGameSpyTex = g_pInterfaceResMgr->GetTexture("interface\\Menu\\sprtex\\gamespy.dtx");
	g_pDrawPrim->SetRGBA(&m_GameSpyPoly,argbWhite);
	SetupQuadUVs(m_GameSpyPoly,m_hGameSpyTex,0.0f,0.0f,1.0f,1.0f);

	return CBaseScreen::Build();
}

// RETAIL: 0x10006d40: the body's frame, opened where the selected tab is
void CBaseScreenJoin::SetTabFrame(CLTGUICtrl *pTab)
{
	if (!pTab) return;

	LTIntPt tabPos = pTab->GetBasePos();
	float fTabLeft = (float)tabPos.x - 10.0f;
	float fTabTop = (float)tabPos.y - 5.0f;
	float fTabRight = (float)pTab->GetBaseWidth() + fTabLeft;

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	LTRect rcBody = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN_SERVERLIST,"BodyRect");
	float fLeft = (float)rcBody.left * fXRatio;
	float fWidth = (float)(rcBody.right - rcBody.left) * fXRatio;
	float fTop = (float)(rcBody.top - 5) * fYRatio;
	float fHeight = (float)(rcBody.bottom - rcBody.top) * fYRatio;

	g_pDrawPrim->SetXYWH(&m_TabPoly[0],fLeft,fTop,fWidth,fHeight);
	g_pDrawPrim->SetXYWH(&m_TabPoly[1],fTabLeft,fTabTop,fTabRight - fTabLeft,fTop - fTabTop);

	float fBottom = fTop + fHeight;
	m_TabLines[0].Create(fLeft,fTop,fLeft,fBottom);
	float fRight = fLeft + fWidth;
	m_TabLines[1].Create(fLeft,fBottom,fRight,fBottom);
	m_TabLines[2].Create(fRight,fTop,fRight,fBottom);
	m_TabLines[3].Create(fLeft,fTop,fTabLeft,fTop);
	m_TabLines[4].Create(fTabRight,fTop,fRight,fTop);
	m_TabLines[5].Create(fTabLeft,fTabTop + 2.0f,fTabLeft,fTop);
	m_TabLines[6].Create(fTabLeft,fTabTop,fTabRight,fTabTop);
	m_TabLines[7].Create(fTabRight,fTabTop + 2.0f,fTabRight,fTop);

	g_pDrawPrim->SetRGBA(&m_TabPoly[0],m_nTabColor);
	g_pDrawPrim->SetRGBA(&m_TabPoly[1],m_nTabColor);
}

// RETAIL: 0x10006f90
uint32 CBaseScreenJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	case kCmdJoinServer:
		JoinServer();
		break;
	case kCmdServerListTab:
		if (m_nScreenID == SCREEN_ID_SERVERINFO)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_JOIN_SERVERLIST);
			return 1;
		}
		break;
	case kCmdServerInfoTab:
		if (m_nScreenID != SCREEN_ID_SERVERINFO)
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_SERVERINFO);
			return 1;
		}
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x10007020
void CBaseScreenJoin::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (!g_pMissionButeMgr->Init("Attributes\\nomissions.txt"))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.","Attributes\\nomissions.txt");
			return;
		}

		LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_SERVERLIST,"GameSpyPos");
		int nWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_SERVERLIST,"GameSpyWidth");

		uint32 nTexW, nTexH;
		g_pTexInterface->GetTextureDims(m_hGameSpyTex,nTexW,nTexH);

		float fXRatio = g_pInterfaceResMgr->GetXRatio();
		float fYRatio = g_pInterfaceResMgr->GetYRatio();
		float fHeight = (float)nWidth * (float)nTexH / (float)nTexW;

		g_pDrawPrim->SetXYWH(&m_GameSpyPoly,(float)pos.x * fXRatio,(float)pos.y * fYRatio,
			(float)nWidth * fXRatio,fHeight * fYRatio);

		// RETAIL: here retail asks GameSpy for a patch (0x10007d70, with
		// ptCheckForPatchAndTrackUsage; its answer 0x10007c40, 0x10007b40):
		// left out, our GameSpy SDK has no pt module yet
	}

	CBaseScreen::OnFocus(bFocus);
}

// RETAIL: 0x10007150
LTBOOL CBaseScreenJoin::Render(HSURFACE hDestSurf)
{
	if (m_bCheckingForUpdate)
	{
		if (g_pLTClient->GetTime() > m_fUpdateTimeout)
		{
			m_bCheckingForUpdate = false;
			g_pInterfaceMgr->CloseMessageBox(LTTRUE);
		}
		ghttpThink();
	}

	SetRenderState();
	g_pDrawPrim->DrawPrim(m_TabPoly,2);
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Render();
	}

	LTBOOL bRet = CBaseScreen::Render(hDestSurf);

	g_pDrawPrim->SetTexture(m_hGameSpyTex);
	g_pDrawPrim->DrawPrim(&m_GameSpyPoly,1);

	return bRet;
}

// RETAIL: 0x10007210: left and right move between the tabs
LTBOOL CBaseScreenJoin::OnLeft()
{
	CLTGUICtrl *pCtrl = GetControl(m_nSelection);
	if (pCtrl == m_pServerListTab)
		return OnDown();
	if (pCtrl == m_pServerInfoTab)
		return OnUp();
	return CBaseScreen::OnLeft();
}

// RETAIL: 0x10007250
LTBOOL CBaseScreenJoin::OnRight()
{
	CLTGUICtrl *pCtrl = GetControl(m_nSelection);
	if (pCtrl == m_pServerListTab)
		return OnDown();
	if (pCtrl == m_pServerInfoTab)
		return OnUp();
	return CBaseScreen::OnRight();
}

// RETAIL: 0x10007290
bool CBaseScreenJoin::UpdateInterfaceSFX()
{
	if (m_bJoinPending)
		DoJoin();
	return CBaseScreen::UpdateInterfaceSFX();
}

// RETAIL: 0x100072b0
void CBaseScreenJoin::JoinServer()
{
	if (!g_hJoinServer || !SBServerHasBasicKeys(g_hJoinServer))
		return;

	const char *pszMod = SBServerGetStringValue(g_hJoinServer,"modname","");
	if (!pszMod || _stricmp(pszMod,g_pClientMultiplayerMgr->GetModName()) != 0)
		return;

	if (s_bWarnLightCycles && m_bInternet)
	{
		// PORT: retail's GameStringTypeToGameType returns whether it knew the
		// type and passes it out (bool (char const*, GameType*))
		GameType eGameType = GameStringTypeToGameType(SBServerGetStringValue(g_hJoinServer,"gametype",""));
		if (eGameType == eGameTypeMultiplayerLightcycles)
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = LightCycleWarningCallBack;
			g_pInterfaceMgr->ShowMessageBox(0xA03,&mb);
			return;
		}
	}

	CheckPassword();
}

// RETAIL: 0x100073c0
void CBaseScreenJoin::CheckPassword()
{
	if (!g_hJoinServer || !SBServerHasBasicKeys(g_hJoinServer))
		return;

	if (SBServerGetBoolValue(g_hJoinServer,"password",SBFalse) > SBFalse)
	{
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPassCallBack;
		mb.pString = "";
		mb.nMaxChars = 16;
		g_pInterfaceMgr->ShowMessageBox(0x2FA,&mb);
		return;
	}

	m_sPassword = "";
	m_bJoinPending = LTTRUE;
}

// RETAIL: 0x100074e0
void CBaseScreenJoin::DoJoin()
{
	m_bJoinPending = LTFALSE;
	UpdateData(LTTRUE);
	if (!ConnectToServer())
		g_pInterfaceMgr->LoadFailed();
}

// RETAIL: 0x10007510
LTBOOL CBaseScreenJoin::ConnectToServer()
{
	const char *pszIP = SBServerGetPublicAddress(g_hJoinServer);
	uint16 nPort = SBServerGetPublicQueryPort(g_hJoinServer);
	if (strlen(pszIP) > 20 || nPort < 1024 || nPort > 0xBFFF)
		return LTFALSE;

	// PORT: see JoinServer
	GameType eGameType = GameStringTypeToGameType(SBServerGetStringValue(g_hJoinServer,"gametype",""));
	g_pGameClientShell->SetGameType(eGameType);

	char szAddress[48];
	sprintf(szAddress,"%s:%d",pszIP,nPort);

	// PORT: retail's SetupClient takes the address and the password only
	// (0x100e87b0)
	if (!g_pClientMultiplayerMgr->SetupClient(szAddress,LTNULL,m_sPassword.c_str()))
		return LTFALSE;

	if (!g_pMissionMgr->StartGameAsClient())
		return LTFALSE;

	return LTTRUE;
}

// RETAIL: 0x10007600: "ip[:port]"
bool CBaseScreenJoin::ParseAddress(const std::string &sAddress, char *pszIP, uint16 *pnPort)
{
	if (sAddress.length() > 60)
		return false;

	int nPos = sAddress.find(':');
	if (nPos == std::string::npos)
	{
		*pnPort = kDefaultPort;
		strcpy(pszIP,sAddress.c_str());
	}
	else
	{
		strcpy(pszIP,sAddress.substr(0,nPos).c_str());
		*pnPort = (uint16)atoi(sAddress.substr(nPos+1).c_str());
		if (*pnPort < 1024 || *pnPort > 0xBFFF)
			*pnPort = kDefaultPort;
	}

	return true;
}

// RETAIL: 0x10007750
SBServer CBaseScreenJoin::FindServer(const char *pszIP, uint16 nPort)
{
	int nCount = ServerBrowserCount(g_hServerBrowser);
	for (int i = 0; i < nCount; i++)
	{
		SBServer hServer = ServerBrowserGetServer(g_hServerBrowser,i);
		if (hServer && SBServerHasBasicKeys(hServer))
		{
			const char *pszAddr = SBServerGetPublicAddress(hServer);
			uint16 nQueryPort = SBServerGetPublicQueryPort(hServer);
			if (!_stricmp(pszAddr,pszIP) && nQueryPort == nPort)
				return hServer;
		}
	}
	return LTNULL;
}

// RETAIL: 0x100077e0
void CBaseScreenJoin::JoinAddress(std::string sAddress)
{
	char szIP[64];
	uint16 nPort;

	g_hJoinServer = LTNULL;
	if (ParseAddress(sAddress,szIP,&nPort))
	{
		g_hJoinServer = FindServer(szIP,nPort);
		if (!g_hJoinServer)
		{
			if (ServerBrowserAuxUpdateIP(g_hServerBrowser,szIP,nPort,SBFalse,SBFalse,SBFalse) == sbe_noerror)
				g_hJoinServer = FindServer(szIP,nPort);
		}
	}

	if (!g_hJoinServer)
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox(0x14C,&mb);
		return;
	}

	JoinServer();
}

// RETAIL: 0x10007970: the password typed into the message box
void CBaseScreenJoin::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	if (dwParam2 == kCmdEditPassword)
	{
		std::wstring sPass((wchar_t*)dwParam1);
		char szPass[17] = {0};
		uint32 nLen = sPass.length();
		for (uint32 i = 0; i < nLen && i < sizeof(szPass); i++)
		{
			szPass[i] = (char)sPass[i];
		}
		m_sPassword = szPass;
		m_bJoinPending = LTTRUE;
	}
	UpdateData(LTTRUE);
}

// RETAIL: 0x10007ae0: tab switches between the two screens
LTBOOL CBaseScreenJoin::HandleKeyDown(int key, int rep)
{
	if (key == VK_TAB)
	{
		if (m_nScreenID == SCREEN_ID_JOIN_SERVERLIST)
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_SERVERINFO);
		else if (m_nScreenID == SCREEN_ID_SERVERINFO)
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_JOIN_SERVERLIST);
	}
	return CBaseScreen::HandleKeyDown(key,rep);
}

// RETAIL: 0x100be620 (folded with CHUDMessage::SetRenderState)
void CBaseScreenJoin::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

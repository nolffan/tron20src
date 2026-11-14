// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScreenHost.cpp
//
// PURPOSE : Base of Tron's host screens: the connection, game and maps
//			 tabs, the list of saved option sets and launching the server
//
// RETAIL: written from retail CShell.dll 0x10002eb0..0x10004cd0 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScreenHost.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "GameClientShell.h"
#include "ServerOptionMgr.h"
#include "LTGUIListItemCtrl.h"

namespace
{
	// RETAIL: inline: a wide string's low bytes into a cleared buffer (the
	// edit box's text is wide; as ScreenSave.cpp's); the name is ours
	inline void NarrowInto(char *pszOut, const std::wstring &sText, uint32 nSize)
	{
		memset(pszOut,0,nSize);
		for (uint32 i = 0; i < sText.length() && i < nSize; i++)
			pszOut[i] = (char)sText[i];
	}

	// RETAIL: 0x102105e0: the light cycle warning is shown once
	bool	s_bWarnLightCycles = true;

	// RETAIL: 0x102105e1: the font size of the option sets' list
	uint8	s_nListFontSize = 20;

	// RETAIL: 0x10002ed0: the launch question's answer
	void LaunchCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreenHost *pThisScreen = (CBaseScreenHost *)pData;
		if (bReturn && pThisScreen)
			pThisScreen->SetLaunchPending(LTTRUE);
	}

	// RETAIL: 0x10002ef0: the new set's name
	void NewOptionsCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdHostNewOptions);
	}

	// RETAIL: 0x10002f40: the set's new name
	void RenameOptionsCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdHostRenameOptions);
	}

	// RETAIL: 0x10002f90: deleting the set
	void DeleteOptionsCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdHostDeleteOptions);
	}

	// RETAIL: 0x10002fe0: the warning before hosting a light cycle game on
	// the internet
	void LightCycleWarningCallBack(LTBOOL bReturn, void *pData)
	{
		if (bReturn)
		{
			eScreenID eCurrent = g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID();
			CBaseScreenHost *pThisScreen = (CBaseScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(eCurrent);
			if (pThisScreen)
			{
				s_bWarnLightCycles = false;
				if (!pThisScreen->StartServer())
					g_pInterfaceMgr->LoadFailed();
			}
		}
	}
}

// RETAIL: 0x10003040
CBaseScreenHost::CBaseScreenHost()
{
	m_pConnectionTab = LTNULL;
	m_pGameTab = LTNULL;
	m_pMapsTab = LTNULL;
	m_pLaunch = LTNULL;
	m_pOptionsName = LTNULL;
	m_pGameType = LTNULL;
	m_bLaunchPending = LTFALSE;
	m_nTabColor = 0x80000080;
	m_pOptionsTitle = LTNULL;
	m_pNewOptions = LTNULL;
	m_pRenameOptions = LTNULL;
	m_pDeleteOptions = LTNULL;
	m_pOptionsList = LTNULL;
	m_sNewName = "unnamed";
	m_bFocusing = false;
}

// RETAIL: 0x10003170
CBaseScreenHost::~CBaseScreenHost()
{
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Term();
	}
}

// RETAIL: 0x10003200
LTBOOL CBaseScreenHost::Build()
{
	CreateTitle(0x20);

	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	m_pOptionsName = AddTextItem(std::wstring(),0,0,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"ConfigurationPos"),LTTRUE);
	LTIntPt gameTypePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"GameTypePos");
	m_pGameType = AddTextItem(std::wstring(),0,0,gameTypePos,LTTRUE);

	LTRect rcTab = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_HOST,"TabRect");
	int nTabWidth = (rcTab.right - rcTab.left) / 3;

	LTVector vColor = g_pLayoutMgr->GetScreenCustomVector(SCREEN_ID_HOST,"TabColor");
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;
	uint8 nA = (uint8)(g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_HOST,"TabAlpha") * 255.0f);
	m_nTabColor = SET_ARGB(nA,nR,nG,nB);

	int nTabX = rcTab.left;
	m_pConnectionTab = AddTextItem(0xF4D,kCmdHostConnectionTab,0,LTIntPt(nTabX,rcTab.top));
	m_pConnectionTab->SetFixedWidth(nTabWidth);
	nTabX += nTabWidth;
	m_pGameTab = AddTextItem(0xF4E,kCmdHostGameTab,0,LTIntPt(nTabX,rcTab.top));
	m_pGameTab->SetFixedWidth(nTabWidth);
	nTabX += nTabWidth;
	m_pMapsTab = AddTextItem(0xF4F,kCmdHostMapsTab,0,LTIntPt(nTabX,rcTab.top));
	m_pMapsTab->SetFixedWidth(nTabWidth);

	LTIntPt launchPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"LaunchPos");
	m_pLaunch = debug_new(CLTGUIButton);

	char szTex[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_HOST,"LaunchTex",szTex,sizeof(szTex));
	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_HOST,"LaunchTexH",szTex,sizeof(szTex));
	HTEXTURE hHot = g_pInterfaceResMgr->GetTexture(szTex);

	m_pLaunch->Create(kCmdHostLaunch,0x212,hNormal,hHot);
	m_pLaunch->SetFont(g_pInterfaceResMgr->GetFontForHeight(nFontSize),nFontSize);
	m_pLaunch->SetText(LoadTempWString(0x199),LTTRUE);
	m_pLaunch->SetBasePos(launchPos);
	m_pLaunch->SetScale(g_pInterfaceResMgr->GetXRatio());
	m_pLaunch->SetCommandHandler(this);
	m_pLaunch->SetFont(g_pInterfaceResMgr->GetFontForHeight(nFontSize),nFontSize);
	m_pLaunch->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_controlArray.push_back(m_pLaunch);

	LTRect rcAvail = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_HOST,"AvailRect");
	m_pOptionsList = AddList(LTIntPt(rcAvail.left,rcAvail.top),rcAvail.bottom - rcAvail.top,LTTRUE,
		(rcAvail.right - rcAvail.left) - 32);
	m_pOptionsList->SetIndent(LTIntPt(5,5));
	// RETAIL: here Tron's list control takes a frame texture
	// ("interface\menu\sprtex\frame_solid.dtx", CLTGUIListCtrl 0x100b98e0):
	// left out, our LTGUIMgr's list control has none yet
	m_pOptionsList->Enable(LTTRUE);
	// RETAIL: and sets a member of Tron's list control (+0x670) to 1: left
	// out, ours has no such member

	m_pOptionsTitle = AddTextItem(0xF92,0,0,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"ConfigTitlePos"),LTTRUE);
	m_pNewOptions = AddTextItem(0xF5C,kCmdHostNewOptions,0xF5D,g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"CommandPos"));
	m_pRenameOptions = AddTextItem(0xF5E,kCmdHostRenameOptions,0xF5F);
	m_pDeleteOptions = AddTextItem(0xF60,kCmdHostDeleteOptions,0xF61);

	char szName[128];
	FormatString(0xA05,szName,sizeof(szName));
	m_sNewName = szName;

	return CBaseScreen::Build();
}

// RETAIL: 0x100038f0: the body's frame, opened where the selected tab is
void CBaseScreenHost::SetTabFrame(CLTGUICtrl *pTab)
{
	if (!pTab) return;

	LTIntPt tabPos = pTab->GetPos();
	float fTabLeft = (float)tabPos.x - 10.0f;
	float fTabTop = (float)tabPos.y - 5.0f;
	float fTabRight = (float)pTab->GetWidth() + fTabLeft;

	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	LTRect rcBody = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_HOST,"BodyRect");
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

// RETAIL: 0x10003b40
uint32 CBaseScreenHost::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleEditResult(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	case kCmdHostSelectOptions:
		{
			if (m_bFocusing)
				break;

			CLTGUICtrl *pCtrl = m_pOptionsList->GetControl(m_pOptionsList->GetSelectedIndex());
			if (!pCtrl)
				break;

			const std::wstring &sText = ((CLTGUITextCtrl *)pCtrl)->GetString()->GetText();
			char szName[128];
			memset(szName,0,sizeof(szName));
			uint32 nLen = sText.length();
			for (int i = 0; i < nLen && i < sizeof(szName); i++)
				szName[i] = (char)sText[i];

			SaveOptions();
			g_pServerOptionMgr->Select(std::string(szName),false);
			LoadOptions();
		}
		break;
	case kCmdHostLaunch:
		SaveOptions();
		if (g_pGameClientShell->IsWorldLoaded())
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = LaunchCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(0x38E,&mb);
		}
		else
		{
			Launch();
		}
		break;
	case kCmdHostConnectionTab:
		if (m_nScreenID != SCREEN_ID_HOST_CONNECTION)
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_CONNECTION,false);
		break;
	case kCmdHostGameTab:
		if (m_nScreenID != SCREEN_ID_HOST_GAME)
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_GAME,false);
		break;
	case kCmdHostMapsTab:
		if (m_nScreenID != SCREEN_ID_HOST_MAPS)
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_MAPS,false);
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	case kCmdHostNewOptions:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = NewOptionsCallBack;
			mb.pString = m_sNewName;
			mb.nMaxChars = 16;
			mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
			g_pInterfaceMgr->ShowMessageBox(0xF62,&mb);
		}
		break;
	case kCmdHostRenameOptions:
		{
			ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = RenameOptionsCallBack;
			mb.pString = pOptions->m_sOptionsName.c_str();
			mb.nMaxChars = 16;
			mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
			g_pInterfaceMgr->ShowMessageBox(0xF5E,&mb);
		}
		break;
	case kCmdHostDeleteOptions:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = DeleteOptionsCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(0xF63,&mb);
		}
		break;
	}
	return 1;
}

// RETAIL: 0x10003f10: the set's name and game type
void CBaseScreenHost::LoadOptions()
{
	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();

	std::wstring sText;
	char szText[128];

	// RETAIL: passes the std::string itself to FormatString's "..." (see
	// docs/RETAIL_BUGS.md); its first member is the text, so "%s" reads it
	FormatString(0xF48,szText,sizeof(szText),pOptions->m_sOptionsName);
	LTGUIWiden(sText,szText);
	m_pOptionsName->SetString(sText);

	int nGameType = 0;
	switch (pOptions->m_eGameType)
	{
	case eGameTypeDiscArena:
		nGameType = 0;
		break;
	case eGameTypeDiscTournament:
		nGameType = 1;
		break;
	case eGameTypeMultiplayerLightcycles:
		nGameType = 2;
		break;
	}
	FormatString(0xF50,szText,sizeof(szText),LoadTempString(0xF51 + nGameType));
	LTGUIWiden(sText,szText);
	m_pGameType->SetString(sText);

	UpdateData(LTFALSE);
}

// RETAIL: 0x100041b0
void CBaseScreenHost::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_bFocusing = true;

		if (g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect(true);
			g_pClientMultiplayerMgr->ForceDisconnect();
		}

		RebuildList();

		if (!g_pMissionButeMgr->Init("Attributes\\nomissions.txt"))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.","Attributes\\nomissions.txt");
			return;
		}

		SetSelection(GetIndex(m_pLaunch));
	}
	else
	{
		SaveOptions();
	}

	CBaseScreen::OnFocus(bFocus);
	m_bFocusing = false;
}

// RETAIL: 0x10004260
LTBOOL CBaseScreenHost::Render(HSURFACE hDestSurf)
{
	SetPolyRenderState();
	g_pDrawPrim->DrawPrim(m_TabPoly,2);
	for (int i = 0; i < 8; i++)
	{
		m_TabLines[i].Render();
	}

	return CBaseScreen::Render(hDestSurf);
}

// RETAIL: 0x100042b0: left and right move between the tabs
LTBOOL CBaseScreenHost::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pConnectionTab || pCtrl == m_pGameTab || pCtrl == m_pMapsTab)
		return OnUp();
	return CBaseScreen::OnLeft();
}

// RETAIL: 0x100042f0
LTBOOL CBaseScreenHost::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pConnectionTab || pCtrl == m_pGameTab)
		return OnDown();
	return CBaseScreen::OnRight();
}

// RETAIL: 0x10004330
void CBaseScreenHost::Launch()
{
	m_bLaunchPending = LTFALSE;
	UpdateData(LTTRUE);

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	if (pOptions->HasNoMaps())
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox(0xF9B,&mb);
		return;
	}

	if (s_bWarnLightCycles && pOptions->m_eGameType == eGameTypeMultiplayerLightcycles && !pOptions->m_bLANOnly)
	{
		MBCreate mb;
		mb.eType = LTMB_YESNO;
		mb.pFn = LightCycleWarningCallBack;
		g_pInterfaceMgr->ShowMessageBox(0xA03,&mb);
		return;
	}

	if (!StartServer())
		g_pInterfaceMgr->LoadFailed();
}

// RETAIL: 0x10004420
LTBOOL CBaseScreenHost::StartServer()
{
	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	uint16 nPort = (uint16)pOptions->m_nPort;
	g_pGameClientShell->SetGameType((GameType)pOptions->m_eGameType);

	// PORT: Tron's SetupServerHost takes the port only (retail 0x100e8a00,
	// ret 4); ours still takes NOLF2's LAN flag
	if (!g_pClientMultiplayerMgr->SetupServerHost(nPort,pOptions->m_bLANOnly))
		return LTFALSE;

	// RETAIL: asked twice
	if (pOptions->HasNoMaps())
		return LTFALSE;
	if (pOptions->HasNoMaps())
		return LTFALSE;

	StringArray *pMaps = pOptions->GetMapList();
	return g_pMissionMgr->StartGameFromLevel((*pMaps)[0].c_str()) ? LTTRUE : LTFALSE;
}

// RETAIL: 0x100044a0
bool CBaseScreenHost::UpdateInterfaceSFX()
{
	if (m_bLaunchPending)
		Launch();
	return CBaseScreen::UpdateInterfaceSFX();
}

// RETAIL: 0x100044c0
void CBaseScreenHost::SetLaunchPending(uint32 bLaunch)
{
	m_bLaunchPending = bLaunch;
}

// RETAIL: 0x100044d0
void CBaseScreenHost::HandleEditResult(uint32 dwParam1, uint32 dwParam2)
{
	switch (dwParam2)
	{
	case kCmdHostNewOptions:
		{
			char szName[128];
			NarrowInto(szName,std::wstring((wchar_t *)dwParam1),sizeof(szName));
			if (!szName[0])
				return;

			SaveOptions();
			g_pServerOptionMgr->Select(std::string(szName),true);
			RebuildList();
		}
		break;
	case kCmdHostRenameOptions:
		{
			char szName[128];
			NarrowInto(szName,std::wstring((wchar_t *)dwParam1),sizeof(szName));
			if (!szName[0])
				return;

			ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
			g_pServerOptionMgr->Rename(pOptions->m_sOptionsName,std::string(szName));
			RebuildList();
		}
		break;
	case kCmdHostDeleteOptions:
		{
			ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
			g_pServerOptionMgr->Delete(pOptions->m_sOptionsName);
			RebuildList();
			LoadOptions();
		}
		break;
	}

	UpdateData(LTTRUE);
}

// RETAIL: 0x100047f0: tab and shift+tab go through the host screens
LTBOOL CBaseScreenHost::HandleKeyDown(int key, int rep)
{
	eScreenID eNext[4] = { SCREEN_ID_HOST_GAME, SCREEN_ID_HOST_MAPS, SCREEN_ID_HOST_CONNECTION, SCREEN_ID_NONE };
	eScreenID ePrev[4] = { SCREEN_ID_HOST_MAPS, SCREEN_ID_HOST_CONNECTION, SCREEN_ID_HOST_GAME, SCREEN_ID_NONE };

	if (key == VK_TAB)
	{
		int nScreen = m_nScreenID - SCREEN_ID_HOST_CONNECTION;
		if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
			g_pInterfaceMgr->SwitchToScreen(ePrev[nScreen],false);
		else
			g_pInterfaceMgr->SwitchToScreen(eNext[nScreen],false);
	}

	return CBaseScreen::HandleKeyDown(key,rep);
}

// RETAIL: 0x10004880
CLTGUIListItemCtrl* CBaseScreenHost::CreateListItem(std::wstring sString, uint32 nSelectCommandID, int helpID,
													LTIntPt pos, LTBOOL bFixed, int nFont)
{
	CLTGUIListItemCtrl* pCtrl = debug_new(CLTGUIListItemCtrl);

	if (nFont < 0)
	{
		nFont = g_pLayoutMgr->GetScreenFontFace((eScreenID)m_nScreenID);
	}

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	if (!pCtrl->Create(sString,0,nSelectCommandID,helpID,pFont,nFontSize,this))
	{
		debug_delete(pCtrl);
		return LTNULL;
	}

	if (pos.x < 0 && pos.y < 0)
	{
		pos = m_nextPos;
	}
	else
	{
		m_nextPos = pos;
	}
	pCtrl->SetBasePos(pos);

	m_nextPos.y += (pCtrl->GetHeight() + m_nItemSpacing);

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	if (bFixed)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}

	return pCtrl;
}

// RETAIL: 0x10004aa0
void CBaseScreenHost::RebuildList()
{
	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	std::wstring sName;

	m_pOptionsList->RemoveAll(LTTRUE);

	StringList lstNames;
	g_pServerOptionMgr->GetNames(lstNames);

	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(s_nListFontSize);

	uint32 nIndex = 0;
	for (StringList::iterator iter = lstNames.begin(); iter != lstNames.end(); ++iter, ++nIndex)
	{
		const char *pszName = iter->c_str();
		sName.resize(0);
		uint32 nLen = strlen(pszName);
		for (uint32 i = 0; i < nLen; i++)
			sName += (wchar_t)(uint8)pszName[i];

		CLTGUIListItemCtrl *pItem = CreateListItem(sName,kCmdHostSelectOptions,0);
		pItem->SetFont(pFont,s_nListFontSize);
		pItem->SetParam1(nIndex);
		pItem->SetFixedWidth(110,LTTRUE);
		m_pOptionsList->AddControl(pItem);

		if (*iter == pOptions->m_sOptionsName || !nIndex)
			m_pOptionsList->SetSelection(nIndex);
	}
}

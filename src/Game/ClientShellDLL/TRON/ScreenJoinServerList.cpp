// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinServerList.cpp
//
// PURPOSE : Tron's server list: the GameSpy internet and LAN browser
//
// RETAIL: written from retail CShell.dll 0x10062af0..0x10064fb0 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoinServerList.h"
#include "InterfaceMgr.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "ClientSoundMgr.h"
#include "ltquaduvutils.h"
#include "../../Libs/GameSpy/GameSpySDK/qr2/qr2regkeys.h"

namespace
{
	// RETAIL: Tron's command IDs (the server list's)
	const uint32 kCmdRefresh		= 0x436;
	const uint32 kCmdSortName		= 0x437;
	const uint32 kCmdSortPing		= 0x438;
	const uint32 kCmdSortPlayers	= 0x439;
	const uint32 kCmdSortType		= 0x43A;
	const uint32 kCmdSortMap		= 0x43B;
	const uint32 kCmdSelectServer	= 0x43C;
	const uint32 kCmdEditAddress	= 0x43D;
	const uint32 kCmdEditPort		= 0x43E;
	const uint32 kCmdPingFilter		= 0x43F;
	const uint32 kCmdPlayersFilter	= 0x440;
	const uint32 kCmdTypeFilter		= 0x441;

	// RETAIL: the sort fields (the browser's sort keys: SortServers)
	enum
	{
		kSortDedicated = 1,
		kSortPassword,
		kSortName,
		kSortPing,
		kSortPlayers,
		kSortType,
		kSortMap,
	};

	// RETAIL: 0x10236c8d: the browser is updating
	bool	s_bUpdating = false;

	// RETAIL: 0x102190cc, 0x102190d8: the keys of the basic update
	// (hostname, 3, mapname, gametype, numplayers, maxplayers, password,
	// Tron's "dedicated" 0x38, 0x1e)
	unsigned char s_aBasicFields[] = { HOSTNAME_KEY, 3, MAPNAME_KEY, GAMETYPE_KEY, NUMPLAYERS_KEY,
		MAXPLAYERS_KEY, PASSWORD_KEY, 0x38, 0x1E };
	int	s_nNumBasicFields = sizeof(s_aBasicFields);

	// RETAIL: 0x101e7b7c: the least time between two refreshes
	const float kRefreshTime = 8.0f;

	// RETAIL: 0x101e9584, 0x101e9588: the version the servers must have
	const int kVersionMajor = 1;
	const int kVersionMinor = 18;

	// RETAIL: 0x10062af0
	void ServerBrowserCallBack(ServerBrowser sb, SBCallbackReason reason, SBServer server, void *instance)
	{
		CScreenJoinServerList *pThisScreen = (CScreenJoinServerList *)instance;
		if (reason == sbc_serverupdated)
		{
			if (g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID() == SCREEN_ID_JOIN_SERVERLIST)
				pThisScreen->AddServer(server);
		}
		pThisScreen->UpdateState();
	}

	// RETAIL: 0x10062b30
	void EditAddressCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_SERVERLIST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdEditAddress);
	}

	// RETAIL: 0x10062b70
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CBaseScreen *pThisScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_SERVERLIST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdEditPort);
	}
}

// RETAIL: 0x10062bb0
CScreenJoinServerList::CScreenJoinServerList()
{
	m_pRefresh = LTNULL;
	m_pNameHeader = LTNULL;
	m_pPingHeader = LTNULL;
	m_pPlayersHeader = LTNULL;
	m_pTypeHeader = LTNULL;
	m_pMapHeader = LTNULL;
	m_pAddressCtrl = LTNULL;
	m_pPortCtrl = LTNULL;
	m_pServerListCtrl = LTNULL;
	m_pPingFilter = LTNULL;
	m_pDedicatedFilter = LTNULL;
	m_pPlayersFilter = LTNULL;
	m_pTypeFilter = LTNULL;
	m_pTronUnknownAC4 = LTNULL;
	m_nPingFilter = 0;
	m_nDedicatedFilter = 0;
	m_nPlayersFilter = 0;
	m_nTypeFilter = 0;
	m_nTronUnknownAC8 = 0;

	sprintf(m_szVersion,"%d.%.3d",kVersionMajor,kVersionMinor-1);

	m_pStatus = LTNULL;
	m_nLockColumn = 0;
	m_nNameColumn = 20;
	m_nPingColumn = 250;
	m_nPlayersColumn = 300;
	m_nTypeColumn = 360;
	m_nMapColumn = 440;
	m_hLocked = LTNULL;
	m_hDedicated = LTNULL;
	m_nSortField = 0;
	m_nSortDir = 2;
	m_nPort = 0x6CF0;

	m_sPort.Format("%d",0x6CF0);
	m_sAddress = "";
}

// RETAIL: 0x10062d70
CScreenJoinServerList::~CScreenJoinServerList()
{
	Term();
}

// RETAIL: 0x10062de0: the browser goes with the screen
void CScreenJoinServerList::Term()
{
	if (g_bServerBrowser)
	{
		ServerBrowserFree(g_hServerBrowser);
		g_bServerBrowser = false;
	}
}

// RETAIL: 0x10062e00
LTBOOL CScreenJoinServerList::Build()
{
	LTBOOL bRet = CBaseScreenJoin::Build();
	SetTabFrame(m_pServerListTab);

	m_nLockColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"LockColumn");
	m_nDedicatedColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"DedicatedColumn");
	m_nNameColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"NameColumn");
	m_nPingColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"PingColumn");
	m_nPlayersColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"PlayersColumn");
	m_nTypeColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"TypeColumn");
	m_nMapColumn = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"MapColumn");
	m_fRefreshTimer = 0.0f;

	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_normal.dtx");
	HTEXTURE hSelected = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_selected.dtx");
	HTEXTURE hDisabled = g_pInterfaceResMgr->GetTexture("interface\\icons\\refresh_disabled.dtx");
	m_pRefresh = debug_new(CLTGUIButton);
	m_pRefresh->Create(kCmdRefresh,0xF66,hNormal,hSelected,hDisabled);
	m_pRefresh->SetCommandHandler(this);

	LTIntPt tablePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"TablePos");
	m_pRefresh->SetBasePos(LTIntPt(m_nLockColumn + tablePos.x,tablePos.y));
	AddControl(m_pRefresh);

	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(14);

	m_pNameHeader = AddTextItem(0x181,kCmdSortName,0xF67,LTIntPt(m_nNameColumn + tablePos.x,tablePos.y));
	m_pNameHeader->SetFont(pFont,14);
	m_pPingHeader = AddTextItem(0x183,kCmdSortPing,0xF68,LTIntPt(m_nPingColumn + tablePos.x,tablePos.y));
	m_pPingHeader->SetFont(pFont,14);
	m_pPlayersHeader = AddTextItem(0x182,kCmdSortPlayers,0xF69,LTIntPt(m_nPlayersColumn + tablePos.x,tablePos.y));
	m_pPlayersHeader->SetFont(pFont,14);
	m_pTypeHeader = AddTextItem(0xF6A,kCmdSortType,0xF6B,LTIntPt(m_nTypeColumn + tablePos.x,tablePos.y));
	m_pTypeHeader->SetFont(pFont,14);
	m_pMapHeader = AddTextItem(0x185,kCmdSortMap,0xF6C,LTIntPt(m_nMapColumn + tablePos.x,tablePos.y));
	m_pMapHeader->SetFont(pFont,14);

	LTRect rcList = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"ListRect");
	m_pServerListCtrl = AddList(LTIntPt(tablePos.x,tablePos.y + 18),rcList.bottom - rcList.top,LTTRUE,rcList.right - rcList.left);
	m_pServerListCtrl->SetIndent(LTIntPt(2,2));
	// RETAIL: here Tron's list control takes a frame texture
	// ("interface\menu\sprtex\frame_solid.dtx", CLTGUIListCtrl 0x100b98e0) and
	// sets two of its members (+0x66c = 0, +0x670 = 1): left out, our
	// LTGUIMgr's list control has neither yet

	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"FilterPos");
	m_pFilterLabel = AddTextItem(0xD79,0,0,pos);
	m_pFilterLabel->Enable(LTFALSE);

	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"FilterPingPos");
	m_pPingFilter = AddCycle(0,0,0,&m_nPingFilter,pos);
	m_pPingFilter->NotifyOnChange(kCmdPingFilter,this);
	m_pPingFilter->AddString(LoadTempWString(0xD6F));
	m_pPingFilter->AddString(LoadTempWString(0xD70));
	m_pPingFilter->AddString(LoadTempWString(0xD71));
	m_pPingFilter->AddString(LoadTempWString(0xD72));

	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"FilterPlayerPos");
	m_pPlayersFilter = AddCycle(0xD7F,0,0,&m_nPlayersFilter,pos);
	m_pPlayersFilter->NotifyOnChange(kCmdPlayersFilter,this);
	m_pPlayersFilter->AddString(LoadTempWString(0xD73));
	m_pPlayersFilter->AddString(LoadTempWString(0xD75));
	m_pPlayersFilter->AddString(LoadTempWString(0xD74));
	m_pPlayersFilter->AddString(LoadTempWString(0xD76));

	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"FilterTypePos");
	m_pTypeFilter = AddCycle(0xD7E,0,0,&m_nTypeFilter,pos);
	m_pTypeFilter->NotifyOnChange(kCmdTypeFilter,this);
	m_pTypeFilter->AddString(LoadTempWString(0xD7A));
	m_pTypeFilter->AddString(LoadTempWString(0xD7B));
	m_pTypeFilter->AddString(LoadTempWString(0xD7C));
	m_pTypeFilter->AddString(LoadTempWString(0xD7D));

	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"StatusPos");
	m_pStatus = AddTextItem(0xD6C,0,0,pos);
	m_pStatus->Enable(LTFALSE);
	m_nStatus = 0;
	m_nLastStatus = 0;

	m_hLocked = g_pInterfaceResMgr->GetTexture("interface\\icons\\locked_server.dtx");
	m_hDedicated = g_pInterfaceResMgr->GetTexture("interface\\icons\\dedicated_server.dtx");
	m_hSortUp = g_pInterfaceResMgr->GetTexture("interface\\icons\\sort_up.dtx");
	m_hSortDown = g_pInterfaceResMgr->GetTexture("interface\\icons\\sort_down.dtx");
	g_pDrawPrim->SetRGBA(&m_SortPoly,argbWhite);
	SetupQuadUVs(m_SortPoly,m_hSortUp,0.0f,0.0f,1.0f,1.0f);

	LTIntPt portPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_SERVERLIST,"PortPos");
	int nPortWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_SERVERLIST,"PortColumnWidth");
	int nIPWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_SERVERLIST,"IPColumnWidth");
	int nValueWidth = (int)(g_pInterfaceResMgr->GetXRatio() * 100.0f);

	m_pAddressCtrl = AddColumnCtrl(kCmdEditAddress,0xF73,portPos);
	m_pAddressCtrl->AddColumn(LoadTempWString(0xF72),nIPWidth);
	m_pAddressCtrl->Show(LTFALSE);
	m_pAddressCtrl->Enable(LTFALSE);

	std::wstring sPort;
	const char *pszPort = m_sPort;
	uint32 nLen = strlen(pszPort);
	for (uint32 i = 0; i < nLen; i++)
	{
		sPort.push_back((uint8)pszPort[i]);
	}

	m_pPortCtrl = AddColumnCtrl(kCmdEditPort,0xF75,portPos);
	m_pPortCtrl->AddColumn(LoadTempWString(0xF74),nPortWidth);
	m_pPortCtrl->AddColumn(sPort,nValueWidth,LTTRUE);
	m_pPortCtrl->Show(LTFALSE);
	m_pPortCtrl->Enable(LTFALSE);

	return bRet;
}

// RETAIL: 0x10063950
void CScreenJoinServerList::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		CBaseScreenJoin *pInfo = (CBaseScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SERVERINFO);
		if (pInfo)
			pInfo->m_bInternet = m_bInternet;

		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
		}

		m_fLastTime = g_pLTClient->GetTime();
		if (m_pScreenMgr->GetLastScreenID() != SCREEN_ID_SERVERINFO)
		{
			m_fRefreshTimer = 0.0f;
			FindServers();
		}
		else
			SortServers();

		SetTabFrame(m_pServerListTab);

		if (m_bInternet)
		{
			m_pAddressCtrl->Show(LTTRUE);
			m_pAddressCtrl->Enable(LTTRUE);
			m_pPortCtrl->Show(LTFALSE);
			m_pPortCtrl->Enable(LTFALSE);
		}
		else
		{
			m_pAddressCtrl->Show(LTFALSE);
			m_pAddressCtrl->Enable(LTFALSE);
			m_pPortCtrl->Show(LTTRUE);
			m_pPortCtrl->Enable(LTTRUE);

			std::wstring sPort;
			const char *pszPort = m_sPort;
			uint32 nLen = strlen(pszPort);
			for (uint32 i = 0; i < nLen; i++)
			{
				sPort.push_back((uint8)pszPort[i]);
			}
			m_pPortCtrl->SetString(1,sPort);
		}
	}
	else
	{
		m_pServerListCtrl->RemoveAll(LTTRUE);
		StopUpdate();
	}

	CBaseScreenJoin::OnFocus(bFocus);
}

// RETAIL: 0x10063c50
void CScreenJoinServerList::ScreenDimsChanged()
{
	CBaseScreen::ScreenDimsChanged();
	SetTabFrame(m_pServerListTab);
}

// RETAIL: 0x10063c70
LTBOOL CScreenJoinServerList::Render(HSURFACE hDestSurf)
{
	if (s_bUpdating)
	{
		ServerBrowserThink(g_hServerBrowser);
		UpdateState();
	}

	if (m_fRefreshTimer > 0.0f)
	{
		float fTime = g_pLTClient->GetTime();
		float fDelta = fTime - m_fLastTime;
		m_fLastTime = fTime;
		m_fRefreshTimer -= fDelta;
		if (m_fRefreshTimer <= 0.0f)
		{
			g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\newping.wav");
			m_pRefresh->Enable(LTTRUE);
		}
	}

	if (m_nLastStatus != m_nStatus)
	{
		m_nLastStatus = m_nStatus;
		int nStringID = 0xD6C;
		switch (m_nStatus)
		{
		case 1: nStringID = 0xD6D; break;
		case 2: nStringID = 0xD6E; break;
		}
		m_pStatus->SetString(LoadTempWString(nStringID));
	}

	LTBOOL bRet = CBaseScreenJoin::Render(hDestSurf);

	if (m_nSortDir != 2)
	{
		LTIntPt pos;
		switch (m_nSortField)
		{
		case kSortName:		pos = m_pNameHeader->GetPos(); break;
		case kSortPing:		pos = m_pPingHeader->GetPos(); break;
		case kSortPlayers:	pos = m_pPlayersHeader->GetPos(); break;
		case kSortType:		pos = m_pTypeHeader->GetPos(); break;
		case kSortMap:		pos = m_pMapHeader->GetPos(); break;
		default:
			return bRet;
		}
		pos.x -= 16;

		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

		g_pDrawPrim->SetTexture(m_nSortDir == 0 ? m_hSortUp : m_hSortDown);
		g_pDrawPrim->SetXYWH(&m_SortPoly,(float)pos.x,(float)pos.y,16.0f,16.0f);
		g_pDrawPrim->DrawPrim(&m_SortPoly,1);
	}

	return bRet;
}

// RETAIL: 0x10063ed0: escape first stops an update
void CScreenJoinServerList::Escape()
{
	if (s_bUpdating == true)
		StopUpdate();
	else
		CBaseScreen::Escape();
}

// RETAIL: 0x10063ef0
LTBOOL CScreenJoinServerList::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		FindServers();
		return LTTRUE;
	}
	return CBaseScreenJoin::HandleKeyDown(key,rep);
}

// RETAIL: inlined in OnCommand: a header clicked twice sorts down
inline void CScreenJoinServerList::SetSort(int nField)
{
	if (m_nSortField == nField && m_nSortDir == 0)
	{
		m_nSortDir = 1;
		SortServers();
		return;
	}
	m_nSortField = nField;
	m_nSortDir = 0;
	SortServers();
}

// RETAIL: 0x10063f20
uint32 CScreenJoinServerList::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}

	switch (dwCommand)
	{
	case CMD_BACK:
		if (s_bUpdating == true)
			StopUpdate();
		m_pScreenMgr->EscapeCurrentScreen();
		break;
	case kCmdRefresh:
		FindServers();
		break;
	case kCmdSortName:		SetSort(kSortName); break;
	case kCmdSortPing:		SetSort(kSortPing); break;
	case kCmdSortPlayers:	SetSort(kSortPlayers); break;
	case kCmdSortType:		SetSort(kSortType); break;
	case kCmdSortMap:		SetSort(kSortMap); break;
	case kCmdSelectServer:
		{
			CLTGUICtrl *pCtrl = m_pServerListCtrl->GetSelectedControl();
			if (pCtrl)
				g_hJoinServer = (SBServer)pCtrl->GetParam1();
			if (dwParam2 == 1)
				JoinServer();
		}
		break;
	case kCmdEditAddress:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditAddressCallBack;
			mb.pString = m_sAddress;
			mb.nMaxChars = 21;
			g_pInterfaceMgr->ShowMessageBox(0xF72,&mb);
		}
		break;
	case kCmdEditPort:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_sPort;
			mb.nMaxChars = 5;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(0xF74,&mb);
		}
		break;
	case kCmdPingFilter:
	case kCmdPlayersFilter:
	case kCmdTypeFilter:
	case kCmdTypeFilter+1:
		UpdateData(LTTRUE);
		SortServers();
		break;
	default:
		return CBaseScreenJoin::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x100641e0: enter joins the selected server
LTBOOL CScreenJoinServerList::OnEnter()
{
	CLTGUICtrl *pCtrl = m_pServerListCtrl->GetSelectedControl();
	if (pCtrl)
	{
		g_hJoinServer = (SBServer)pCtrl->GetParam1();
		JoinServer();
		return LTTRUE;
	}
	return CBaseScreen::OnEnter();
}

// RETAIL: 0x10064220: (re)starts the browser's update, no more often than
// every 8 seconds
void CScreenJoinServerList::FindServers()
{
	if (m_fRefreshTimer > 0.0f)
	{
		MBCreate mb;
		g_pInterfaceMgr->ShowMessageBox(0xCA4,&mb);
		return;
	}

	m_fRefreshTimer = kRefreshTime;
	m_fLastTime = g_pLTClient->GetTime();
	m_pRefresh->Enable(LTFALSE);
	g_hJoinServer = LTNULL;
	m_pServerListCtrl->RemoveAll(LTTRUE);

	if (!g_bServerBrowser)
	{
		// the more bandwidth, the more servers are queried at once
		int nMaxUpdates = 10;
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		int nBandwidth = pProfile->m_nBandwidthClient;
		if (nBandwidth == 6)
		{
			if (pProfile->m_nBandwidthClientCustom >= 128)
				nMaxUpdates = 20;
		}
		else if (nBandwidth >= 4)
			nMaxUpdates = 30;
		else if (nBandwidth >= 1)
			nMaxUpdates = 20;

		qr2_register_key(0x38,"dedicated");
		g_hServerBrowser = ServerBrowserNew("tron20","tron20",m_szSecretKey,0,nMaxUpdates,QVERSION_QR2,ServerBrowserCallBack,this);
		g_bServerBrowser = true;
	}

	StopUpdate();
	ServerBrowserClear(g_hServerBrowser);

	if (m_bInternet)
	{
		ServerBrowserUpdate(g_hServerBrowser,SBTrue,SBTrue,s_aBasicFields,s_nNumBasicFields,LTNULL);
		s_bUpdating = true;
	}
	else
	{
		ServerBrowserLANUpdate(g_hServerBrowser,SBTrue,(uint16)m_nPort,(uint16)(m_nPort + 20));
		s_bUpdating = true;
	}
}

// RETAIL: 0x100643c0
void CScreenJoinServerList::StopUpdate()
{
	if (s_bUpdating)
	{
		ServerBrowserHalt(g_hServerBrowser);
		ServerBrowserThink(g_hServerBrowser);
		s_bUpdating = false;
		UpdateState();
	}
}

// RETAIL: 0x10064400: sorts the browser's servers and refills the list
void CScreenJoinServerList::SortServers()
{
	if (!g_bServerBrowser)
		return;

	m_pServerListCtrl->RemoveAll(LTTRUE);

	SBBool bAscending = (SBBool)(m_nSortDir != 1);
	switch (m_nSortField)
	{
	case kSortName:			ServerBrowserSort(g_hServerBrowser,bAscending,"hostname",sbcm_stricase); break;
	case kSortPing:			ServerBrowserSort(g_hServerBrowser,bAscending,"ping",sbcm_int); break;
	case kSortPlayers:		ServerBrowserSort(g_hServerBrowser,bAscending,"numplayers",sbcm_int); break;
	case kSortType:			ServerBrowserSort(g_hServerBrowser,bAscending,"gametype",sbcm_stricase); break;
	case kSortMap:			ServerBrowserSort(g_hServerBrowser,bAscending,"mapname",sbcm_stricase); break;
	case kSortDedicated:	ServerBrowserSort(g_hServerBrowser,bAscending,"dedicated",sbcm_int); break;
	case kSortPassword:		ServerBrowserSort(g_hServerBrowser,bAscending,"password",sbcm_int); break;
	}

	int nCount = ServerBrowserCount(g_hServerBrowser);
	for (int i = 0; i < nCount; i++)
	{
		AddServer(ServerBrowserGetServer(g_hServerBrowser,i));
	}

	m_pServerListCtrl->SetSelection(0);
}

// RETAIL: 0x10064510
void CScreenJoinServerList::UpdateState()
{
	switch (ServerBrowserState(g_hServerBrowser))
	{
	case sb_disconnected:
	case sb_connected:
		s_bUpdating = false;
		if (m_nStatus && m_nSortField)
			SortServers();
		m_nStatus = 0;
		break;
	case sb_listxfer:
		m_nStatus = 1;
		break;
	case sb_querying:
		m_nStatus = 2;
		break;
	}
}

// RETAIL: 0x10064590: a row of the list for a server that passes the filters
void CScreenJoinServerList::AddServer(SBServer hServer)
{
	std::wstring sText;

	if (!SBServerHasBasicKeys(hServer))
		return;
	if (!FilterServer(hServer))
		return;

	bool bPassword = (SBServerGetIntValue(hServer,"password",0) != 0);
	bool bDedicated = (SBServerGetIntValue(hServer,"dedicated",0) != 0);
	const char *pszName = SBServerGetStringValue(hServer,"hostname","- none -");
	int nPlayers = SBServerGetIntValue(hServer,"numplayers",0);
	int nMaxPlayers = SBServerGetIntValue(hServer,"maxplayers",0);
	const char *pszType = SBServerGetStringValue(hServer,"gametype","");
	const char *pszMap = SBServerGetStringValue(hServer,"mapname","- none -");
	int nPing = SBServerGetPing(hServer);

	CLTGUIColumnCtrl *pCtrl = CreateColumnCtrl(kCmdSelectServer,0,LTIntPt(0,0));

	// RETAIL: the locked and dedicated icons are texture columns here
	// (CLTGUIColumnCtrl 0x100bc3e0 with m_hLocked/m_nLockColumn and
	// m_hDedicated/m_nDedicatedColumn): left out, our LTGUIMgr's column
	// control has no texture columns yet
	bPassword;
	bDedicated;

	pCtrl->AddColumn(std::wstring(),(uint16)m_nNameColumn);

	char szTemp[1024];
	uint32 i, nLen;

	sText.erase();
	nLen = strlen(pszName);
	for (i = 0; i < nLen; i++)
		sText.push_back((uint8)pszName[i]);
	pCtrl->AddColumn(sText,m_nPingColumn - m_nNameColumn,LTTRUE);

	sprintf(szTemp,"%d",nPing);
	sText.erase();
	nLen = strlen(szTemp);
	for (i = 0; i < nLen; i++)
		sText.push_back((uint8)szTemp[i]);
	pCtrl->AddColumn(sText,m_nPlayersColumn - m_nPingColumn);

	sprintf(szTemp,"%d/%d",nPlayers,nMaxPlayers);
	sText.erase();
	nLen = strlen(szTemp);
	for (i = 0; i < nLen; i++)
		sText.push_back((uint8)szTemp[i]);
	pCtrl->AddColumn(sText,m_nTypeColumn - m_nPlayersColumn);

	// PORT: see CBaseScreenJoin::JoinServer (retail tests whether the type
	// string was known)
	int nTypeID = 0x3D;
	switch (GameStringTypeToGameType(pszType))
	{
	case eGameTypeDiscArena:				nTypeID = 0xF51; break;
	case eGameTypeDiscTournament:			nTypeID = 0xF52; break;
	case eGameTypeMultiplayerLightcycles:	nTypeID = 0xF53; break;
	}
	pCtrl->AddColumn(LoadTempWString(nTypeID),m_nMapColumn - m_nTypeColumn,LTTRUE);

	sText.erase();
	nLen = strlen(pszMap);
	for (i = 0; i < nLen; i++)
		sText.push_back((uint8)pszMap[i]);
	pCtrl->AddColumn(sText,0x244 - m_nMapColumn,LTTRUE);

	pCtrl->SetParam1((uint32)hServer);
	m_pServerListCtrl->AddControl(pCtrl);
}

// RETAIL: 0x10064aa0
bool CScreenJoinServerList::FilterServer(SBServer hServer)
{
	if (m_nPingFilter > 0)
	{
		int nPing = SBServerGetPing(hServer);
		switch (m_nPingFilter)
		{
		case 1: if (nPing >= 100) return false; break;
		case 2: if (nPing >= 200) return false; break;
		case 3: if (nPing >= 400) return false; break;
		}
	}

	if (m_nPlayersFilter)
	{
		int nPlayers = SBServerGetIntValue(hServer,"numplayers",0);
		int nMaxPlayers = SBServerGetIntValue(hServer,"maxplayers",0);
		switch (m_nPlayersFilter)
		{
		case 1: if (nPlayers >= nMaxPlayers) return false; break;		// not full
		case 2: if (nPlayers < 1) return false; break;					// not empty
		case 3: if (nPlayers < 1 || nPlayers >= nMaxPlayers) return false; break;
		}
	}

	if (m_nDedicatedFilter)
	{
		if (!SBServerGetIntValue(hServer,"dedicated",0))
			return false;
	}

	if (m_nTypeFilter)
	{
		const char *pszType = SBServerGetStringValue(hServer,"gametype","");
		switch (m_nTypeFilter)
		{
		case 1: if (_stricmp(pszType,GameTypeToString(eGameTypeDiscArena))) return false; break;
		case 2: if (_stricmp(pszType,GameTypeToString(eGameTypeDiscTournament))) return false; break;
		case 3: if (_stricmp(pszType,GameTypeToString(eGameTypeMultiplayerLightcycles))) return false; break;
		}
	}

	if (_stricmp(SBServerGetStringValue(hServer,"gamever",""),m_szVersion))
		return false;

	return (_stricmp(SBServerGetStringValue(hServer,"modname",""),g_pClientMultiplayerMgr->GetModName()) == 0);
}

// RETAIL: 0x10064c60: the address and the port typed into the message box
void CScreenJoinServerList::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	std::wstring sValue;
	char szValue[256];
	uint32 i, nLen;

	switch (dwParam2)
	{
	case kCmdEditAddress:
		{
			m_pServerListCtrl->ClearSelection();
			sValue = std::wstring((wchar_t*)dwParam1);
			memset(szValue,0,sizeof(szValue));
			nLen = sValue.length();
			for (i = 0; i < nLen && i < sizeof(szValue); i++)
				szValue[i] = (char)sValue[i];
			m_sAddress = szValue;
			JoinAddress(std::string(szValue));
		}
		break;
	case kCmdEditPort:
		{
			sValue = std::wstring((wchar_t*)dwParam1);
			memset(szValue,0,sizeof(szValue));
			nLen = sValue.length();
			for (i = 0; i < nLen && i < sizeof(szValue); i++)
				szValue[i] = (char)sValue[i];
			m_sPort = szValue;
			m_pPortCtrl->SetString(1,sValue);

			int nPort = atoi(szValue);
			if ((uint16)nPort >= 1024 && (uint16)nPort <= 0xBFFF && nPort != m_nPort)
			{
				m_nPort = atoi(szValue);
				m_fRefreshTimer = 0.0f;
				FindServers();
			}
		}
		break;
	default:
		CBaseScreenJoin::HandleCallback(dwParam1,dwParam2);
		return;
	}

	UpdateData(LTTRUE);
}

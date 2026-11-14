// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostConnection.cpp
//
// PURPOSE : Tron's host screen: the session's name, passwords, port and
//			 bandwidth
//
// RETAIL: written from retail CShell.dll 0x1005b9c0..0x1005d9e0, after
// NOLF2's CScreenHost (TO2/ScreenHost.cpp)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostConnection.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ServerOptionMgr.h"
#include "NetDefs.h"
#include "InterfaceMgr.h"

namespace
{
	// RETAIL: the lengths of the session name and the passwords (NOLF2's
	// MAX_SESSION_NAME, MAX_PASSWORD)
	const int kMaxSessionName	= 25;
	const int kMaxPassword		= 17;
	const int kMaxPort			= 5;
	const int kMaxBandwidth		= 7;

	// RETAIL: inline: a wide string's low bytes into a cleared buffer (the
	// edit box's text is wide; as ScreenSave.cpp's); the name is ours
	inline void NarrowInto(char *pszOut, const std::wstring &sText, uint32 nSize)
	{
		memset(pszOut,0,nSize);
		for (uint32 i = 0; i < sText.length() && i < nSize; i++)
			pszOut[i] = (char)sText[i];
	}

	// RETAIL: 0x1005b9e0
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostConnection *pThisScreen = (CScreenHostConnection *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_CONNECTION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	}

	// RETAIL: 0x1005ba20
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostConnection *pThisScreen = (CScreenHostConnection *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_CONNECTION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
	}

	// RETAIL: 0x1005ba60
	void EditScmdPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostConnection *pThisScreen = (CScreenHostConnection *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_CONNECTION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_SCMDPASS);
	}

	// RETAIL: 0x1005baa0
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostConnection *pThisScreen = (CScreenHostConnection *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_CONNECTION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PORT);
	}

	// RETAIL: 0x1005bae0
	void EditBandwidthCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostConnection *pThisScreen = (CScreenHostConnection *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_CONNECTION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	}
}

// RETAIL: 0x1005bb20
CScreenHostConnection::CScreenHostConnection()
{
	m_pName				= LTNULL;
	m_pLANToggle		= LTNULL;
	m_pDedicatedToggle	= LTNULL;
	m_pPassword			= LTNULL;
	m_pPassToggle		= LTNULL;
	m_pScmdPassword		= LTNULL;
	m_pScmdPassToggle	= LTNULL;
	m_pPort				= LTNULL;
	m_pBandwidthCycle	= LTNULL;
	m_pBandwidth		= LTNULL;
	m_nBandwidth		= 0;

	LoadString( 0x1BF, m_sSessionName.GetBuffer( kMaxSessionName ), kMaxSessionName );
	m_sSessionName.ReleaseBuffer( );
	LoadString( 0x1C0, m_sPassword.GetBuffer( kMaxPassword ), kMaxPassword );
	m_sPassword.ReleaseBuffer( );
	// RETAIL: the default password again into m_sPassword, not into
	// m_sScmdPassword (see docs/RETAIL_BUGS.md)
	LoadString( 0x1C0, m_sPassword.GetBuffer( kMaxPassword ), kMaxPassword );
	m_sPassword.ReleaseBuffer( );

	m_sPort.Format( "%d", DEFAULT_PORT );
	m_bUsePassword = LTFALSE;
	m_bLANOnly = LTFALSE;
	m_bDedicated = LTFALSE;
	m_bAllowScmdCommands = LTFALSE;
}

// RETAIL: 0x1005bcc0
CScreenHostConnection::~CScreenHostConnection()
{
}

// RETAIL: 0x1005bd60
LTBOOL CScreenHostConnection::Build()
{
	LTBOOL bRet = CBaseScreenHost::Build();

	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;

	m_pName = AddColumnCtrl(CMD_EDIT_NAME, 0x20E, g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST_CONNECTION,"CommandPos"));
	m_pName->AddColumn(LoadTempWString(0x181), kColumn0);
	m_pName->AddColumn(std::wstring(L"<host name>"), kColumn1, LTTRUE);
	m_pName->GetColumn(1)->SetFixedWidth(160, LTTRUE);

	m_pLANToggle = AddToggle(0xF44, 0xF45, kColumn0, &m_bLANOnly);
	m_pLANToggle->SetOnString(LoadTempString(0x35));
	m_pLANToggle->SetOffString(LoadTempString(0x36));

	m_pDedicatedToggle = AddToggle(0xF46, 0xF47, kColumn0, &m_bDedicated);
	m_pDedicatedToggle->SetOnString(LoadTempString(0x35));
	m_pDedicatedToggle->SetOffString(LoadTempString(0x36));

	m_pPassToggle = AddToggle(0xF4A, 0xF4B, kColumn0, &m_bUsePassword);
	m_pPassToggle->NotifyOnChange(CMD_TOGGLE_PASS, this);
	m_pPassToggle->SetOnString(LoadTempString(0x35));
	m_pPassToggle->SetOffString(LoadTempString(0x36));

	m_pPassword = AddColumnCtrl(CMD_EDIT_PASS, 0x2FB);
	m_pPassword->AddColumn(LoadTempWString(0x2FA), kColumn0);
	m_pPassword->AddColumn(std::wstring(L"<<password>"), kColumn1, LTTRUE);
	m_pPassword->GetColumn(1)->SetFixedWidth(160, LTTRUE);

	m_pScmdPassToggle = AddToggle(0x5DF, 0x5E0, kColumn0, &m_bAllowScmdCommands);
	m_pScmdPassToggle->NotifyOnChange(CMD_TOGGLE_SCMDPASS, this);
	m_pScmdPassToggle->SetOnString(LoadTempString(0x35));
	m_pScmdPassToggle->SetOffString(LoadTempString(0x36));

	m_pScmdPassword = AddColumnCtrl(CMD_EDIT_SCMDPASS, 0x5E1);
	m_pScmdPassword->AddColumn(LoadTempWString(0x5E2), kColumn0);
	m_pScmdPassword->AddColumn(std::wstring(L"<<password>"), kColumn1, LTTRUE);
	m_pScmdPassword->GetColumn(1)->SetFixedWidth(160, LTTRUE);

	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, 0x2A9);
	m_pPort->AddColumn(LoadTempWString(0x2A8), kColumn0);
	m_pPort->AddColumn(std::wstring(L"port"), kColumn1, LTTRUE);

	m_pBandwidthCycle = AddCycle(0x75, 0x76, kColumn0, &m_nBandwidth);
	m_pBandwidthCycle->AddString(LoadTempWString(0x70));
	m_pBandwidthCycle->AddString(LoadTempWString(0x71));
	m_pBandwidthCycle->AddString(LoadTempWString(0x286));
	m_pBandwidthCycle->AddString(LoadTempWString(0x72));
	m_pBandwidthCycle->AddString(LoadTempWString(0x73));
	m_pBandwidthCycle->AddString(LoadTempWString(0x74));
	m_pBandwidthCycle->AddString(LoadTempWString(0x32));

	m_pBandwidth = AddColumnCtrl(CMD_EDIT_BANDWIDTH, 0x78);
	m_pBandwidth->AddColumn(LoadTempWString(0x79), kColumn0);
	m_pBandwidth->AddColumn(std::wstring(L"bandwidth"), kColumn1, LTTRUE);

	return bRet;
}

// RETAIL: 0x1005c4a0
uint32 CScreenHostConnection::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleEditResult(dwParam1,dwParam2);
		return 1;
	}

	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sSessionName;
			mb.nMaxChars = kMaxSessionName - 1;
			g_pInterfaceMgr->ShowMessageBox(0x196,&mb);
		} break;
	case CMD_EDIT_PASS:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPassCallBack;
			mb.pString = m_sPassword;
			mb.nMaxChars = kMaxPassword - 1;
			g_pInterfaceMgr->ShowMessageBox(0x2FA,&mb);
		} break;
	case CMD_EDIT_SCMDPASS:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditScmdPassCallBack;
			mb.pString = m_sScmdPassword;
			mb.nMaxChars = kMaxPassword - 1;
			g_pInterfaceMgr->ShowMessageBox(0x5E2,&mb);
		} break;
	case CMD_EDIT_PORT:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_sPort;
			mb.nMaxChars = kMaxPort;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(0x2A8,&mb);
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pString = m_sBandwidth;
			mb.nMaxChars = kMaxBandwidth;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(0x79,&mb);
		} break;
	case CMD_TOGGLE_PASS:
		{
			m_pPassToggle->UpdateData(LTTRUE);
			m_pPassword->Enable(m_bUsePassword);
		} break;
	case CMD_TOGGLE_SCMDPASS:
		{
			m_pScmdPassToggle->UpdateData(LTTRUE);
			m_pScmdPassword->Enable(m_bAllowScmdCommands);
		} break;
	default:
		return CBaseScreenHost::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// RETAIL: 0x1005c730
void CScreenHostConnection::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		LoadOptions();
		SetTabFrame(m_pConnectionTab);
	}

	CBaseScreenHost::OnFocus(bFocus);

	if (bFocus)
	{
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
	}
}

// RETAIL: 0x1005c780
void CScreenHostConnection::ScreenDimsChanged()
{
	CBaseScreen::ScreenDimsChanged();
	SetTabFrame(m_pConnectionTab);
}

// RETAIL: 0x1005c7a0
LTBOOL CScreenHostConnection::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = 5;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		return LTTRUE;
	}
	return CBaseScreenHost::OnLeft();
}

// RETAIL: 0x1005c800
LTBOOL CScreenHostConnection::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex() + 1;
		if (m_nBandwidth >= 6)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		return LTTRUE;
	}
	return CBaseScreenHost::OnRight();
}

// RETAIL: 0x1005c860
LTBOOL CScreenHostConnection::OnLButtonUp(int x, int y)
{
	if (!CBaseScreen::OnLButtonUp(x,y))
		return LTFALSE;

	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (m_nBandwidth >= 6)
		{
			m_nBandwidth = 0;
			m_pBandwidthCycle->SetSelIndex(0);
		}
		UpdateBandwidth();
	}
	return LTTRUE;
}

// RETAIL: 0x1005c8d0
LTBOOL CScreenHostConnection::OnRButtonUp(int x, int y)
{
	if (!CBaseScreen::OnRButtonUp(x,y))
		return LTFALSE;

	// RETAIL: asked for and not used
	GetSelectedControl();
	return LTTRUE;
}

// RETAIL: 0x1005c910
void CScreenHostConnection::UpdateBandwidth()
{
	if (m_nBandwidth >= eBandwidth_Custom)
		m_sBandwidth.Format("%d",g_pServerOptionMgr->GetCurrent()->m_nBandwidthServerCustom);
	else
		m_sBandwidth.Format("%d",g_BandwidthServer[m_nBandwidth]);

	std::wstring sText;
	LTGUIWiden(sText,m_sBandwidth);
	m_pBandwidth->SetString(1,sText);
}

// RETAIL: 0x1005cb00
void CScreenHostConnection::HandleEditResult(uint32 dwParam1, uint32 dwParam2)
{
	std::wstring sText;

	switch(dwParam2)
	{
	case CMD_EDIT_NAME:
		{
			sText = std::wstring((wchar_t *)dwParam1);
			char szName[kMaxSessionName];
			NarrowInto(szName,sText,sizeof(szName));
			m_sSessionName = szName;
			m_sSessionName = m_sSessionName.Left(kMaxSessionName);
			m_pName->SetString(1,sText);
		} break;
	case CMD_EDIT_PASS:
		{
			sText = std::wstring((wchar_t *)dwParam1);
			char szPass[kMaxPassword];
			NarrowInto(szPass,sText,sizeof(szPass));
			m_sPassword = szPass;
			m_sPassword = m_sPassword.Left(kMaxPassword);
			m_pPassword->SetString(1,sText);
		} break;
	case CMD_EDIT_SCMDPASS:
		{
			sText = std::wstring((wchar_t *)dwParam1);
			char szPass[kMaxPassword];
			NarrowInto(szPass,sText,sizeof(szPass));
			m_sScmdPassword = szPass;
			m_sScmdPassword = m_sScmdPassword.Left(kMaxPassword);
			m_pScmdPassword->SetString(1,sText);
		} break;
	case CMD_EDIT_PORT:
		{
			sText = std::wstring((wchar_t *)dwParam1);
			char szPort[kMaxSessionName];
			NarrowInto(szPort,sText,sizeof(szPort));
			uint16 nPort = (uint16)atoi(szPort);
			if (nPort >= 1024 && nPort <= 0xBFFF)
			{
				m_sPort = szPort;
				m_sPort = m_sPort.Left(kMaxPort);
				m_pPort->SetString(1,sText);
			}
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			sText = std::wstring((wchar_t *)dwParam1);
			char szBandwidth[kMaxSessionName];
			NarrowInto(szBandwidth,sText,sizeof(szBandwidth));
			uint16 nBandwidth = (uint16)atoi(szBandwidth);
			if (nBandwidth >= 16 && nBandwidth <= 20000)
			{
				m_sBandwidth = szBandwidth;
				m_sBandwidth = m_sBandwidth.Left(kMaxBandwidth);
				m_nBandwidth = eBandwidth_Custom;
				g_pServerOptionMgr->GetCurrent()->m_nBandwidthServerCustom = (uint16)atol(m_sBandwidth);
				m_pBandwidthCycle->UpdateData(LTFALSE);
				UpdateBandwidth();
			}
			else
			{
				MBCreate mb;
				char *pszMsg = FormatTempString(0x1F2,16,20000);
				LTGUIWiden(sText,pszMsg);
				// PORT: Tron's ShowMessageBox takes the wide text (by value)
				g_pInterfaceMgr->ShowMessageBox(pszMsg,&mb);
			}
		} break;
	default:
		CBaseScreenHost::HandleEditResult(dwParam1,dwParam2);
		return;
	}

	UpdateData(LTTRUE);
}

// RETAIL: 0x1005d370
void CScreenHostConnection::LoadOptions()
{
	CBaseScreenHost::LoadOptions();

	std::wstring sText;
	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();

	m_sSessionName = pOptions->m_sSessionName.c_str();
	if (m_sSessionName.GetLength() < 1)
	{
		LoadString( 0x1BF, m_sSessionName.GetBuffer( kMaxSessionName ), kMaxSessionName );
	}
	m_sSessionName = m_sSessionName.Left( kMaxSessionName );
	LTGUIWiden(sText,m_sSessionName);
	m_pName->SetString(1,sText);

	m_bLANOnly = pOptions->m_bLANOnly;
	m_bDedicated = pOptions->m_bDedicated;
	m_bUsePassword = pOptions->m_bUsePassword;
	m_bAllowScmdCommands = pOptions->m_bAllowScmdCommands;

	m_sPassword = pOptions->m_sPassword.c_str();
	m_sPassword = m_sPassword.Left( kMaxPassword );
	m_pPassword->Enable(m_bUsePassword);
	LTGUIWiden(sText,m_sPassword);
	m_pPassword->SetString(1,sText);

	m_sScmdPassword = pOptions->m_sScmdPassword.c_str();
	m_sScmdPassword = m_sScmdPassword.Left( kMaxPassword );
	m_pScmdPassword->Enable(m_bAllowScmdCommands);
	LTGUIWiden(sText,m_sScmdPassword);
	m_pScmdPassword->SetString(1,sText);

	m_nBandwidth = pOptions->m_nBandwidthServer;
	m_sPort.Format( "%d", pOptions->m_nPort );
	LTGUIWiden(sText,m_sPort);
	m_pPort->SetString(1,sText);

	UpdateBandwidth();
	SetSelection(GetIndex(m_pConnectionTab));
	UpdateData(LTFALSE);
}

// RETAIL: 0x1005d810
void CScreenHostConnection::SaveOptions()
{
	UpdateData(LTTRUE);

	ServerGameOptions *pOptions = g_pServerOptionMgr->GetCurrent();
	pOptions->m_sSessionName = (const char*)m_sSessionName;
	pOptions->m_sPassword = (const char*)m_sPassword;
	pOptions->m_sScmdPassword = (const char*)m_sScmdPassword;
	pOptions->m_bLANOnly = !!m_bLANOnly;
	pOptions->m_bDedicated = !!m_bDedicated;
	pOptions->m_bUsePassword = !!m_bUsePassword;
	pOptions->m_bAllowScmdCommands = !!m_bAllowScmdCommands;
	pOptions->m_nPort = atoi(m_sPort);
	pOptions->m_nBandwidthServer = m_nBandwidth;

	g_pServerOptionMgr->SaveCurrent();
}

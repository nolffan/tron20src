// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.cpp
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's player screen, written from CShell.dll 0x10072880..
// 0x10075190 (NOLF2's TO2 screen was the start). Its string IDs, commands
// (name 0x1c, bandwidth 0x2a, game type 0x43b) and screen ID (8) are Tron's
// numbers. Where Tron's light cycle and team managers stand behind it (the
// light cycle colors, the team models), NOLF2's managers stand in (PORT).
#include "stdafx.h"
#include "ScreenPlayer.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"
#include "ModelButeMgr.h"
#include "LightCycleButeMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

static const int kMaxBandwidthStrLen = 8;

extern bool g_bLAN;

namespace
{
	// RETAIL: Tron's command IDs
	const uint32 kCmdEditName		= 0x1C;
	const uint32 kCmdEditBandwidth	= 0x2A;
	const uint32 kCmdGameType		= 0x43B;

	// RETAIL: Tron's interface sound for a changed value (2; NOLF2's IS_CHANGE
	// is 1)
	const int kSndChange			= 2;

	// RETAIL: 0x1026eb18, the console's DynamicLight while the screen shows
	int		s_nDynamicLight;

	// RETAIL: 0x1021a798, the character's turn rate
	float	s_fRotationRate = 1.0f;

	// RETAIL: 0x100728a0
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdEditName);
	};
	// RETAIL: 0x100728e0
	void EditBandwidthCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,kCmdEditBandwidth);
	};

	#define INVALID_ANI			((HMODELANIM)-1)
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x10072920
CScreenPlayer::CScreenPlayer()
{
	m_bCanChangeColor	= true;
	m_bCanChangeModel	= true;
	m_pName			= LTNULL;
	m_pModel		= LTNULL;
	m_pLeft			= LTNULL;
	m_pRight		= LTNULL;
	m_pColorLeft	= LTNULL;
	m_pColorRight	= LTNULL;
	m_nCurrentModel = 0;
	m_nCurrentColor = 0;

	m_pBandwidthCycle	= LTNULL;
	m_pBandwidth		= LTNULL;
	m_nBandwidth		= 0;

	m_pGameType			= LTNULL;
	m_nGameType			= 0;
	m_nCycleTeam		= 0;
	m_nCycleColor		= 0;
	m_fRotation			= 90.0f;
	m_fLastTime			= 0.0f;
}

// RETAIL: 0x10072a40
CScreenPlayer::~CScreenPlayer()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayer::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x10072af0
void CScreenPlayer::Term()
{
	CBaseScreen::Term();
}

// Build the screen
// RETAIL: 0x10072b00
LTBOOL CScreenPlayer::Build()
{
	CreateTitle(0x13);

	m_pGameType = AddCycle(0xF5A, 0xF5B, 200, &m_nGameType, g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"GameTypePos"));
	m_pGameType->NotifyOnChange(kCmdGameType, this);
	m_pGameType->AddString(LoadTempWString(0xF93));
	m_pGameType->AddString(LoadTempWString(0xF94));

	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"NamePos");
	int nColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"NameColumn");
	int nWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"NameWidth");
	m_pName = AddColumnCtrl(kCmdEditName, 0x1FC, pos);
	m_pName->AddColumn(LoadTempWString(0x1F8), nColumn);
	m_pName->AddColumn(L"<player name>", nWidth, LTTRUE);

	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"BandwidthPos");
	nColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"BandwidthColumn");
	nWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"BandwidthWidth");
	m_pBandwidthCycle = AddCycle(0x75, 0x76, nColumn, &m_nBandwidth, pos);
	m_pBandwidthCycle->AddString(LoadTempWString(0x70));
	m_pBandwidthCycle->AddString(LoadTempWString(0x71));
	m_pBandwidthCycle->AddString(LoadTempWString(0x286));
	m_pBandwidthCycle->AddString(LoadTempWString(0x72));
	m_pBandwidthCycle->AddString(LoadTempWString(0x73));
	m_pBandwidthCycle->AddString(LoadTempWString(0x74));
	m_pBandwidthCycle->AddString(LoadTempWString(0x32));

	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"Band2Pos");
	nColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"Band2Column");
	nWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"Band2Width");
	m_pBandwidth = AddColumnCtrl(kCmdEditBandwidth, 0x78, pos);
	m_pBandwidth->AddColumn(LoadTempWString(0x79), nColumn);
	m_pBandwidth->AddColumn(L" ", nWidth, LTTRUE);

	m_pModel = AddTextItem(0x226, LTNULL, 0x1FB, g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ModelTextPos"));
	m_pColor = AddTextItem(0xD68, LTNULL, 0xD69, g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ColorTextPos"));

	HTEXTURE hLeft = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt.dtx");
	HTEXTURE hLeftH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt_h.dtx");
	HTEXTURE hRight = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt.dtx");
	HTEXTURE hRightH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt_h.dtx");

	m_pLeft = debug_new(CLTGUIButton);
	if (m_pLeft)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ModelLeftArrowPos");
		m_pLeft->Create(CMD_LEFT,LTNULL,hLeft,hLeftH);
		m_pLeft->SetBasePos(arrowPos);
		AddControl(m_pLeft);
		m_pLeft->SetCommandHandler(this);
	}

	m_pRight = debug_new(CLTGUIButton);
	if (m_pRight)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ModelRightArrowPos");
		m_pRight->Create(CMD_RIGHT,LTNULL,hRight,hRightH);
		m_pRight->SetBasePos(arrowPos);
		AddControl(m_pRight);
		m_pRight->SetCommandHandler(this);
	}

	m_pColorLeft = debug_new(CLTGUIButton);
	if (m_pColorLeft)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ColorLeftArrowPos");
		m_pColorLeft->Create(CMD_LEFT,LTNULL,hLeft,hLeftH);
		m_pColorLeft->SetBasePos(arrowPos);
		AddControl(m_pColorLeft);
		m_pColorLeft->SetCommandHandler(this);
	}

	m_pColorRight = debug_new(CLTGUIButton);
	if (m_pColorRight)
	{
		LTIntPt arrowPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_PLAYER,"ColorRightArrowPos");
		m_pColorRight->Create(CMD_RIGHT,LTNULL,hRight,hRightH);
		m_pColorRight->SetBasePos(arrowPos);
		AddControl(m_pColorRight);
		m_pColorRight->SetCommandHandler(this);
	}

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

// RETAIL: 0x10073240
uint32 CScreenPlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}
	switch(dwCommand)
	{
	case kCmdEditName:
		{
			//show edit box here
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sPlayerName.c_str();
			mb.nMaxChars = MAX_PLAYER_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(0x1F8,&mb);
		} break;
	case kCmdEditBandwidth:
		{
			//show edit box here
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pString = m_sBandwidth;
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(0x79,&mb);
		} break;
	case CMD_LEFT:
		{
			if (m_pModel->IsSelected())
				PrevModel(false);
			else if (m_pColor->IsSelected())
				PrevColor(false);
		} break;
	case CMD_RIGHT:
		{
			if (m_pModel->IsSelected())
				NextModel(false);
			else if (m_pColor->IsSelected())
				NextColor(false);
		} break;
	case kCmdGameType:
		{
			m_pGameType->UpdateData(LTTRUE);
			UpdateGameType();
		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
// RETAIL: 0x10073410. PORT: the light cycle and team parts (Tron's managers,
// 0x1026dd10 and 0x1027065c) are left out; the profile's model, color and
// bandwidth are NOLF2's members.
void CScreenPlayer::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	LTBOOL bInMPGame = (IsMultiplayerGame() && g_pGameClientShell->IsWorldLoaded());
	if (bFocus)
	{
		s_nDynamicLight = GetConsoleInt("DynamicLight",0);
		WriteConsoleInt("DynamicLight",1);

		int n;
		for (n = 0; n < g_pModelButeMgr->GetNumTeamModels(); n++)
		{
			ModelId id = (ModelId)g_pModelButeMgr->GetTeamDefaultModel(n);
			HMODELDB dummy = NULL;
			g_pILTModelClient->CacheModelDB(g_pModelButeMgr->GetModelFilename(id),dummy);
		}

		m_sPlayerName = pProfile->m_sPlayerName;
		m_nCurrentModel = pProfile->m_nDMPlayerModel;
		m_nCurrentColor = pProfile->m_nCPPlayerModel;

		m_bCanChangeColor = true;
		m_bCanChangeModel = true;

		m_pColor->Enable(m_bCanChangeColor);
		m_pColorLeft->Enable(m_bCanChangeColor);
		m_pColorRight->Enable(m_bCanChangeColor);

		std::wstring sName;
		LTGUIWiden(sName, m_sPlayerName.c_str());
		m_pName->SetString(1,sName);

		m_nBandwidth = pProfile->m_nBandwidthClient;
		UpdateBandwidth();

        UpdateData(LTFALSE);
	}
	else
	{
		WriteConsoleInt("DynamicLight",s_nDynamicLight);
		UpdateData();
		pProfile->m_sPlayerName = m_sPlayerName;
		pProfile->m_nDMPlayerModel = m_nCurrentModel;
		if (m_bCanChangeColor)
			pProfile->m_nCPPlayerModel = m_nCurrentColor;

		pProfile->m_nBandwidthClient = m_nBandwidth;
		pProfile->m_nBandwidthClientCustom = (uint16)atol(m_sBandwidth);

		pProfile->Save();
		pProfile->ApplyMultiplayer();

		int n;
		for (n = 0; n < g_pModelButeMgr->GetNumTeamModels(); n++)
		{
			ModelId id = (ModelId)g_pModelButeMgr->GetTeamDefaultModel(n);
			g_pILTModelClient->UncacheModelDB(g_pModelButeMgr->GetModelFilename(id));
		}

		if (bInMPGame)
			g_pClientMultiplayerMgr->UpdateMultiPlayer();
	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		UpdateGameType();
}

// RETAIL: 0x10073b90: the character turns
LTBOOL CScreenPlayer::Render(HSURFACE hDestSurf)
{
	LTBOOL bRet = CBaseScreen::Render(hDestSurf);

	HOBJECT hChar = m_CharSFX.GetObject();
	if (hChar)
	{
		LTFLOAT fTime = g_pLTClient->GetTime();
		m_fRotation += (fTime - m_fLastTime) * s_fRotationRate;
		m_fLastTime = fTime;

		LTRotation rRot;
		rRot.Init();
		rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), m_fRotation);
		g_pLTClient->SetObjectRotation(hChar, &rRot);
	}

	return bRet;
}

// RETAIL: 0x10073ce0
LTBOOL	CScreenPlayer::OnUp()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	m_pColorLeft->Enable(LTFALSE);
	m_pColorRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnUp();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	m_pColorLeft->Enable(m_bCanChangeColor);
	m_pColorRight->Enable(m_bCanChangeColor);
	return bHandled;
}

// RETAIL: 0x10073d80
LTBOOL	CScreenPlayer::OnDown()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	m_pColorLeft->Enable(LTFALSE);
	m_pColorRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnDown();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	m_pColorLeft->Enable(m_bCanChangeColor);
	m_pColorRight->Enable(m_bCanChangeColor);
	return bHandled;
}

// RETAIL: 0x10073e20
LTBOOL	CScreenPlayer::OnLeft()
{
	if (m_pModel->IsSelected())
	{
		PrevModel(true);
		return LTTRUE;
	}
	if (m_pColor->IsSelected())
	{
		PrevColor(true);
		return LTTRUE;
	}
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		PlayInterfaceSound(kSndChange);
	}
	return LTTRUE;
}

// RETAIL: 0x10073ec0
LTBOOL	CScreenPlayer::OnRight()
{
	if (m_pModel->IsSelected())
	{
		NextModel(true);
		return LTTRUE;
	}
	if (m_pColor->IsSelected())
	{
		NextColor(true);
		return LTTRUE;
	}
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		PlayInterfaceSound(kSndChange);
	}
	return LTTRUE;
}


/******************************************************************/
// RETAIL: 0x10073f60
LTBOOL CScreenPlayer::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		PlayInterfaceSound(kSndChange);
        return LTTRUE;
	}
	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
// RETAIL: 0x10073fd0
LTBOOL CScreenPlayer::OnRButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
		PlayInterfaceSound(kSndChange);
        return LTTRUE;
	}
	return CBaseScreen::OnRButtonUp(x,y);
}

// RETAIL: 0x10074050
LTBOOL CScreenPlayer::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseScreen::OnMouseMove(x, y);
	if (GetSelectedControl() != m_pModel)
		m_pModel->Select(LTFALSE);
	if (GetSelectedControl() != m_pColor)
		m_pColor->Select(LTFALSE);
	if (GetSelectedControl() == m_pLeft || GetSelectedControl() == m_pRight)
		m_pModel->Select(LTTRUE);
	else if (GetSelectedControl() == m_pColorLeft || GetSelectedControl() == m_pColorRight)
		m_pColor->Select(LTTRUE);
	return bHandled;
}

// RETAIL: 0x10074130: the cycle color steps through the light cycle bute
// manager's shades (g_pLightCycleButeMgr, CShell 0x1026dd10)
void CScreenPlayer::NextColor(bool bSound)
{
	if (!m_nGameType)
	{
		if (++m_nCurrentColor >= 8)
			m_nCurrentColor = 0;
	}
	else
	{
		m_nCycleColor++;
		if (m_nCycleColor == g_pLightCycleButeMgr->GetNumShades())
			m_nCycleColor = 0;
	}
	UpdateChar();
	if (bSound)
		PlayInterfaceSound(kSndChange);
}

// RETAIL: 0x100741b0
void CScreenPlayer::PrevColor(bool bSound)
{
	if (!m_nGameType)
	{
		if (!m_nCurrentColor)
			m_nCurrentColor = 7;
		else
			m_nCurrentColor--;
	}
	else
	{
		if (!m_nCycleColor)
			m_nCycleColor = g_pLightCycleButeMgr->GetNumShades();
		m_nCycleColor--;
	}
	UpdateChar();
	if (bSound)
		PlayInterfaceSound(kSndChange);
}

// RETAIL: 0x10074220
void CScreenPlayer::NextModel(bool bSound)
{
	if (!m_nGameType)
	{
		m_nCurrentModel++;
		if (m_nCurrentModel >= g_pModelButeMgr->GetNumTeamModels())
			m_nCurrentModel = 0;
	}
	else
	{
		m_nCycleTeam = (m_nCycleTeam != 1);
	}
	UpdateChar();
	if (bSound)
		PlayInterfaceSound(kSndChange);
}

// RETAIL: 0x10074290
void CScreenPlayer::PrevModel(bool bSound)
{
	if (!m_nGameType)
	{
		if (!m_nCurrentModel)
			m_nCurrentModel = g_pModelButeMgr->GetNumTeamModels();
		m_nCurrentModel--;
	}
	else
	{
		m_nCycleTeam = (m_nCycleTeam != 1);
	}
	UpdateChar();
	if (bSound)
		PlayInterfaceSound(kSndChange);
}

// RETAIL: 0x100742f0
void CScreenPlayer::UpdateChar()
{
	if (m_nGameType)
		UpdateCycleChar();
	else
		UpdatePlayerChar();
}

// RETAIL: 0x10074310: the multiplayer model, with the skins of its color
void CScreenPlayer::UpdatePlayerChar()
{
	HOBJECT hChar = m_CharSFX.GetObject();
	if (!hChar) return;
	if (m_nCurrentModel >= g_pModelButeMgr->GetNumMPModels()) return;

	const CModelButeMgr::MPModel* pModel = g_pModelButeMgr->GetMPModel(m_nCurrentModel);
	ModelId id = pModel->m_eModelId;
	if (id == eModelIdInvalid) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, g_pModelButeMgr->GetMultiModelFilename(id));

	const std::vector<std::string> & lstSkins = pModel->m_vecColorSkins[m_nCurrentColor];
	int nSkins = lstSkins.size();
	for (int i = 0; i < nSkins; i++)
	{
		SAFE_STRCPY(createStruct.m_SkinNames[i], lstSkins[i].c_str());
	}

	g_pModelButeMgr->CopyRenderStyleFilenames(id, &createStruct);
	g_pCommonLT->SetObjectFilenames(hChar, &createStruct);

	HMODELANIM hAnim = g_pLTClient->GetAnimIndex(hChar, "Interface");
	if (hAnim != INVALID_ANI)
	{
		g_pLTClient->SetModelAnimation(hChar, hAnim);
	}
	else
	{
		hAnim = g_pLTClient->GetAnimIndex(hChar, "LST");
		if (hAnim != INVALID_ANI)
			g_pLTClient->SetModelAnimation(hChar, hAnim);
		else
			g_pLTClient->SetModelAnimation(hChar, 0);
	}

	LTVector vScale(0.15f, 0.15f, 0.15f);
	g_pLTClient->SetObjectScale(m_CharSFX.GetObject(), &vScale);

	char szTmp[80];
	std::wstring sText;

	FormatString(0xDAC, szTmp, sizeof(szTmp), LoadTempString(pModel->m_nNameId));
	LTGUIWiden(sText, szTmp);
	m_pModel->SetString(sText);
	m_pModel->SetHelpID(0x1FB);

	FormatString(0xDAD, szTmp, sizeof(szTmp), LoadTempString(g_pModelButeMgr->GetColorNameId(m_nCurrentColor)));
	LTGUIWiden(sText, szTmp);
	m_pColor->SetString(sText);
	m_pColor->SetHelpID(0xD69);
}

// RETAIL: 0x10074840: the light cycle of the chosen team (style) and shade
void CScreenPlayer::UpdateCycleChar()
{
	HOBJECT hChar = m_CharSFX.GetObject();
	if (!hChar) return;

	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetShadeCycleColor((uint8)m_nCycleTeam, m_nCycleColor);
	if (!pColor) return;

	uint32 nShade = m_nCycleColor;
	if (nShade >= g_pLightCycleButeMgr->GetNumShades()) return;

	m_pColor->SetString(LoadTempWString(g_pLightCycleButeMgr->GetShade(nShade)->nNameId));
	m_pColor->SetHelpID(0x25CB);
	m_pModel->SetHelpID(0x25C9);

	if (m_nCycleTeam == 0)
	{
		m_pModel->SetString(LoadTempWString(0x259A));
		LTVector vScale(0.1f, 0.1f, 0.1f);
		g_pLTClient->SetObjectScale(hChar, &vScale);
	}
	else
	{
		m_pModel->SetString(LoadTempWString(0x259B));
		LTVector vScale(0.09f, 0.09f, 0.09f);
		g_pLTClient->SetObjectScale(hChar, &vScale);
	}

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, pColor->szModel);
	pColor->blrRenderStyles.CopyList(0, createStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);
	pColor->blrSkins.CopyList(0, createStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	g_pCommonLT->SetObjectFilenames(hChar, &createStruct);
}

// RETAIL: 0x10074b70
void CScreenPlayer::UpdateBandwidth()
{
	if (m_nBandwidth >= eBandwidth_Custom)
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		m_sBandwidth.Format( "%d", pProfile->m_nBandwidthClientCustom);
	}
	else
	{
		m_sBandwidth.Format( "%d", g_BandwidthClient[m_nBandwidth]);
	}

	std::wstring sText;
	LTGUIWiden(sText, m_sBandwidth);
	m_pBandwidth->SetString(1,sText);
}

// RETAIL: Tron's custom bandwidth limits (0x10074e31, 0x10074e3b): fixed
// values, not ProfileMgr.h's GetMinimumBandwidth/GetMaximumBandwidth (from
// the bandwidth tables). The names are ours.
const uint16 kTronMinBandwidth = 16;
const uint16 kTronMaxBandwidth = 20000;
static inline bool TronIsValidBandwidth(uint16 nBandwidth)
{
	return (nBandwidth >= kTronMinBandwidth && nBandwidth <= kTronMaxBandwidth);
}

// RETAIL: inlined in HandleCallback (0x10074dd4, 0x10074fec): a wide
// string's characters into a cleared char buffer (the name is ours)
static inline void NarrowCopy(char* pDest, const std::wstring& sText, uint32 nSize)
{
	memset(pDest, 0, nSize);
	for (uint32 i = 0; i < sText.size() && i < nSize; i++)
		pDest[i] = (char)sText[i];
}

// RETAIL: 0x10074d60: the message box's text is wide
void CScreenPlayer::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case kCmdEditBandwidth:
		{
			char szBandwidth[63];
			NarrowCopy(szBandwidth, std::wstring((wchar_t *)dwParam1), sizeof(szBandwidth));

			uint32 nBandwidth = (uint32)atoi(szBandwidth);
			if ( TronIsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = szBandwidth;
				m_sBandwidth = m_sBandwidth.Left( kMaxBandwidthStrLen );
				m_nBandwidth = eBandwidth_Custom;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				pProfile->m_nBandwidthClientCustom = (uint16)atol(m_sBandwidth);
				m_pBandwidthCycle->UpdateData(LTFALSE);
				UpdateBandwidth();
			}
			else
			{
				// TRON: the message widened (RETAIL: 0x10074f09)
				MBCreate mb;
				std::wstring sMsg;
				LTGUIWiden(sMsg, FormatTempString(498,kTronMinBandwidth,kTronMaxBandwidth));
				g_pInterfaceMgr->ShowMessageBox(sMsg,&mb);
			}
		}
		break;
	case kCmdEditName:
		{
			// RETAIL: 0x10074fac: no test for an empty name, and the name is
			// cut to 16 characters
			char szName[16];
			NarrowCopy(szName, std::wstring((wchar_t *)dwParam1), sizeof(szName));
			m_sPlayerName = szName;
			m_pName->SetString(1,std::wstring((wchar_t *)dwParam1));
		} break;
	}
	UpdateData();
}

// RETAIL: 0x10075100
void CScreenPlayer::UpdateGameType()
{
	if (m_nGameType == 1)
	{
		m_pModel->Enable(m_bCanChangeModel);
		m_pLeft->Enable(m_bCanChangeModel);
		m_pRight->Enable(m_bCanChangeModel);
	}
	else
	{
		m_pModel->Enable(LTTRUE);
		m_pLeft->Enable(LTTRUE);
		m_pRight->Enable(LTTRUE);
	}
	UpdateChar();
}

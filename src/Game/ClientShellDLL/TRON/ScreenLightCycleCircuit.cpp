// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLightCycleCircuit.cpp
//
// PURPOSE : Tron's light cycle circuit screen: the circuits, their lives,
//			 waves, cycle restrictions and image, and launching one
//
// RETAIL: written from retail CShell.dll 0x10065b10..0x10068420 (Tron's
// own class: the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenLightCycleCircuit.h"
#include "ScreenSubroutines.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ProfileMgr.h"
#include "CheatMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "CircuitButeMgr.h"
#include "LightCycleGameDataMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "GameClientShell.h"

namespace
{
	// RETAIL: 0x10219670: the message when the cycle may not race the
	// circuit (a variable)
	int		s_nCycleNotAllowedId = 0x2613;

	// RETAIL: 0x10219674: the font size of the circuit list (a variable)
	int		s_nListFontSize = 16;

	// RETAIL: the cheat that unlocks every cycle color (ours: see
	// CBaseScreenLightCycle)
	const int kCheatTronUnknown37 = 0x37;
}

// RETAIL: 0x10065b80 (called from CTronScreenMgr::AddScreen's new)
CScreenLightCycleCircuit::CScreenLightCycleCircuit()
{
	m_pCircuitList = LTNULL;
	m_pLaunch = LTNULL;
	m_pLives = LTNULL;
	m_pWaves = LTNULL;
	m_nListWidth = 0;
}

// RETAIL: 0x10065dc0
LTBOOL CScreenLightCycleCircuit::Build()
{
	LTBOOL bRet = CBaseScreenLightCycle::Build();

	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	LTRect rcAvail = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	CLTGUITextCtrl *pTitle = AddTextItem(0x25AC,0,0xDA6,LTIntPt(rcAvail.left,rcAvail.top - 20));
	pTitle->Enable(LTFALSE);

	m_nListWidth = rcAvail.right - rcAvail.left - 32;
	m_pCircuitList = AddList(LTIntPt(rcAvail.left,rcAvail.top),rcAvail.bottom - rcAvail.top,LTTRUE,m_nListWidth);
	m_pCircuitList->SetIndent(LTIntPt(5,5));
	m_pCircuitList->SetFrameWidth(2);
	// RETAIL: here a member of Tron's list control is cleared (+0x66c):
	// left out, our LTGUIMgr's list control has none

	LTIntPt restrictPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RestrictionPos");
	AddTextItem(0x25FE,0,0xDA7,restrictPos);

	LTIntPt regularPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RegularPos");
	LTIntPt superPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"SuperPos");
	AddTextItem(0x25F6,0,0xDA9,regularPos);
	AddTextItem(0x25F5,0,0xDA8,superPos);

	int nOffset = restrictPos.y - regularPos.y;
	m_pSuperRestriction = AddTextItem(std::wstring(),0,0xDA9,LTIntPt(superPos.x,superPos.y + nOffset));
	m_pRegularRestriction = AddTextItem(std::wstring(),0,0xDA8,LTIntPt(regularPos.x,regularPos.y + nOffset));
	m_pSuperRestriction->SetColors(argbWhite,argbWhite,argbWhite);
	m_pRegularRestriction->SetColors(argbWhite,argbWhite,argbWhite);

	m_pLives = AddTextItem(std::wstring(),0,0xDAA,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LivesPos"));
	m_pWaves = AddTextItem(std::wstring(),0,0xDAB,g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"WavesPos"));

	LTIntPt launchPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"LaunchPos");
	m_pLaunch = debug_new(CLTGUIButton);

	char szTex[128];
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"LaunchTex",szTex,sizeof(szTex));
	HTEXTURE hNormal = g_pInterfaceResMgr->GetTexture(szTex);
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"LaunchTexH",szTex,sizeof(szTex));
	HTEXTURE hHot = g_pInterfaceResMgr->GetTexture(szTex);

	m_pLaunch->Create(kCmdLCLaunch,0x25B6,hNormal,hHot);
	m_pLaunch->SetFont(g_pInterfaceResMgr->GetFontForHeight(nFontSize),nFontSize);
	m_pLaunch->SetText(LoadTempWString(0x25B5),LTTRUE);
	m_pLaunch->SetBasePos(launchPos);
	m_pLaunch->SetScale(g_pInterfaceResMgr->GetXRatio());
	m_pLaunch->SetCommandHandler(this);
	m_pLaunch->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_controlArray.push_back(m_pLaunch);

	LTIntPt imagePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ImagePos");
	LTRect rcImage = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"ImageRect");
	g_pDrawPrim->SetXYWH(&m_ImagePoly,(float)imagePos.x * g_pInterfaceResMgr->GetXRatio(),(float)imagePos.y * g_pInterfaceResMgr->GetXRatio(),
		(float)(rcImage.right - rcImage.left) * g_pInterfaceResMgr->GetXRatio(),(float)(rcImage.bottom - rcImage.top) * g_pInterfaceResMgr->GetXRatio());
	g_pDrawPrim->SetUVWH(&m_ImagePoly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_ImagePoly,argbWhite);

	LTVector vColor = g_pLayoutMgr->GetScreenCustomVector((eScreenID)m_nScreenID,"CompletedCircuitColor");
	m_nCompletedColor = SET_ARGB(0xFF,(uint8)vColor.x,(uint8)vColor.y,(uint8)vColor.z);
	vColor = g_pLayoutMgr->GetScreenCustomVector((eScreenID)m_nScreenID,"UncompletedCircuitColor");
	m_nUncompletedColor = SET_ARGB(0xFF,(uint8)vColor.x,(uint8)vColor.y,(uint8)vColor.z);

	return bRet;
}

// RETAIL: 0x10066450
void CScreenLightCycleCircuit::ScreenDimsChanged()
{
	LTIntPt imagePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ImagePos");
	LTRect rcImage = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"ImageRect");
	g_pDrawPrim->SetXYWH(&m_ImagePoly,g_pInterfaceResMgr->GetXRatio() * imagePos.x,g_pInterfaceResMgr->GetXRatio() * imagePos.y,
		(float)(rcImage.right - rcImage.left) * g_pInterfaceResMgr->GetXRatio(),(float)(rcImage.bottom - rcImage.top) * g_pInterfaceResMgr->GetXRatio());
	g_pDrawPrim->SetUVWH(&m_ImagePoly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_ImagePoly,argbWhite);

	CBaseScreenLightCycle::ScreenDimsChanged();
}

// RETAIL: 0x10066550: the circuit's image over the screen
LTBOOL CScreenLightCycleCircuit::Render(HSURFACE hDestSurf)
{
	LTBOOL bRet = CBaseScreenLightCycle::Render(hDestSurf);
	if (m_hImage)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_DECAL);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
		g_pDrawPrim->SetTexture(m_hImage);
		g_pDrawPrim->DrawPrim(&m_ImagePoly,1);
	}
	return bRet;
}

// RETAIL: 0x10066600
void CScreenLightCycleCircuit::OnSelectionChange()
{
	if (m_pCircuitList && GetControl(m_nSelection) == m_pCircuitList)
	{
		CLTGUICtrl *pCtrl = m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex());
		if (pCtrl)
			SelectCircuit(pCtrl->GetParam1());
	}
}

// RETAIL: 0x10066640
void CScreenLightCycleCircuit::SelectCircuit(uint32 nCircuit)
{
	const Circuit *pCircuit = &GetCircuitMgr()->GetCircuit(nCircuit);
	if (!pCircuit) return;

	std::wstring sText;
	char szText[256] = "";

	FormatString(0x25FD,szText,sizeof(szText),pCircuit->nInitialLives);
	LTGUIWiden(sText,szText);
	m_pLives->SetString(sText);

	FormatString(0x25FF,szText,sizeof(szText),pCircuit->aWaves.size());
	LTGUIWiden(sText,szText);
	m_pWaves->SetString(sText);

	m_pSuperRestriction->SetString(LoadTempWString(pCircuit->nSuperCycleRestrictionId));
	m_pRegularRestriction->SetString(LoadTempWString(pCircuit->nStandardCycleRestrictionId));

	if (IsCircuitLocked(pCircuit))
		m_pLaunch->Enable(LTFALSE);
	else
		m_pLaunch->Enable(LTTRUE);

	m_hImage = g_pInterfaceResMgr->GetTexture(pCircuit->sImage.c_str());
}

// RETAIL: 0x10066930
uint32 CScreenLightCycleCircuit::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_BACK:
		m_pScreenMgr->EscapeCurrentScreen();
		return 1;
	case kCmdLCLaunch:
		if (IsCycleAllowed())
			LaunchCircuit();
		else
			ShowCycleNotAllowed();
		return 1;
	}
	return CBaseScreenLightCycle::OnCommand(dwCommand,dwParam1,dwParam2);
}

// RETAIL: 0x100669a0
void CScreenLightCycleCircuit::LaunchCircuit()
{
	if (!m_pCircuitList) return;

	g_pGameClientShell->SetGameType(eGameTypeSingleLightCycleCircuit);

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	if (!pData) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return;

	pData->m_bTronUnknownA0 = pProfile->m_nLightCycleStyle;
	pData->m_bTronUnknownA1 = pProfile->m_nLightCycleShade;
	SaveSettings();

	CLTGUICtrl *pCtrl = m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex());
	if (!pCtrl) return;

	// RETAIL: an empty string retail makes and does not use
	std::wstring sUnused;

	const Circuit *pCircuit = &GetCircuitMgr()->GetCircuit(pCtrl->GetParam1());
	if (!pCircuit) return;

	g_pClientMultiplayerMgr->SetupServerSinglePlayer();
	g_pMissionMgr->StartGameFromLevel(pCircuit->sMapName.c_str());
}

// RETAIL: 0x10066b50: CBaseScreen's, but the circuit list keeps its
// selection when the focus leaves it, and the sound is Tron's
uint16 CScreenLightCycleCircuit::SetSelection(uint16 select, LTBOOL bFindSelectable)
{
	if (!m_pCircuitList) return 0;

	if (select == m_nSelection && GetControl(select) != m_pCircuitList) return select;

	int nOldSelect=m_nSelection;

	if (select == kNoSelection)
	{
		if (nOldSelect != kNoSelection)
		{
			GetControl(nOldSelect)->Select(LTFALSE);
			OnSelectionChange();
		}
		m_nOldSelection = m_nSelection;
		m_nSelection = kNoSelection;
		UpdateHelpText();
		return kNoSelection;
	}

	CLTGUICtrl *pSelCtrl;

	if (select >= 0)
	{
		if (select >= m_controlArray.size())
			select = m_controlArray.size()-1;
	}

	pSelCtrl = GetControl(select);
	if (!pSelCtrl)
	{
		UpdateHelpText();
		return nOldSelect;
	}
	//check to see if we can select this item
	if (!pSelCtrl->IsEnabled())
	{
		//if we don't need to find a selectable item return
		if (!bFindSelectable)
		{
			UpdateHelpText();
			return nOldSelect;
		}

		//keep looking until we run out of on screen items or find a selectable one
		while (pSelCtrl && !pSelCtrl->IsEnabled())
		{
			select++;
			pSelCtrl = GetControl(select);
		}
		if (!pSelCtrl || !pSelCtrl->IsEnabled())
		{
			UpdateHelpText();
			return nOldSelect;
		}
	}

	if (nOldSelect != kNoSelection && GetControl(nOldSelect) != m_pCircuitList)
	{
		GetControl(nOldSelect)->Select(LTFALSE);
	}

	m_nOldSelection = m_nSelection;
	m_nSelection = select;

	if (m_nSelection == kNoSelection)
	{
		UpdateHelpText();
		return nOldSelect;
	}

	LTIntPt pos = pSelCtrl->GetPos();
	if (m_bSelectFXCenter)
		pos.x += (pSelCtrl->GetWidth() / 2);
	pos.y += (pSelCtrl->GetHeight() / 2);
	g_pInterfaceMgr->ShowSelectFX(pos);
	PlayInterfaceSound(1);

	UpdateHelpText();
	OnSelectionChange();
	return m_nSelection;
}

// RETAIL: 0x10066d90
LTBOOL CScreenLightCycleCircuit::FillCircuitList()
{
	if (!m_pCircuitList) return LTFALSE;

	m_CircuitNames.clear();
	m_pCircuitList->ClearSelection();
	m_pCircuitList->RemoveAll();

	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight((uint8)s_nListFontSize);

	int nNumCircuits = GetCircuitMgr()->GetNumCircuits();
	std::wstring sName;
	for (int i = 0; i < nNumCircuits; i++)
	{
		const Circuit *pCircuit = &GetCircuitMgr()->GetCircuit(i);

		char szName[80];
		FormatString(pCircuit->nNameId,szName,sizeof(szName));
		LTGUIWiden(sName,szName);

		CLTGUIListItemCtrl *pItem = CreateListItem(sName,kCmdLCLaunch,0);
		pItem->SetFont(pFont,(uint8)s_nListFontSize);
		pItem->SetParam1(i);
		pItem->SetFixedWidth(m_nListWidth,LTTRUE);
		pItem->SetColors(m_SelectedColor,IsCircuitCompleted(pCircuit) ? m_nCompletedColor : m_nUncompletedColor,m_DisabledColor);
		m_pCircuitList->AddControl(pItem);

		m_CircuitNames.push_back(std::string(szName));
	}

	return LTTRUE;
}

// RETAIL: 0x100670a0: the circuit of the current settings is selected
void CScreenLightCycleCircuit::LoadSettings()
{
	CBaseScreenLightCycle::LoadSettings();

	CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
	FillCircuitList();
	m_pCircuitList->Select(LTTRUE);

	int nNumCircuits = GetCircuitMgr()->GetNumCircuits();
	for (int i = 0; i < nNumCircuits; i++)
	{
		if (LCNameEquals(GetCircuitMgr()->GetCircuit(i).sNameKey,pData->m_Circuit.sNameKey))
		{
			pData->m_Circuit = GetCircuitMgr()->GetCircuit(i);
			m_pCircuitList->SetSelection(i);
			SelectCircuit(i);
			UpdateData(LTFALSE);
			return;
		}
	}

	if (nNumCircuits)
	{
		pData->m_Circuit = GetCircuitMgr()->GetCircuit(0);
		m_pCircuitList->SetSelection(0);
		SelectCircuit(0);
	}
	UpdateData(LTFALSE);
}

// RETAIL: 0x10067310
void CScreenLightCycleCircuit::SaveSettings()
{
	UpdateData(LTTRUE);

	CLTGUICtrl *pCtrl = m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex());
	if (pCtrl)
	{
		CLightCycleGameData *pData = g_pLightCycleGameDataMgr->GetCurrent();
		pData->m_Circuit = GetCircuitMgr()->GetCircuit(pCtrl->GetParam1());
		g_pLightCycleGameDataMgr->SaveCurrent();
	}

	CBaseScreenLightCycle::SaveSettings();
}

// RETAIL: 0x100675b0: moving in the list shows the circuit
LTBOOL CScreenLightCycleCircuit::OnUp()
{
	if (!m_pCircuitList) return LTFALSE;

	CLTGUICtrl *pOld = m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex());
	LTBOOL bRet = CBaseScreen::OnUp();
	if (pOld != m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex()))
		OnSelectionChange();
	return bRet;
}

// RETAIL: 0x10067600
LTBOOL CScreenLightCycleCircuit::OnDown()
{
	if (!m_pCircuitList) return LTFALSE;

	CLTGUICtrl *pOld = m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex());
	LTBOOL bRet = CBaseScreen::OnDown();
	if (pOld != m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex()))
		OnSelectionChange();
	return bRet;
}

// RETAIL: 0x10067650
CircuitButeMgr* CScreenLightCycleCircuit::GetCircuitMgr()
{
	return g_pGameClientShell->GetCircuitButeMgr();
}

// RETAIL: 0x10067660
bool CScreenLightCycleCircuit::IsCircuitLocked(const Circuit *pCircuit)
{
	return false;
}

// RETAIL: 0x10067670
bool CScreenLightCycleCircuit::IsCircuitCompleted(const Circuit *pCircuit)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	StringArray::iterator it;
	StringArray::iterator itEnd = pProfile->m_CompletedCircuits.end();
	for (it = pProfile->m_CompletedCircuits.begin(); it != itEnd; ++it)
	{
		if (LCNameEquals(*it,pCircuit->sNameKey))
			break;
	}
	return (it != pProfile->m_CompletedCircuits.end());
}

// RETAIL: 0x100676e0: the circuit must allow the cycle, and the profile
// have unlocked it
bool CScreenLightCycleCircuit::IsCycleAllowed()
{
	CLTGUITextCtrl *pCtrl = dynamic_cast<CLTGUITextCtrl*>(m_pCircuitList->GetControl(m_pCircuitList->GetSelectedIndex()));
	if (!pCtrl) return false;

	const Circuit *pCircuit = &GetCircuitMgr()->GetCircuit(pCtrl->GetParam1());
	if (!pCircuit) return false;

	if (!g_pProfileMgr) return false;

	LIGHTCYCLECOLOR *pColor = LTNULL;
	uint8 nStyle = 0;
	uint8 nShade = 0;
	GetProfileCycleColor(&pColor,&nStyle,&nShade);

	CircuitStringArray::const_iterator it;
	CircuitStringArray::const_iterator itEnd = pCircuit->aCyclesAllowed.end();
	for (it = pCircuit->aCyclesAllowed.begin(); it != itEnd; ++it)
	{
		if (LCNameEquals(*it,pColor->szName))
			break;
	}
	if (it == pCircuit->aCyclesAllowed.end())
		return false;

	if (g_pCheatMgr->IsCheatActive(kCheatTronUnknown37))
		return true;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (!pProfile) return false;

	StringArray::iterator itColor;
	StringArray::iterator itColorEnd = pProfile->m_UnlockedCycleColors.end();
	for (itColor = pProfile->m_UnlockedCycleColors.begin(); itColor != itColorEnd; ++itColor)
	{
		if (LCNameEquals(*itColor,pColor->szName))
			break;
	}
	return (itColor != pProfile->m_UnlockedCycleColors.end());
}

// RETAIL: 0x10067870
void CScreenLightCycleCircuit::ShowCycleNotAllowed()
{
	char szMsg[1024] = "";
	LoadString(s_nCycleNotAllowedId,szMsg,sizeof(szMsg));

	std::wstring sMsg;
	LTGUIWiden(sMsg,szMsg);

	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(sMsg,&mb);
}

// RETAIL: 0x10067a20. RETAIL: on focus retail shows, then clears, the
// message Tron's mission manager keeps for the circuit screen
// (dynamic_cast<CTronMissionMgr*>(g_pMissionMgr), 0x100a3fd0, 0x100a3fe0):
// left out, CTronMissionMgr is not written yet
void CScreenLightCycleCircuit::OnFocus(LTBOOL bFocus)
{
	CBaseScreenLightCycle::OnFocus(bFocus);
}

// RETAIL: 0x1006d860 (folded with the other light cycle screens')
LTRect CScreenLightCycleCircuit::GetBodyRect()
{
	return g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"TabBodyRect");
}

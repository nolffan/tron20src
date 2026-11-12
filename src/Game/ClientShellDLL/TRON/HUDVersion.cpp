// ----------------------------------------------------------------------- //
//
// MODULE  : HUDVersion.cpp
//
// PURPOSE : HUDItem to display Jet's version number
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"

//******************************************************************************************
//**
//** HUD version number display
//**
//******************************************************************************************


CHUDVersion::CHUDVersion() : m_pString(LTNULL)
{
	LTIntPt kNullPt(0,0);

	m_UpdateFlags = kHUDVersion;
	m_BasePos = kNullPt;
	m_fUpdateTime = 1.0f;
	m_vColor = LTVector(32.0f,32.0f,255.0f);	// RETAIL: 0x1003e379
}


// RETAIL: 0x1003e390
LTBOOL CHUDVersion::Init()
{
	UpdateLayout();

	m_BasePos			= g_pLayoutMgr->GetPoint("VersionNumber", "Position");
	m_nFont				= g_pLayoutMgr->GetInt("VersionNumber", "Font");
	m_nFontSize			= g_pLayoutMgr->GetInt("VersionNumber", "FontSize");
	m_vColor			= g_pLayoutMgr->GetVector("VersionNumber", "BaseColor");
	m_nMajorLoops		= g_pLayoutMgr->GetInt("VersionNumber", "MajorUpgradeLoops");
	m_fMajorLoopLength	= g_pLayoutMgr->GetFloat("VersionNumber", "MajorUpgradeLoopLength");
	m_vMajorColor		= g_pLayoutMgr->GetVector("VersionNumber", "MajorUpgradeColor");
	m_nMinorLoops		= g_pLayoutMgr->GetInt("VersionNumber", "MinorUpgradeLoops");
	m_fMinorLoopLength	= g_pLayoutMgr->GetFloat("VersionNumber", "MinorUpgradeLoopLength");
	m_vMinorColor		= g_pLayoutMgr->GetVector("VersionNumber", "MinorUpgradeColor");

	m_fUpdateTime = 1.0f;

	return LTTRUE;
}

void CHUDVersion::Term()
{
	if (m_pString)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pString);
		m_pString = LTNULL;
	}
}

// RETAIL: 0x1003e520: single player only, flashing towards the upgrade's
// colour and back once a loop
void CHUDVersion::Render()
{
	if (IsMultiplayerGame()) return;
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28B14()) return;
	if (!IsEnabled()) return;

	LTVector vColor = m_vColor;

	if (m_nLoops)
	{
		float fTime = g_pLTClient->GetTime();
		float fDelta = fTime - m_fLastTime;
		m_fLastTime = fTime;

		m_fLoopTime -= fDelta;
		if (m_fLoopTime < 0.0f)
		{
			m_fLoopTime += m_fLoopLength;
			m_nLoops--;
		}

		if (m_nLoops)
		{
			float fPhase = (m_fLoopLength - m_fLoopTime) / m_fLoopLength;
			float fBlend = (fPhase > 0.5f) ? (2.0f - fPhase * 2.0f) : (fPhase * 2.0f);

			vColor.x = (m_vFlashColor.x - m_vColor.x) * fBlend + m_vColor.x;
			vColor.y = (m_vFlashColor.y - m_vColor.y) * fBlend + m_vColor.y;
			vColor.z = (m_vFlashColor.z - m_vColor.z) * fBlend + m_vColor.z;
		}
	}

	if (m_pString)
	{
		m_pString->SetColor(SET_ARGB(0xFF, vColor.x, vColor.y, vColor.z));
		m_pString->Render();
	}
}

// RETAIL: 0x1003e660
void CHUDVersion::Update()
{
	float xs = g_pInterfaceResMgr->GetXRatio();

	// build the new string
	int nOldVersion = m_iVersion;
	uint16 newVersion;
	g_pTronPlayerStats->GetJetVersion(newVersion);

	m_fTimeRemaining = m_fUpdateTime;
	m_iVersion = newVersion;

	// a change of version flashes the number: a major one in its colour,
	// a minor one in its own
	if (nOldVersion / 100 != newVersion / 100)
	{
		m_fLastTime = g_pLTClient->GetTime();
		m_fLoopLength = m_fLoopTime = m_fMajorLoopLength;
		m_nLoops = m_nMajorLoops;
		m_vFlashColor = m_vMajorColor;
	}
	else if (nOldVersion != newVersion)
	{
		m_fLastTime = g_pLTClient->GetTime();
		m_fLoopLength = m_fLoopTime = m_fMinorLoopLength;
		m_nLoops = m_nMinorLoops;
		m_vFlashColor = m_vMinorColor;
	}

	int major = newVersion / 100;
	newVersion -= 100 * major;
	int minor = newVersion / 10;
	newVersion -= 10 * minor;

	CUIFont* pFont;
	if (!m_pString)
	{
		pFont = g_pInterfaceResMgr->GetFontForHeight((int)(m_nFontSize * xs));
		m_pString = g_pLTClient->GetFontManager()->CreatePolyString(pFont, std::wstring(), (float)m_BasePos.x, (float)m_BasePos.y);
		m_pString->SetDropShadow(true);
		m_pString->SetColor(0xFF2020FF);
		m_pString->SetCharScreenHeight(m_nFontSize);
	}

	pFont = g_pInterfaceResMgr->GetFontForHeight((int)(m_nFontSize * xs));
	m_pString->SetFont(pFont);
	m_pString->SetCharScreenHeight((int)(m_nFontSize * xs));
	m_pString->SetPosition((int)(m_BasePos.x * xs), (int)(g_pInterfaceResMgr->GetYRatio() * m_BasePos.y));

	// RETAIL: string 3820 (0xeec), the version as "%d.%d.%d"
	m_pString->SetText(FormatTempString(3820, major, minor, newVersion));
}

void CHUDVersion::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);
}

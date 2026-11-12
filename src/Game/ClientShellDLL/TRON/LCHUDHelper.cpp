// ----------------------------------------------------------------------- //
//
// MODULE  : LCHUDHelper.cpp
//
// PURPOSE : Light cycle HUD layout values ("HUDLayout<n>" in layout.txt)
//
// RETAIL: Tron 2.0's, written from retail CShell.dll 0x10040600..0x10042070
// (see LCHUDHelper.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LCHUDHelper.h"
#include "TronLayoutMgr.h"

#define LO_HUD_TAG					"HUDLayout"

// RETAIL: this file's own buffers (retail 0x1026db84, 0x1026db20)
static char s_aTagName[100];
static char s_aAttName[100];

LCHUDHelper::~LCHUDHelper()
{
}

LCHUDHelper::LCHUDHelper()
{
}

void LCHUDHelper::Init()
{
	int nLayout = 0;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	while (g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName))
	{
		int nIcon = 0;
		sprintf(s_aAttName, "%s%d", "PowerupIconTexture", nIcon);

		StringArray textures;
		while (g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			char szTexture[256];
			g_pTronLayoutMgr->GetString(s_aTagName, s_aAttName, szTexture, sizeof(szTexture));
			textures.push_back(szTexture);

			nIcon++;
			sprintf(s_aAttName, "%s%d", "PowerupIconTexture", nIcon);
		}

		m_PowerupIconTextures.push_back(textures);

		nLayout++;
		sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	}
}

LTIntPt LCHUDHelper::GetPowerupIconPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "PowerupIconPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "PowerupIconPos");
}

const std::string& LCHUDHelper::GetPowerupIconTexture(int nLayout, int nIndex)
{
	static std::string s_sEmpty("");

	if (nLayout >= 0 && nLayout < (int)m_PowerupIconTextures.size())
	{
		StringArray& textures = m_PowerupIconTextures[nLayout];
		if (nIndex >= 0 && nIndex < (int)textures.size())
			return textures[nIndex];
	}

	return s_sEmpty;
}

LTIntPt LCHUDHelper::GetLivesIconPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "LivesIconPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "LivesIconPos");
}

int LCHUDHelper::GetLivesTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "LivesTextOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "LivesTextOffset", 0);
}

int LCHUDHelper::GetLivesTextHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "LivesTextHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "LivesTextHeight", 28);
}

LTIntPt LCHUDHelper::GetRosterPosition(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterPosition"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "RosterPosition");
}

int LCHUDHelper::GetRosterTextHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterTextHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "RosterTextHeight", 20);
}

int LCHUDHelper::GetRosterTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterTextOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "RosterTextOffset", 0);
}

int LCHUDHelper::GetRosterLineOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterLineOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "RosterLineOffset", 0);
}

int LCHUDHelper::GetRosterEnableIconScaling(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterEnableIconScaling"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "RosterEnableIconScaling", 0);
}

LTIntPt LCHUDHelper::GetRosterIconBaseSizeIfScalingIsEnabled(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "RosterIconBaseSizeIfScalingIsEnabled"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "RosterIconBaseSizeIfScalingIsEnabled");
}

LTIntPt LCHUDHelper::GetIntroMasterPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroMasterPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "IntroMasterPos");
}

int LCHUDHelper::GetIntroVFrameBorder(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroVFrameBorder"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroVFrameBorder", 0);
}

int LCHUDHelper::GetIntroHFrameBorder(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroHFrameBorder"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroHFrameBorder", 0);
}

int LCHUDHelper::GetIntroTitleHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroTitleHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroTitleHeight", 0);
}

int LCHUDHelper::GetIntroEntryHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroEntryHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroEntryHeight", 0);
}

int LCHUDHelper::GetIntroTitleSpace(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroTitleSpace"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroTitleSpace", 0);
}

int LCHUDHelper::GetIntroEntrySpace(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroEntrySpace"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroEntrySpace", 0);
}

int LCHUDHelper::GetIntroHeaderOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroHeaderOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroHeaderOffset", 0);
}

int LCHUDHelper::GetIntroValueOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroValueOffset"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "IntroValueOffset", 0);
}

float LCHUDHelper::GetIntroValueTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroValueTime"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetFloat(s_aTagName, "IntroValueTime", 3.0f);
}

std::string LCHUDHelper::GetIntroTexturedFrameName(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "IntroTexturedFrameName"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	char szName[128];
	g_pTronLayoutMgr->GetString(s_aTagName, "IntroTexturedFrameName", szName, sizeof(szName));
	return std::string(szName);
}

LTIntPt LCHUDHelper::GetSpeedometerPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerPos"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetPoint(s_aTagName, "SpeedometerPos");
}

int LCHUDHelper::GetSpeedometerFontNum(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerFontNum"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerFontNum", 0);
}

int LCHUDHelper::GetSpeedometerTextHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerTextHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerTextHeight", 0);
}

int LCHUDHelper::GetSpeedometerVBorder(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerVBorder"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerVBorder", 0);
}

int LCHUDHelper::GetSpeedometerHBorder(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerHBorder"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerHBorder", 0);
}

int LCHUDHelper::GetSpeedometerFrameWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerFrameWidth"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerFrameWidth", 0);
}

int LCHUDHelper::GetSpeedometerFrameHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerFrameHeight"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return g_pTronLayoutMgr->GetInt(s_aTagName, "SpeedometerFrameHeight", 0);
}

std::string LCHUDHelper::GetSpeedometerTexturedFrameName(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!g_pTronLayoutMgr->m_buteMgr.Exist(s_aTagName, "SpeedometerTexturedFrameName"))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	char szName[128];
	g_pTronLayoutMgr->GetString(s_aTagName, "SpeedometerTexturedFrameName", szName, sizeof(szName));
	return szName;
}


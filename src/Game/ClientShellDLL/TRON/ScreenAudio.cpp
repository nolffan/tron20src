// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.cpp
//
// PURPOSE : Interface screen for setting audio options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenAudio.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "ProfileMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"

// RETAIL: this TU is 0x10054d50..0x100560c3 in Tron's CShell.dll, written
// from the retail code. Tron's string IDs are not in our ClientRes headers:
// the numbers are retail's.

namespace
{
	int kGap = 0;
	int kWidth = 0;

	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenAudio *pThisScreen = (CScreenAudio *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_AUDIO);
		if (pThisScreen)
		{
			pThisScreen->SendCommand(CMD_CONFIRM,bReturn,(uint32)pData);
		}
	}
}

// RETAIL: Tron's music volume range and step (0x1026e288, 0x1026e278,
// 0x1026e268)
VarTrack g_vtMusicVolumeMin;
VarTrack g_vtMusicVolumeMax;
VarTrack g_vtMusicVolumeSliderIncrement;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenAudio::CScreenAudio()
{
	m_nSoundVolume=SOUND_DEFAULT_VOL;
	m_nMusicVolume=MUSIC_DEFAULT_VOL;
	m_nSpeechVolume = SPEECH_DEFAULT_VOL;
    m_bSoundQuality=LTFALSE;
    m_bMusicQuality=LTFALSE;
    m_bMusicEnabled=LTFALSE;
	m_bEAX=LTFALSE;

    m_pSoundVolumeCtrl=LTNULL;
    m_pSpeechVolumeCtrl=LTNULL;
    m_pMusicVolumeCtrl=LTNULL;
    m_pSoundQualityCtrl=LTNULL;
    m_pMusicQualityCtrl=LTNULL;
	m_pEAXCtrl=LTNULL;
}

CScreenAudio::~CScreenAudio()
{
}

// Build the screen
LTBOOL CScreenAudio::Build()
{
	if (!g_vtMusicVolumeMin.IsInitted())
		g_vtMusicVolumeMin.Init(g_pLTClient, "MusicVolumeMin", NULL, -3000.0f);
	if (!g_vtMusicVolumeMax.IsInitted())
		g_vtMusicVolumeMax.Init(g_pLTClient, "MusicVolumeMax", NULL, 1000.0f);
	if (!g_vtMusicVolumeSliderIncrement.IsInitted())
		g_vtMusicVolumeSliderIncrement.Init(g_pLTClient, "MusicVolumeSliderIncrement", NULL, 200.0f);

	CreateTitle(0xc);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_AUDIO,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_AUDIO,"SliderWidth");

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	m_bMusicEnabled = (dwAdvancedOptions & AO_MUSIC);

	// RETAIL: Tron's volume sliders step by 5
	m_pSoundVolumeCtrl=AddSlider(0x156, 0xb5, kGap, kWidth, -1, &m_nSoundVolume);
	m_pSoundVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSoundVolumeCtrl->SetSliderRange(SOUND_MIN_VOL, SOUND_MAX_VOL);
	m_pSoundVolumeCtrl->SetSliderIncrement(5);

	m_pSpeechVolumeCtrl=AddSlider(0x1d3, 0x1d4, kGap, kWidth, -1, &m_nSpeechVolume);
	m_pSpeechVolumeCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );
	m_pSpeechVolumeCtrl->SetSliderRange(SPEECH_MIN_VOL, SPEECH_MAX_VOL);
	m_pSpeechVolumeCtrl->SetSliderIncrement(5);

	m_pSoundQualityCtrl=AddToggle(0x157, 0xb6, kGap, &m_bSoundQuality);
	m_pSoundQualityCtrl->SetOnString(LoadTempWString(0x120));
	m_pSoundQualityCtrl->SetOffString(LoadTempWString(0x121));
	m_pSoundQualityCtrl->Enable( (dwAdvancedOptions & AO_SOUND) );

	m_pMusicVolumeCtrl = AddSlider(0x154, 0xb8, kGap, kWidth, -1, &m_nMusicVolume);
	m_pMusicVolumeCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );
	m_pMusicVolumeCtrl->SetSliderRange((int)g_vtMusicVolumeMin.GetFloat(), (int)g_vtMusicVolumeMax.GetFloat());
	m_pMusicVolumeCtrl->SetSliderIncrement((int)g_vtMusicVolumeSliderIncrement.GetFloat());

	m_pMusicQualityCtrl=AddToggle(0x222, 0x223, kGap, &m_bMusicQuality);
	m_pMusicQualityCtrl->SetOnString(LoadTempWString(0x15c));
	m_pMusicQualityCtrl->SetOffString(LoadTempWString(0x30));
	m_pMusicQualityCtrl->Enable( (dwAdvancedOptions & AO_MUSIC) );

	// RETAIL: Tron reads the toggle's position and header width but does
	// not use them (docs/RETAIL_BUGS.md).
	LTIntPt eaxPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID, "EAXTogglePos");
	int nEAXHeaderWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID, "EAXToggleHeaderWidth");
	m_pEAXCtrl=AddToggle(0x9fe, 0x9ff, kGap, &m_bEAX);
	m_pEAXCtrl->SetOnString(LoadTempWString(0x35));
	m_pEAXCtrl->SetOffString(LoadTempWString(0x36));
	m_pEAXCtrl->Enable( (dwAdvancedOptions & AO_SOUNDFILTERS) && (dwAdvancedOptions & AO_SOUND) );

	// The EAX logo
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID, "EAXImagePos");
	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID, "EAXImageRect");
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: on the centred page
	g_pDrawPrim->SetXYWH(&m_EAXPoly, (g_pInterfaceResMgr->GetXRatio() * pos.x + (float)g_pInterfaceResMgr->GetXOffset()), (g_pInterfaceResMgr->GetXRatio() * pos.y + (float)g_pInterfaceResMgr->GetYOffset()),
		nWidth * g_pInterfaceResMgr->GetXRatio(), nHeight * g_pInterfaceResMgr->GetXRatio());
#else
	g_pDrawPrim->SetXYWH(&m_EAXPoly, g_pInterfaceResMgr->GetXRatio() * pos.x, g_pInterfaceResMgr->GetXRatio() * pos.y,
		nWidth * g_pInterfaceResMgr->GetXRatio(), nHeight * g_pInterfaceResMgr->GetXRatio());
#endif
	g_pDrawPrim->SetUVWH(&m_EAXPoly, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_EAXPoly, 0xFFFFFFFF);

	char szTex[256];
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID, "EAXImageTexture", szTex, sizeof(szTex));
	m_hEAXTexture = g_pInterfaceResMgr->GetTexture(szTex);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenAudio::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_CONFIRM)
	{
		if (dwParam2 == 0x224)
		{
			m_bMusicQuality = (dwParam1 > 0);
			m_pMusicQualityCtrl->UpdateData(LTFALSE);
		}
		else if (dwParam2 == 0x225)
		{
			m_bSoundQuality = (dwParam1 > 0);
			m_pSoundQualityCtrl->UpdateData(LTFALSE);
		}
		return 1;
	}
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Change in focus
void CScreenAudio::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetSound();

		m_nSoundVolume = pProfile->m_nSoundVolume;
		m_nMusicVolume = pProfile->m_nMusicVolume;
		m_nSpeechVolume = (int)(100.0f * pProfile->m_fSpeechSoundMultiplier);
		m_bSoundQuality = pProfile->m_bSoundQuality;

		LTBOOL bSoundOn = (LTBOOL)GetConsoleInt("soundenable",1);
        m_pSoundVolumeCtrl->Enable(bSoundOn);
		m_pSpeechVolumeCtrl->Enable(bSoundOn);
        m_pSoundQualityCtrl->Enable(bSoundOn);

		m_bMusicQuality = pProfile->m_bMusicQuality;

		uint32 dwEAX = g_pInterfaceMgr->GetAdvancedOptions() & AO_SOUNDFILTERS;
		m_pEAXCtrl->Enable(dwEAX && bSoundOn);
		if (dwEAX)
			m_bEAX = pProfile->m_bEAXEnabled;
		else
			m_bEAX = LTFALSE;

		if (!GetConsoleInt("MusicActive",0))
			m_nMusicVolume = (int)g_vtMusicVolumeMin.GetFloat();

		m_pMusicVolumeCtrl->Enable(m_bMusicEnabled);
		m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());

		m_nPerformance = g_pPerformanceMgr->GetPerformanceCfg(false);

	    UpdateData(LTFALSE);
	}
	else
	{
		SaveSoundSettings();
		pProfile->Save();
	}
	CBaseScreen::OnFocus(bFocus);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenAudio::Render
//
//	PURPOSE:	Draw the screen and the EAX logo
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenAudio::Render(HSURFACE hDestSurf)
{
	LTBOOL bResult = CBaseScreen::Render(hDestSurf);

	if (m_hEAXTexture)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_DECAL);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);

		g_pDrawPrim->SetTexture(m_hEAXTexture);
		g_pDrawPrim->DrawPrim(&m_EAXPoly, 1);
	}

	return bResult;
}

void CScreenAudio::ConfirmQualityChange(bool bMusic)
{
	UpdateData(LTTRUE);
	if (bMusic)
	{
		if (m_bMusicQuality && (m_nPerformance < 2))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = (void *)0x224;
			g_pInterfaceMgr->ShowMessageBox(0x224,&mb,0,LTFALSE);
		}
	}
	else
	{
		if (m_bSoundQuality && (m_nPerformance == 0))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = AreYouSureCallBack;
			mb.pData = (void *)0x225;
			g_pInterfaceMgr->ShowMessageBox(0x225,&mb,0,LTFALSE);
		}
	}
}

void CScreenAudio::SaveSoundSettings()
{
    UpdateData(LTTRUE);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nSoundVolume = m_nSoundVolume;
	pProfile->m_nMusicVolume = m_nMusicVolume;
	WriteConsoleInt("MusicActive", (m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat()) && m_bMusicEnabled);
	pProfile->m_fSpeechSoundMultiplier = m_nSpeechVolume * 0.01f;
	pProfile->m_bSoundQuality = m_bSoundQuality;
	pProfile->m_bMusicQuality = m_bMusicQuality;
	pProfile->m_bEAXEnabled = m_bEAX;

	pProfile->ApplySound();
}

LTBOOL CScreenAudio::OnLeft()
{
    LTBOOL handled = LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl)
		handled = pCtrl->OnLeft();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		PlayInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnRight()
{
    LTBOOL handled = CBaseScreen::OnRight();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		PlayInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnEnter()
{
    LTBOOL handled = CBaseScreen::OnEnter();
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		PlayInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnLButtonUp(int x, int y)
{
    LTBOOL handled = CBaseScreen::OnLButtonUp(x, y);
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		PlayInterfaceSound(IS_SELECT);
	}
	return handled;
}

LTBOOL CScreenAudio::OnRButtonUp(int x, int y)
{
    LTBOOL handled = CBaseScreen::OnRButtonUp(x, y);
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (handled)
	{
		if (pCtrl == m_pMusicVolumeCtrl)
		{
			UpdateData(LTTRUE);
			m_pMusicQualityCtrl->Enable(m_bMusicEnabled && m_nMusicVolume > (int)g_vtMusicVolumeMin.GetFloat());
			SaveSoundSettings();
		}
		else if (pCtrl == m_pMusicQualityCtrl)
			ConfirmQualityChange(true);
		else if (pCtrl == m_pSoundQualityCtrl)
			ConfirmQualityChange(false);
		else
			SaveSoundSettings();
		PlayInterfaceSound(IS_SELECT);
	}
	return handled;
}

void CScreenAudio::ScreenDimsChanged()
{
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID, "EAXImagePos");
	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID, "EAXImageRect");
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
#ifdef TRON20SRC_MODERN
	// MODERN.md CL-30: on the centred page
	g_pDrawPrim->SetXYWH(&m_EAXPoly, (g_pInterfaceResMgr->GetXRatio() * pos.x + (float)g_pInterfaceResMgr->GetXOffset()), (g_pInterfaceResMgr->GetXRatio() * pos.y + (float)g_pInterfaceResMgr->GetYOffset()),
		nWidth * g_pInterfaceResMgr->GetXRatio(), nHeight * g_pInterfaceResMgr->GetXRatio());
#else
	g_pDrawPrim->SetXYWH(&m_EAXPoly, g_pInterfaceResMgr->GetXRatio() * pos.x, g_pInterfaceResMgr->GetXRatio() * pos.y,
		nWidth * g_pInterfaceResMgr->GetXRatio(), nHeight * g_pInterfaceResMgr->GetXRatio());
#endif
	g_pDrawPrim->SetUVWH(&m_EAXPoly, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_EAXPoly, 0xFFFFFFFF);

	CBaseScreen::ScreenDimsChanged();
}

// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSecurityCamera.cpp
//
// PURPOSE : HUDItem drawn over the view from a security camera
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1003a6b0..
// 0x1003acea; HUDSecurityCamera.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initialisers are in this TU
#include "TronInterfaceMgr.h"
#include "HUDSecurityCamera.h"
#include "ClientSoundMgr.h"

// RETAIL: 0x1003a6b0
CHUDSecurityCamera::CHUDSecurityCamera()
{
	// RETAIL: Tron's update flag for it
	m_UpdateFlags = (eHUDUpdateFlag)0x800000;
	m_fScale = 0.0f;
	m_eLevel = kHUDRenderText;
	m_hSound = LTNULL;
}

// RETAIL: 0x1003a760
LTBOOL CHUDSecurityCamera::Init()
{
	m_nTronUnknown68 = 0;
	m_hOverlay = g_pInterfaceResMgr->GetTexture("Interface\\hud\\SecurityCam_overlay.dtx");
	g_pDrawPrim->SetUVWH(&m_Overlay, 0.0f, 0.0f, 1.0f, 1.0f);
	g_pDrawPrim->SetRGBA(&m_Overlay, 0xFFFFFFFF);
	g_pDrawPrim->SetRGBA(&m_Bands[0], 0xFF000000);
	g_pDrawPrim->SetRGBA(&m_Bands[1], 0xFF000000);
	g_pDrawPrim->SetRGBA(&m_Tint, 0xFFFFFFFF);

	UpdateLayout();

	m_bShow = false;

	return LTTRUE;
}

// RETAIL: 0x1003a810
void CHUDSecurityCamera::Term()
{
	if (m_hSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = LTNULL;
	}
}

// RETAIL: 0x1003a850
void CHUDSecurityCamera::Render()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
	}

	bool bWasShown = m_bShow;
	if (!IsEnabled()) return;

	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->IsDisplayingProgress())
	{
		if (m_hSound)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hSound);
			m_hSound = LTNULL;
		}
		m_bShow = false;
		return;
	}

	m_bShow = true;
	if (!m_hSound)
	{
		// PORT: retail passes one more argument (1.0f) to PlaySoundLocal
		m_hSound = g_pClientSoundMgr->PlaySoundLocal("snd\\amb\\tvmontAmb.wav", SOUNDPRIORITY_PLAYER_HIGH,
			PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT, 100, 1.0f, 0);
	}

	float fDelta = g_pLTClient->GetFrameTime();
	m_fFlickerTime -= fDelta;
	if (m_fFlickerTime < 0.0f || !bWasShown)
	{
		StartFlicker();
	}

	// the band rolls down the screen, and wraps
	float fMove = fDelta * m_fBandSpeed;
	m_fBandStart += fMove;
	if (m_fBandStart > m_fBottom)
		m_fBandStart -= m_fHeight;
	m_fBandEnd += fMove;
	if (m_fBandEnd > m_fBottom)
		m_fBandEnd -= m_fHeight;

	if (m_fBandStart > m_fBandEnd)
	{
		g_pDrawPrim->SetXYWH(&m_Bands[0], m_fLeft, m_fTop, m_fWidth, m_fBandEnd - m_fTop);
		g_pDrawPrim->SetXYWH(&m_Bands[1], m_fLeft, m_fBandStart, m_fWidth, m_fBottom - m_fBandStart);
	}
	else
	{
		g_pDrawPrim->SetXYWH(&m_Bands[0], m_fLeft, m_fBandStart, m_fWidth, m_fBandEnd - m_fBandStart);
	}

	SetRenderState();

	g_pDrawPrim->DrawPrim(&m_Tint, 1);
	if (m_fBandStart > m_fBandEnd)
		g_pDrawPrim->DrawPrim(m_Bands, 2);
	else
		g_pDrawPrim->DrawPrim(m_Bands, 1);

	g_pDrawPrim->SetTexture(m_hOverlay);
	g_pDrawPrim->DrawPrim(&m_Overlay, 1);
}

// RETAIL: 0x1003aae0: the overlay at a random alpha for a random time
void CHUDSecurityCamera::StartFlicker()
{
	m_fFlickerTime = GetRandom(m_fMinFlickerTime, m_fMaxFlickerTime);
	uint8 nAlpha = (uint8)(GetRandom(m_fMinAlpha, m_fMaxAlpha) * 255.0f);
	g_pDrawPrim->SetALPHA(&m_Overlay, nAlpha);
}

// RETAIL: 0x1003ab30
void CHUDSecurityCamera::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
	}
}

// RETAIL: 0x1003ab50
void CHUDSecurityCamera::UpdateLayout()
{
	m_fScale = g_pInterfaceResMgr->GetXRatio();
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	// PORT: retail keeps the height in two locals (near: their stores)
	float fHeight = ys * 480.0f;
	m_fTop = 0.0f;
	m_fLeft = 0.0f;
	m_fBottom = fHeight;
	m_fHeight = fHeight;
	float fWidth = xs * 640.0f;
	m_fWidth = fWidth;

	g_pDrawPrim->SetXYWH(&m_Overlay, 0.0f, 0.0f, fWidth, fHeight);
	g_pDrawPrim->SetXYWH(&m_Tint, m_fLeft, m_fTop, m_fWidth, m_fHeight);

	m_fMinFlickerTime = 0.01f;
	m_fMaxFlickerTime = 0.1f;
	m_fBandHeight = ys * 100.0f;
	m_fMinAlpha = 0.2f;
	m_fMaxAlpha = 0.5f;
	m_fBandSpeed = ys * 30.0f;

	m_fBandStart = GetRandom(m_fTop, m_fBottom);
	m_fBandEnd = m_fBandStart + m_fBandHeight;
	if (m_fBandEnd > m_fBottom)
		m_fBandEnd -= m_fHeight;

	g_pDrawPrim->SetRGBA(&m_Bands[0], 0x30000000);
	g_pDrawPrim->SetRGBA(&m_Bands[1], 0x30000000);
	g_pDrawPrim->SetRGBA(&m_Tint, 0x608080D0);

	g_pDrawPrim->SetXYWH(&m_Overlay, 0.0f, 0.0f, fWidth, fHeight);
}

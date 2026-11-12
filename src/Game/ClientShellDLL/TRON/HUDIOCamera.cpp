// ----------------------------------------------------------------------- //
//
// MODULE  : HUDIOCamera.cpp
//
// PURPOSE : HUDItem drawn over the view from an I/O node's camera
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1002b470..
// 0x1002c180; HUDIOCamera.h).
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TronInterfaceMgr.h"
#include "HUDIOCamera.h"

CHUDIOCamera::CHUDIOCamera()
{
	// RETAIL: Tron's update flag for it, drawn at every render level
	m_UpdateFlags = (eHUDUpdateFlag)0x40000;
	m_fScale = 0.0f;
	m_eLevel = kHUDRenderNone;
}

LTBOOL CHUDIOCamera::Init()
{
	m_hPiece1 = g_pInterfaceResMgr->GetTexture("Interface\\hud\\IONode_hud.dtx");
	m_rcRect.Init(0, 0, 200, 150);

	UpdateLayout();

	for (int i = 0; i < 4; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Piece1[i], 0x88FFFFFF);
	}

	// the four quarters, mirrored about the center
	SetupQuadUVs(m_Piece1[0], m_hPiece1, 0.0f, 0.0f, 1.0f, 1.0f);
	SetupQuadUVs(m_Piece1[1], m_hPiece1, 1.0f, 0.0f, -1.0f, 1.0f);
	SetupQuadUVs(m_Piece1[2], m_hPiece1, 1.0f, 1.0f, -1.0f, -1.0f);
	SetupQuadUVs(m_Piece1[3], m_hPiece1, 0.0f, 1.0f, 1.0f, -1.0f);

	SetupQuadUVs(m_Piece2[0], m_hPiece2, 0.0f, 0.0f, 1.0f, 1.0f);
	SetupQuadUVs(m_Piece2[1], m_hPiece2, 1.0f, 0.0f, -1.0f, 1.0f);
	SetupQuadUVs(m_Piece2[2], m_hPiece2, 1.0f, 1.0f, -1.0f, -1.0f);
	SetupQuadUVs(m_Piece2[3], m_hPiece2, 0.0f, 1.0f, 1.0f, -1.0f);

	m_bShow = false;

	return LTTRUE;
}

void CHUDIOCamera::Render()
{
	if (!IsEnabled()) return;

	bool bWasShown = m_bShow;
	if (!((CTronInterfaceMgr*)g_pInterfaceMgr)->GetTronUnknown28AE8())
	{
		m_bShow = false;
		return;
	}

	m_bShow = true;
	if (!bWasShown)
	{
		m_fNextFlicker = GetRandom(m_fPiece2MinInterval, m_fPiece2MaxInterval);
	}

	float fTime = g_pLTClient->GetTime();
	float fDelta = fTime - m_fLastTime;
	m_fLastTime = fTime;

	float fAlpha = (float)sin(fTime * m_fPiece1Freq) * m_fPiece1AlphaRange + m_fPiece1AlphaMid;
	if (fAlpha < 0.0f)
		fAlpha = 0.0f;
	else if (fAlpha > 1.0f)
		fAlpha = 1.0f;

	uint8 nAlpha = (uint8)(fAlpha * 255.0f);
	for (int i = 0; i < 4; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Piece1[i], SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
	}

	if (m_fNextFlicker > 0.0f)
	{
		m_fNextFlicker -= fDelta;
		if (m_fNextFlicker < 0.0f)
		{
			m_nFlickers = GetRandom(m_nPiece2MinFlickers, m_nPiece2MaxFlickers);
			m_fFlickerTime = m_fPiece2FlickerTime;
			StartFlicker();
		}
	}
	else if (m_fFlickerTime > 0.0f)
	{
		m_fFlickerTime -= fDelta;
		if (m_fFlickerTime < 0.0f)
		{
			if (--m_nFlickers)
			{
				StartFlicker();
				m_fFlickerTime = m_fPiece2FlickerTime;
			}
			else
			{
				m_fNextFlicker = GetRandom(m_fPiece2MinInterval, m_fPiece2MaxInterval);
			}
		}
	}

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hPiece1);
	g_pDrawPrim->DrawPrim(m_Piece1, 4);

	if (m_fFlickerTime > 0.0f)
	{
		g_pDrawPrim->SetTexture(m_hPiece2);
		g_pDrawPrim->DrawPrim(m_Piece2, 4);
	}
}

// RETAIL: 0x1002bb10: the second ring at a random size and alpha
void CHUDIOCamera::StartFlicker()
{
	float fRadius = GetRandom(m_fPiece2MinRadius, m_fPiece2MaxRadius);

	uint8 nAlpha = (uint8)(GetRandom(m_fPiece2MinAlpha, m_fPiece2MaxAlpha) * 255.0f);
	for (int i = 0; i < 4; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Piece2[i], SET_ARGB(nAlpha, 0xFF, 0xFF, 0xFF));
	}

	g_pDrawPrim->SetXYWH(&m_Piece2[0], m_fCenterX, m_fCenterY - fRadius, fRadius, fRadius);
	g_pDrawPrim->SetXYWH(&m_Piece2[1], m_fCenterX - fRadius, m_fCenterY - fRadius, fRadius, fRadius);
	g_pDrawPrim->SetXYWH(&m_Piece2[2], m_fCenterX - fRadius, m_fCenterY, fRadius, fRadius);
	g_pDrawPrim->SetXYWH(&m_Piece2[3], m_fCenterX, m_fCenterY, fRadius, fRadius);
}

void CHUDIOCamera::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
}

void CHUDIOCamera::UpdateLayout()
{
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	char szTmp[1024];
	g_pLayoutMgr->GetString("IONodeCam", "Piece1Tex", szTmp, sizeof(szTmp));
	m_hPiece1 = g_pInterfaceResMgr->GetTexture(szTmp);
	g_pLayoutMgr->GetString("IONodeCam", "Piece2Tex", szTmp, sizeof(szTmp));
	m_hPiece2 = g_pInterfaceResMgr->GetTexture(szTmp);

	float fMinAlpha = g_pLayoutMgr->GetFloat("IONodeCam", "Piece1MinAlpha", 0.3f);
	float fMaxAlpha = g_pLayoutMgr->GetFloat("IONodeCam", "Piece1MaxAlpha", 0.9f);
	m_fPiece1AlphaMid = (fMaxAlpha + fMinAlpha) * 0.5f;
	m_fPiece1AlphaRange = fMaxAlpha - m_fPiece1AlphaMid;
	m_fPiece1Period = g_pLayoutMgr->GetFloat("IONodeCam", "Piece1Period", 3.0f);
	m_fPiece1Freq = MATH_CIRCLE / m_fPiece1Period;
	m_fPiece1Radius = g_pLayoutMgr->GetFloat("IONodeCam", "Piece1Radius", 256.0f) * xs;

	m_rcRect = g_pLayoutMgr->GetRect("Miscellaneous", "IOCameraRect");
	m_rcRect.left = (int)(m_rcRect.left * xs);
	m_rcRect.right = (int)(m_rcRect.right * xs);
	m_rcRect.top = (int)(m_rcRect.top * ys);
	m_rcRect.bottom = (int)(m_rcRect.bottom * ys);

	LTIntPt ptCenter = g_pLayoutMgr->GetPoint("IONodeCam", "Center");
	m_fCenterX = (float)ptCenter.x * xs;
	m_fCenterY = (float)ptCenter.y * ys;

	m_fPiece2MinInterval = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MinInterval", 3.0f);
	m_fPiece2MaxInterval = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MaxInterval", 3.0f);
	m_fPiece2MinRadius = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MinRadius", 80.0f) * xs;
	m_fPiece2MaxRadius = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MaxRadius", 240.0f) * ys;
	m_fPiece2MinAlpha = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MinAlpha", 0.2f);
	m_fPiece2MaxAlpha = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2MaxAlpha", 0.6f);
	m_nPiece2MinFlickers = g_pLayoutMgr->GetInt("IONodeCam", "Piece2MinFlickers", 2);
	m_nPiece2MaxFlickers = g_pLayoutMgr->GetInt("IONodeCam", "Piece2MaxFlickers", 8);
	m_fPiece2FlickerTime = g_pLayoutMgr->GetFloat("IONodeCam", "Piece2FlickerTime", 0.1f);

	g_pDrawPrim->SetXYWH(&m_Piece1[0], m_fCenterX, m_fCenterY - m_fPiece1Radius, m_fPiece1Radius, m_fPiece1Radius);
	g_pDrawPrim->SetXYWH(&m_Piece1[1], m_fCenterX - m_fPiece1Radius, m_fCenterY - m_fPiece1Radius, m_fPiece1Radius, m_fPiece1Radius);
	g_pDrawPrim->SetXYWH(&m_Piece1[2], m_fCenterX - m_fPiece1Radius, m_fCenterY, m_fPiece1Radius, m_fPiece1Radius);
	g_pDrawPrim->SetXYWH(&m_Piece1[3], m_fCenterX, m_fCenterY, m_fPiece1Radius, m_fPiece1Radius);
}

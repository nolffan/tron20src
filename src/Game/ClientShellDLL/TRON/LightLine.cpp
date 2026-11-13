// ----------------------------------------------------------------------- //
//
// MODULE  : LightLine.cpp
//
// PURPOSE : A thin line drawn on a menu screen with glints running along it
//
// RETAIL: Tron's, written from retail CShell.dll 0x100d7bc0..0x100d8460
// (the names are ours)
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightLine.h"
#include "InterfaceMgr.h"

// RETAIL: 0x100d7bc0
CLightGlintBase::CLightGlintBase()
{
	m_dwTronUnknown0 = m_dwTronUnknown4 = 0;
	m_fX1 = m_fX2 = m_fY1 = m_fY2 = 0.0f;
	m_dwTronUnknownB8 = m_dwTronUnknownBC = m_dwTronUnknownC0 = 0;
}

CLightGlintBase::~CLightGlintBase()
{
}

// RETAIL: 0x100d7c10
CLightGlint::CLightGlint()
{
}

// RETAIL: 0x100d7c80: fades the glint in and out over its run
void CLightGlint::Render()
{
	m_fTime -= g_pLTClient->GetFrameTime();
	if (m_fTime < 0.0f)
		Reset();

	float fRatio = m_fTime / m_fPeriod;
	uint8 a;
	if (fRatio > 0.5f)
		a = (uint8)(((1.0f - fRatio) + (1.0f - fRatio)) * 96.0f);
	else
		a = (uint8)((fRatio + fRatio) * 96.0f);

	uint32 nColor = SET_ARGB(0xFF,a,a,a);

	if (m_bVertical)
	{
		g_pDrawPrim->SetRGBA4(&m_Poly[0],0xFF000000,0xFF000000,nColor,nColor);
		g_pDrawPrim->SetRGBA4(&m_Poly[1],nColor,nColor,0xFF000000,0xFF000000);
	}
	else
	{
		g_pDrawPrim->SetRGBA4(&m_Poly[0],0xFF000000,nColor,nColor,0xFF000000);
		g_pDrawPrim->SetRGBA4(&m_Poly[1],nColor,0xFF000000,0xFF000000,nColor);
	}

	g_pDrawPrim->DrawPrim(m_Poly,2);
}

// RETAIL: 0x100d7da0: starts the next run at a random point on the line
void CLightGlint::Reset()
{
	m_fTime += m_fPeriod;
	if (m_fTime < 0.0f)
		m_fTime = GetRandom(0.0f,m_fPeriod);

	float fPos;
	m_fPos = fPos = GetRandom(0.0f,1.0f);

	float dx = m_fX2 - m_fX1;
	float dy = m_fY2 - m_fY1;
	float x = m_fX1 + dx * fPos;
	float y = m_fY1 + dy * fPos;

	if (dx < 0.0f) dx = -dx;
	if (dy < 0.0f) dy = -dy;

	if (dx > dy)
	{
		m_bVertical = false;
		g_pDrawPrim->SetXYWH(&m_Poly[0],x - 6.0f,y,6.0f,2.0f);
		g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,6.0f,2.0f);
	}
	else
	{
		m_bVertical = true;
		g_pDrawPrim->SetXYWH(&m_Poly[0],x,y - 6.0f,2.0f,6.0f);
		g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,2.0f,6.0f);
	}
}

// RETAIL: 0x100d7f20
void CLightGlint::Init(float fX1, float fY1, float fX2, float fY2)
{
	m_fX1 = fX1;
	m_fX2 = fX2;
	m_fY1 = fY1;
	m_fY2 = fY2;
	m_fPeriod = 1.0f;
	Reset();
	m_fTime = GetRandom(0.0f,m_fPeriod);
}

// RETAIL: 0x100d7f70: keeps the glint where it is on the moved line
void CLightGlint::Move(float fX1, float fY1, float fX2, float fY2)
{
	float dx = fX2 - fX1;
	float dy = fY2 - fY1;

	m_fX1 = fX1;
	m_fX2 = fX2;
	m_fY1 = fY1;
	m_fY2 = fY2;

	float x = fX1 + dx * m_fPos;
	float y = fY1 + dy * m_fPos;

	if (dx < 0.0f) dx = -dx;
	if (dy < 0.0f) dy = -dy;

	if (dx > dy)
	{
		m_bVertical = false;
		g_pDrawPrim->SetXYWH(&m_Poly[0],x - 6.0f,y,6.0f,2.0f);
		g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,6.0f,2.0f);
	}
	else
	{
		m_bVertical = true;
		g_pDrawPrim->SetXYWH(&m_Poly[0],x,y - 6.0f,2.0f,6.0f);
		g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,2.0f,6.0f);
	}
}


// RETAIL: 0x100d80c0
CLightLine::CLightLine()
{
	m_pGlints = LTNULL;
	m_nGlints = 0;
}

// RETAIL: 0x100d80f0: one glint per ten pixels of line
void CLightLine::Create(float fX1, float fY1, float fX2, float fY2)
{
	if (m_nGlints)
		Term();

	float dx = fX2 - fX1;
	float dy = fY2 - fY1;
	m_nGlints = (int)((float)sqrt(dx*dx + dy*dy) * 0.1f);
	if (m_nGlints < 1)
		m_nGlints = 1;

	m_pGlints = debug_newa(CLightGlint,m_nGlints);

	// RETAIL: the time is read and not used
	float fTime = g_pLTClient->GetTime();

	for (int i = 0; i < m_nGlints; i++)
	{
		m_pGlints[i].Init(fX1,fY1,fX2,fY2);
	}

	g_pDrawPrim->SetRGBA(&m_Poly,0x800000C0);
	if (dx > dy)
		g_pDrawPrim->SetXYWH(&m_Poly,fX1,fY1,dx + 2.0f,2.0f);
	else
		g_pDrawPrim->SetXYWH(&m_Poly,fX1,fY1,2.0f,dy + 2.0f);
}

// RETAIL: 0x100d8210
void CLightLine::Move(float fX1, float fY1, float fX2, float fY2)
{
	for (int i = 0; i < m_nGlints; i++)
	{
		m_pGlints[i].Move(fX1,fY1,fX2,fY2);
	}

	float dx = fX2 - fX1;
	float dy = fY2 - fY1;
	if (dx > dy)
		g_pDrawPrim->SetXYWH(&m_Poly,fX1,fY1,dx + 2.0f,2.0f);
	else
		g_pDrawPrim->SetXYWH(&m_Poly,fX1,fY1,2.0f,dy + 2.0f);
}

// RETAIL: 0x100d82d0
void CLightLine::Term()
{
	if (m_pGlints)
	{
		debug_deletea(m_pGlints);
		m_pGlints = LTNULL;
		m_nGlints = 0;
	}
}

// RETAIL: 0x100d8310
void CLightLine::SetColor(uint32 nColor)
{
	g_pDrawPrim->SetRGBA(&m_Poly,nColor);
}

// RETAIL: 0x100d8330
void CLightLine::Render()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_ADD);

	g_pDrawPrim->DrawPrim(&m_Poly,1);

	if (m_nGlints)
	{
		for (int i = 0; i < m_nGlints; i++)
		{
			m_pGlints[i].Render();
		}
	}
}

// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.cpp
//
// PURPOSE : Implementation of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "InterfaceTimer.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "InterfaceResMgr.h"

static void BuildTimeString(char* aBuffer, int nTime);

CInterfaceTimer::CInterfaceTimer() 
{
	m_fTime		= 0.0f;
	m_bPause	= LTFALSE;
	m_pTimeStr	= LTNULL;
	m_nBaseSize = 0;
}

CInterfaceTimer::~CInterfaceTimer() 
{ 
	if (m_pTimeStr) 
		g_pFontManager->DestroyPolyString(m_pTimeStr); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceTimer::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CInterfaceTimer::Draw()
{
	// RETAIL: Tron's (retail 0x10170230): the time left is counted from the
	// game time on every draw, paused or not.
	float fTimeLeft = m_fTime - g_pLTClient->TronUnknownVirtual44( );	// TRON: the game clock (iltclient.h)

	// Update/Draw the timer if there is anytime left...
	if( fTimeLeft <= 0.0f )
		return;

	// Draw the string to the surface...
	int nMinutes = int(fTimeLeft) / 60;
	int nSeconds = fTimeLeft > 60.0 ? int(fTimeLeft) % 60 : int(fTimeLeft);

	char aBuffer[8];
	char aMinutes[8];
	char aSeconds[8];
	BuildTimeString(aMinutes, nMinutes);
	BuildTimeString(aSeconds, nSeconds);
	sprintf(aBuffer, "%s:%s", aMinutes, aSeconds);

	uint8 nSize = (uint8)((LTFLOAT)m_nBaseSize * g_pInterfaceResMgr->GetXRatio());
	std::wstring sText;
	LTGUIWiden(sText, aBuffer);

 	if (!m_pTimeStr)
 	{
		// RETAIL: the font is read but not used (the font is the one for
		// the height)
		g_pLayoutMgr->GetInt("Miscellaneous","TimerFont");
		m_nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","TimerSize");
		if (m_nBaseSize == 0)
			m_nBaseSize = 24;

		m_BasePos = g_pLayoutMgr->GetPoint("Miscellaneous","TimerPos");
		if (m_BasePos.x == 0)
			m_BasePos = LTIntPt(320,40);

		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
		m_pTimeStr = g_pFontManager->CreateFormattedPolyString(pFont, sText, 0.0f, 0.0f, CUI_HALIGN_LEFT);
		m_pTimeStr->SetAlignmentH(CUI_HALIGN_CENTER);
 	}
 	else
 	{
		m_pTimeStr->SetText(sText);
 	}

	m_pTimeStr->SetCharScreenHeight(nSize);

	// Position the text
	int x = (int)(g_pInterfaceResMgr->GetXRatio() * m_BasePos.x);
	int y = (int)(g_pInterfaceResMgr->GetYRatio() * m_BasePos.y);

	m_pTimeStr->SetPosition( x+2, y+2);
	m_pTimeStr->SetColor(argbBlack);
	m_pTimeStr->Render();

	m_pTimeStr->SetPosition( x, y );
	m_pTimeStr->SetColor(argbWhite);
	m_pTimeStr->Render();
}

static void BuildTimeString(char* aBuffer, int nTime)
{
	if (nTime > 9)
	{
		sprintf(aBuffer, "%d", nTime);
	}
	else
	{
		sprintf(aBuffer, "0%d", nTime);
	}
}
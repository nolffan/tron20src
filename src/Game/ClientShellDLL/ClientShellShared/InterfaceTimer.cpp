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

	// RETAIL: reconstructed from the retail 1.3 CShell.dll (0x100eb260): the
	// time left is counted from the game time on every draw, paused or not.
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

	// RETAIL: Tron's (0x10170230): the size comes first, and the font is
	// the one for that size ("TimerFont" is read but not used)
	uint8 nSize = (uint8)((LTFLOAT)m_nBaseSize * g_pInterfaceResMgr->GetXRatio());
	// RETAIL: Tron's widens the text inline (0x101702fc)
	std::wstring sText;
	LTGUIWiden(sText, aBuffer);

 	if (!m_pTimeStr)
 	{
		g_pLayoutMgr->GetInt("Miscellaneous","TimerFont");
		m_nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","TimerSize");
		if (m_nBaseSize == 0) 
			m_nBaseSize = 24;

		m_BasePos = g_pLayoutMgr->GetPoint("Miscellaneous","TimerPos");
		if (m_BasePos.x == 0)
			m_BasePos = LTIntPt(320,40);

		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
		m_pTimeStr = g_pFontManager->CreateFormattedPolyString(pFont, sText);
		m_pTimeStr->SetAlignmentH(CUI_HALIGN_CENTER);
 	}
 	else
 	{
		m_pTimeStr->SetText(sText);
 	}

	m_pTimeStr->SetCharScreenHeight(nSize);

	// Position the text
	int x = (int)((float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio());
	int y = (int)((float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio());

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
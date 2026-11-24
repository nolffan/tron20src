/****************************************************************************
;
;	 MODULE:		Credits (.CPP)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;					07/26/00 [jrg] modified for NOLF
;
;	Copyright (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "GameClientShell.h"
#include "Credits.h"
#include <stdio.h>
#include <mbstring.h>

// Macros...



LTFLOAT		CCredits::s_fSpeed = DEF_CREDITS_SPEED;


namespace
{
	LTBOOL		s_bPause = LTFALSE;
	LTFLOAT		s_fFadeInTime = -1.0f;
	LTFLOAT		s_fHoldTime = -1.0f;
	LTFLOAT		s_fFadeOutTime = -1.0f;
	LTFLOAT		s_fDelayTime = -1.0f;
	LTIntPt		s_PositionUL;
	LTIntPt		s_PositionUR;
	LTIntPt		s_PositionLL;
	LTIntPt		s_PositionLR;
	HMODULE s_hModule = NULL;
	HRSRC   s_hRes    = NULL;
	HGLOBAL	s_hGlobal = NULL;
	char*	s_sBuf    = LTNULL;
}

static char* GetTextBuffer(char* sName);

// RETAIL: inline (CCredit::SetState, 0x1014622c): the image's texture
// coordinates, inset half a texel. The name is ours.
static inline void SetHalfTexelUVs(LT_POLYFT4* pPrim, HTEXTURE hTex)
{
	float fU = 0.0f;
	float fV = 0.0f;
	uint32 nWidth, nHeight;
	if (g_pTexInterface->GetTextureDims(hTex, nWidth, nHeight) == LT_OK)
	{
		fU = 0.5f / (float)nWidth;
		fV = 0.5f / (float)nHeight;
	}
	pPrim->verts[0].u = fU;
	pPrim->verts[0].v = fV;
	pPrim->verts[1].u = 1.0f - fU;
	pPrim->verts[1].v = fV;
	pPrim->verts[2].u = 1.0f - fU;
	pPrim->verts[2].v = 1.0f - fV;
	pPrim->verts[3].u = fU;
	pPrim->verts[3].v = 1.0f - fV;
}

// RETAIL: inline (CCredit::AddString, 0x10145bd0, 0x10145ca8): a wide
// string's characters into a cleared char buffer. The name is ours.
static inline void NarrowCopy(char* pDest, const std::wstring& sText, uint32 nSize)
{
	memset(pDest, 0, nSize);
	uint32 i = 0;
	while (i < sText.size() && i < nSize)
	{
		pDest[i] = (char)sText[i];
		i++;
	}
}


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::Init(const std::wstring& sBuf)
{
	// Set simple members...

	m_nState    = CS_START;

	s_bPause    = LTFALSE;

	m_Pos.x		= 320;
	m_Pos.y		= 240;
	m_ePosition	= CP_CENTER;
	m_bBig		= LTFALSE;
	m_hAlign = CUI_HALIGN_CENTER;

	m_szImage[0] = '\0';
	m_hImage = LTNULL;

	// Parse the text buffer and add each line as a separate string...

	std::wstring sString;

	std::wstring::const_iterator iter = sBuf.begin();
	while (iter != sBuf.end() && *iter)
	{
		if ((*iter == '\n') || (*iter == '\r'))
		{
			sString.push_back(L'\0');
			AddString(sString);
			sString.clear();

			do
			{
				iter++;
			} while (((*iter == '\n') || (*iter == '\r')));
		}
		else
		{
			sString.push_back(*iter);
			iter++;
		}
	}
	sString.push_back(L'\0');
	if (sString.size() > 0) AddString(sString);

	// All done...
	FormatStrings();

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CCredit::Term()
{
	for (uint16 i = 0; i < m_Strings.size(); i++)
	{
		g_pFontManager->DestroyPolyString(m_Strings[i]);
	}
	m_Strings.clear();

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::AddString
//
//	PURPOSE:	Adds a new string
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::AddString(const std::wstring& sString)
{
	// Sanity checks...

	if (!g_pLTClient) return(LTFALSE);

	// Check if this is a special command string...

	if (wcsncmp(sString.c_str(), L">TIME:", 6) == 0)
	{
		if (sString.size() > 6)
		{
			// RETAIL: the terminator goes one past the copied characters
			char szTime[16];
			NarrowCopy(szTime, std::wstring(&sString.c_str()[6]), sizeof(szTime));
			((char*)szTime)[sizeof(szTime)] = '\0';
			m_fHoldTime = (float)atof(szTime);
			return(LTTRUE);
		}
	}
	if (wcsncmp(sString.c_str(), L">IMAGE:", 7) == 0)
	{
		if (sString.size() > 7 && !m_szImage[0])
		{
			NarrowCopy(m_szImage, std::wstring(&sString.c_str()[7]), sizeof(m_szImage));
			// RETAIL: this clears m_szImage[128], the low byte of m_hImage,
			// one past the buffer (a retail bug; not yet listed: the function is not identical)
			((char*)m_szImage)[sizeof(m_szImage)] = '\0';
			return(LTTRUE);
		}
	}
	if (wcsncmp(sString.c_str(), L">POS:UL", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_LEFT;
		m_Pos = s_PositionUL;
		m_ePosition = CP_UL;
		return(LTTRUE);
	}
	if (wcsncmp(sString.c_str(), L">POS:UR", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_RIGHT;
		m_Pos = s_PositionUR;
		m_ePosition = CP_UR;
		return(LTTRUE);
	}
	if (wcsncmp(sString.c_str(), L">POS:LR", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_RIGHT;
		m_Pos = s_PositionLR;
		m_ePosition = CP_LR;
		return(LTTRUE);
	}
	if (wcsncmp(sString.c_str(), L">POS:LL", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_LEFT;
		m_Pos = s_PositionLL;
		m_ePosition = CP_LL;
		return(LTTRUE);
	}
	if (wcsncmp(sString.c_str(), L">BIG", 4) == 0)
	{
		m_bBig = LTTRUE;
		return(LTTRUE);
	}

	uint8 nFont = 0;
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	CUIFormattedPolyString* pStr = g_pFontManager->CreateFormattedPolyString(pFont,sString,(float)m_Pos.x,(float)m_Pos.y);
	if (pStr)
	{
		m_Strings.push_back(pStr);
		pStr->SetDropShadow(true);
	}

	return(LTTRUE);
}

void CCredit::FormatStrings()
{
	int x = (int)(g_pInterfaceResMgr->GetXRatio() * m_Pos.x);
	int y = (int)(g_pInterfaceResMgr->GetYRatio() * m_Pos.y);
	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		CUIFormattedPolyString* pStr = *iter;

		uint8 nSize = (uint8)(16.0f * g_pInterfaceResMgr->GetXRatio());
		if (m_bBig)
			nSize = (uint8)(24.0f * g_pInterfaceResMgr->GetYRatio());

		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);
		pStr->SetFont(pFont);
		pStr->SetCharScreenHeight(nSize);
		pStr->SetColor(argbWhite);

		// TRON: a centred string is placed by its width
		if (m_hAlign == CUI_HALIGN_CENTER)
		{
			pStr->TronUnknownVirtual30(CUI_HALIGN_LEFT);
			int32 nWidth, nHeight;
			pStr->GetDims(&nWidth,&nHeight);
			pStr->SetPosition(x - nWidth / 2,y);
		}
		else
		{
			pStr->TronUnknownVirtual30(m_hAlign);
			pStr->SetPosition(x,y);
		}
		y += pStr->GetHeight();

		iter++;

	}

	int offset = y - (int)(g_pInterfaceResMgr->GetYRatio() * m_Pos.y);
	switch (m_ePosition)
	{
	case CP_CENTER:
		offset /= 2;
		break;
	case CP_UR:
	case CP_UL:
		offset = 0;
		break;
	}

	if (offset > 0)
	{
		FPStringArray::iterator iter = m_Strings.begin();
		while (iter != m_Strings.end())
		{
			CUIFormattedPolyString* pStr = *iter;

			int32 x,y;
			pStr->GetPosition(&x,&y);

			y -= offset;

			pStr->SetPosition(x,y);

			iter++;

		}

	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Draw
//
//	PURPOSE:	Draws all the strings of the credit
//
// ----------------------------------------------------------------------- //

void CCredit::Draw()
{
	// TRON: the image first
	if (m_hImage)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		g_pDrawPrim->SetTexture(m_hImage);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	// Sanity checks...

	if (m_Strings.size() <= 0) return;

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->Render();
		iter++;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Update
//
//	PURPOSE:	Updates the credit
//
//	RETURNS:	LTFALSE = Stay with current credit
//				LTTRUE  = Advance to next credit
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::Update()
{
	// Sanity checks...

	if (!g_pLTClient) return(LTFALSE);


	// Update based on our current state...

	switch(m_nState)
	{
		case CS_START:
		{
			SetState(CS_FADEIN);
			return(LTFALSE);
		}

		case CS_FADEIN:
		{
			if (UpdateFadeIn())	SetState(CS_HOLDIN);
			return(LTFALSE);
		}

		case CS_HOLDIN:
		{
			if (UpdateHoldIn()) SetState(CS_FADEOUT);
			return(LTFALSE);
		}

		case CS_FADEOUT:
		{
			if (UpdateFadeOut()) SetState(CS_HOLDOUT);
			return(LTFALSE);
		}

		case CS_HOLDOUT:
		{
			if (UpdateHoldOut()) SetState(CS_DONE);
			return(LTFALSE);
		}

		case CS_DONE:
		{
			return(LTTRUE);
		}

		default:
		{
			return(LTTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::SetState
//
//	PURPOSE:	Sets the update state
//
// ----------------------------------------------------------------------- //

void CCredit::SetState(int nState)
{
	// Set the new state...

	m_nState = nState;

	switch (nState)
	{
		case CS_START:
		{
			// TRON: load the image and centre it on the screen
			if (m_szImage[0])
			{
				m_hImage = LTNULL;
				if (g_pTexInterface->FindTextureFromName(m_hImage, m_szImage) != LT_OK)
				{
					g_pTexInterface->CreateTextureFromName(m_hImage, m_szImage);
				}
			}

			if (m_hImage)
			{
				uint32 nWidth, nHeight;
				g_pTexInterface->GetTextureDims(m_hImage, nWidth, nHeight);

				float fX = g_pInterfaceResMgr->GetXRatio() * 320.0f - (float)nWidth * 0.5f;
				float fY = g_pInterfaceResMgr->GetYRatio() * 240.0f - (float)nHeight * 0.5f;

				// Half a texel in from the edges
				SetHalfTexelUVs(&m_Poly, m_hImage);

				g_pDrawPrim->SetXYWH(&m_Poly, fX, fY, (float)nWidth, (float)nHeight);
			}
			break;
		}

		case CS_FADEIN:
		{
			m_fTimer = s_fFadeInTime;
			m_fAlpha = 0.0f;
			break;
		}

		case CS_HOLDIN:
		{
			m_fTimer = s_fHoldTime;
			m_fAlpha = 1.0f;
			if (m_fHoldTime > 0.0f) m_fTimer = m_fHoldTime;
			break;
		}

		case CS_FADEOUT:
		{
			m_fTimer = s_fFadeOutTime;
			m_fAlpha = 1.0f;
			break;
		}

		case CS_HOLDOUT:
		{
			m_fTimer = s_fDelayTime;
			m_fAlpha = 1.0f;
			break;
		}

		case CS_DONE:
		{
			if (m_hImage)
			{
				g_pTexInterface->ReleaseTextureHandle(m_hImage);
				m_hImage = LTNULL;
			}
			break;
		}
	}

	m_fTimerStart = m_fTimer;
	m_fAlphaStart = m_fAlpha;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateFadeIn
//
//	PURPOSE:	Updates the CS_FADEIN state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateFadeIn()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0.0f;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// set the alpha...

	m_fAlpha = 1.0f - (m_fTimer / m_fTimerStart);
	if (m_fAlpha < 0.0f) m_fAlpha = 0.0f;
	if (m_fAlpha > 1.0f) m_fAlpha = 1.0f;

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->SetColor(color);
		iter++;
	}
	g_pDrawPrim->SetRGBA(&m_Poly, color);
	

	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateHoldIn
//
//	PURPOSE:	Updates the CS_HOLDIN state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateHoldIn()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->SetColor(argbWhite);
		iter++;
	}
	g_pDrawPrim->SetRGBA(&m_Poly, argbWhite);


	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateFadeOut
//
//	PURPOSE:	Updates the CS_FADEOUT state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateFadeOut()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0.0f;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// set the alpha...

	m_fAlpha = (m_fTimer / m_fTimerStart);
	if (m_fAlpha < 0.0f) m_fAlpha = 0.0f;
	if (m_fAlpha > 1.0f) m_fAlpha = 1.0f;

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->SetColor(color);
		iter++;
	}
	g_pDrawPrim->SetRGBA(&m_Poly, color);

	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateHoldOut
//
//	PURPOSE:	Updates the CS_HOLDOUT state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateHoldOut()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// All done...

	return(m_fTimer == 0.0f);
}


void CCredit::Clear()
{
	m_nState    = CS_START;
	m_fTimer    = 0.0f;
	m_fHoldTime = -1.0f;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

LTBOOL CCredits::Init(int nMode, LTBOOL bClearScreen)
{
	// Sanity checks...


	Term();


	// Set simple members...

	m_iCredit      = 0;
	m_nMode        = nMode;
	m_bDone        = LTFALSE;
	m_bClearScreen = bClearScreen;
	s_fSpeed       = DEF_CREDITS_SPEED;

	if (s_fFadeInTime < 0.0f)
	{
		s_fFadeInTime = g_pLayoutMgr->GetCreditsFadeInTime();
		s_fHoldTime = g_pLayoutMgr->GetCreditsHoldTime();
		s_fFadeOutTime = g_pLayoutMgr->GetCreditsFadeOutTime();
		s_fDelayTime = g_pLayoutMgr->GetCreditsDelayTime();
		s_PositionUL = g_pLayoutMgr->GetCreditsPositionUL();
		s_PositionUR = g_pLayoutMgr->GetCreditsPositionUR();
		s_PositionLL = g_pLayoutMgr->GetCreditsPositionLL();
		s_PositionLR = g_pLayoutMgr->GetCreditsPositionLR();
	}


	// Add all the credit objects...

	AddCredits();


	// All done...

	if (m_Credits.size())
		m_bInited = LTTRUE;

	return(LTTRUE);
}

void CCredits::Clear()
{

	m_bInited      = LTFALSE;
	m_bDone        = LTTRUE;

	m_iCredit  = 0;
	m_nMode    = CM_CREDITS;

	s_fSpeed = DEF_CREDITS_SPEED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CCredits::Term()
{
	// Term all the credits...

	CreditArray::iterator iter = m_Credits.begin();
	while (iter != m_Credits.end())
	{
		
		(*iter)->Term();
		debug_delete( (*iter) );
		iter++;

	}

	m_Credits.clear();


	// Clear all members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::HandleInput
//
//	PURPOSE:	Handler for input
//
// ----------------------------------------------------------------------- //

void CCredits::HandleInput(int vkey)
{
	// Handle various keys...

	switch (vkey)
	{
		case VK_UP:
		{
			if (IsIntro()) break;
			IncSpeed();
			break;
		}

		case VK_DOWN:
		{
			if (IsIntro()) break;
			DecSpeed();
			break;
		}

		case VK_HOME:
		{
			if (IsIntro()) break;
			s_fSpeed = 1.0;
			break;
		}

		case VK_RETURN:
		case VK_SPACE:
		case VK_NEXT:
		{
			if (IsIntro() || IsDemoIntro())
			{
				CCredit* pCredit = GetCredit(m_iCredit);
				if (pCredit)
				{
					if (pCredit->GetState() == CS_HOLDIN)
					{
						pCredit->SetState(CS_FADEOUT);
					}
				}
			}
			else
			{
				AdvanceCredit(CS_HOLDIN);
			}
			break;
		}

		case VK_PRIOR:
		{
			if (IsIntro() || IsDemoIntro()) break;
			BackupCredit(CS_HOLDIN);
			break;
		}

		case VK_PAUSE:
		{
			if (IsIntro() || IsDemoIntro()) break;
			s_bPause ^= 1;
			break;
		}

		case VK_ESCAPE:
		{
//			ExitToMainMenu();
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::ExitToMainMenu
//
//	PURPOSE:	Exits back to the main menu game state
//
// ----------------------------------------------------------------------- //

//void CCredits::ExitToMainMenu()
//{
//	m_bDone = LTTRUE;
//}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Update
//
//	PURPOSE:	Updates the credits
//
// ----------------------------------------------------------------------- //

void CCredits::Update()
{
	// Sanity checks...

	if (!g_pLTClient) return;
	if (m_Credits.size() <= 0) return;


	// Clear the screen...

	if (m_bClearScreen)
	{
		g_pLTClient->ClearScreen(NULL, CLEARSCREEN_SCREEN);
	}



	// Update the current credit...

	CCredit* pCredit = GetCredit(m_iCredit);
	if (!pCredit) return;

	if (pCredit->Update())
	{
		AdvanceCredit();
	}
}

void CCredits::AdvanceCredit(int nState)
{
	m_iCredit++;
	if(m_iCredit >= m_Credits.size())
	{
		m_iCredit = 0;
		m_bDone = LTTRUE;
	}
	CCredit* pCredit = GetCredit(m_iCredit);
	if (pCredit) pCredit->SetState(nState);
}

void CCredits::BackupCredit(int nState)
{
	m_iCredit--;
	if(m_iCredit < 0)
		m_iCredit = 0;

	CCredit* pCredit = GetCredit(m_iCredit);
	if (pCredit) pCredit->SetState(nState);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::AddCredits
//
//	PURPOSE:	Adds all the credit objects
//
// ----------------------------------------------------------------------- //

void CCredits::AddCredits()
{
	// Sanity checks...

	if (!g_pLTClient) return;


	// Get the credits text buffer...

	char* sName = NULL;

	switch (GetMode())
	{
		case CM_INTRO:		sName = "INTRO"; break;
		case CM_CREDITS:	sName = "CREDITS"; break;
		case CM_DEMO_INFO:	sName = "DEMOINFO"; break;
		case CM_DEMO_INTRO:	sName = "DEMOINTRO"; break;
		case CM_DEMO_MULTI:	sName = "DEMOMULTI"; break;

		default: sName = "CREDITS";
	}

	// TRON: the text is UTF-16
	std::wstring sBuf;
	wchar_t* pBuf = (wchar_t*)GetTextBuffer(sName);
	if (pBuf) sBuf = pBuf;

	// Skip the byte order mark
	std::wstring::iterator iter = sBuf.begin();
	if (iter != sBuf.end()) iter++;


	// Parse the credits text...

	std::wstring sCredit;

	while (iter != sBuf.end())
	{

		if (*iter == '#' && *(iter+1) == '#')
		{
			// RETAIL: the terminator is added before the check, so ">END"
			// never matches (a retail bug; not yet listed: the function is not identical)
			sCredit.push_back(L'\0');

			if (sCredit == L">END")	// end?
			{
				return;
			}

			AddCredit(sCredit);
			sCredit.clear();

			iter+=2;

			while (*iter != '\0' && ((*iter == '\n') || (*iter == '\r'))) iter++;
		}
		else
		{
			sCredit.push_back(*iter);
			iter++;
		}
	}
}

void CCredits::AddCredit(const std::wstring& sText)
{
	// Sanity checks...

	if (!g_pLTClient) return;


	// Add the credit...

	CCredit* pCredit = debug_new(CCredit);
	if (!pCredit->Init(sText))
	{
		debug_delete(pCredit);
		return;
	}

	// Inc our credit counter...

	m_Credits.push_back(pCredit);
}


CCredit* CCredits::GetCredit(uint16 iCredit)
{
	if (iCredit >= m_Credits.size()) return(NULL);
	if (iCredit < 0) return(NULL);
	return(m_Credits[iCredit]);
}



// Functions...

char* GetTextBuffer(char* sName)
{
	//if (s_sBuf)
	//{
		//return(s_sBuf);
	//}
	//else
	{
		void* hModule;
		g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
		s_hModule = (HINSTANCE)hModule;
		if (!s_hModule)	return(NULL);

		s_hRes = FindResource(s_hModule, sName, "TEXT");
		if (!s_hRes) return(NULL);

		s_hGlobal = LoadResource(s_hModule, s_hRes);
		if (!s_hGlobal) return(NULL);

		s_sBuf = (char*)LockResource(s_hGlobal);
		if (!s_sBuf) return(NULL);
		
		return(s_sBuf);
	}
}

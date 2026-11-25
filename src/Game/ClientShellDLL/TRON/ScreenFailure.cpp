// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.cpp
//
// PURPOSE : Interface screen for setting crosshair options
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenFailure.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "TRONInterfaceMgr.h"
#include "SaveLoadMgr.h"
#include "GameClientShell.h"
#include "iltfontmanager.h"
// MERGE: Tron's side (build 69); NOLF2 1.3 had:
//| #include "ClientSaveLoadMgr.h"
//| #include "ClientMultiplayerMgr.h"
//| #include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientMultiplayerMgr.h"	// MERGE: NOLF2's, which its code here uses
#include "MissionMgr.h"


extern ILTFontManager* g_pFontManager;

namespace
{
	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 10.0f;
	LTRect	stringRect(20,240,620,400);
	uint8	stringSize = 24;
	LTIntPt helpPos(160,440);
	uint8	helpSize = 12;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x1005abd0
CScreenFailure::CScreenFailure()
{
	m_pString = LTNULL;
}

CScreenFailure::~CScreenFailure()
{

}

// Build the screen
// RETAIL: 0x1005ac20
LTBOOL CScreenFailure::Build()
{

	CreateTitle(NULL);

	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"FailStringRect"))
	{
		stringRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_FAILURE,"FailStringRect");
	}
	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"FailStringSize"))
	{
		stringSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_FAILURE,"FailStringSize");
	}
	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"HelpStringPos"))
	{
		helpPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_FAILURE,"HelpStringPos");
	}
	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"HelpStringSize"))
	{
		helpSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_FAILURE,"HelpStringSize");
	}

	// RETAIL: the failure text is a polystring, aligned left (0x1005adc4
	// pushes CUI_HALIGN_LEFT); OnFocus centres it by its width. Centred
	// here too, it was drawn off the screen ("END OF LINE." was not seen)
	int nHeight = (int)(g_pInterfaceResMgr->GetYRatio() * (float)stringSize);
	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(nHeight);
	m_pString = g_pFontManager->CreateFormattedPolyString(pFont, std::wstring(L" "), 0.0f, 0.0f, CUI_HALIGN_LEFT);
	m_pString->SetCharScreenHeight(nHeight);
	m_pString->SetColor(0xFFFFFFFF);

	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"Delay"))
	{
		g_fDelay = g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_FAILURE,"Delay");
	}

	if( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		CLTGUITextCtrl *pHelp = AddTextItem(IDS_HELP_FAILURE,NULL,NULL,helpPos,LTTRUE);
		if (pHelp)
		{
			CUIFont* pHelpFont = g_pInterfaceResMgr->GetFontForHeight(helpSize);
			pHelp->SetFont(pHelpFont,helpSize);
		}
	}

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE,LTFALSE,LTFALSE);

	return LTTRUE;

}

// RETAIL: 0x1005af00
void CScreenFailure::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		g_fDuration = 0.0f;

		m_pString->SetText(LoadTempWString(g_pInterfaceMgr->GetFailStringID()));

		// RETAIL: both coordinates are scaled by the x ratio
		float fXRatio = g_pInterfaceResMgr->GetXRatio();
		int x = (int)((float)((stringRect.left + stringRect.right) / 2) * fXRatio);
		int y = (int)((float)stringRect.top * fXRatio);

		int32 nWidth, nHeight;
		m_pString->GetDims(&nWidth, &nHeight);
		m_pString->SetPosition(x - nWidth / 2, y);
	}

	CBaseScreen::OnFocus(bFocus);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ExitFailureScreenCB
//
//  PURPOSE:	Callback for the Exit failure mission screen messagebox...
//
// ----------------------------------------------------------------------- //

static void ExitFailureScreenCB(LTBOOL bReturn, void *pData)
{
	// If they confirm, then go back.
	if (bReturn)
	{
		CScreenMgr* pScreenMgr = g_pInterfaceMgr->GetScreenMgr( );

		//hack to rebuild our screen history
		pScreenMgr->ClearHistory();
		pScreenMgr->AddScreenToHistory( SCREEN_ID_MAIN );
		pScreenMgr->AddScreenToHistory( SCREEN_ID_MULTI );

		g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
		g_pInterfaceMgr->GetScreenMgr( )->PreviousScreen( );
	}
}



void CScreenFailure::Escape()
{
	// RETAIL: 0x1005b020.  A client of a remote server is asked first.
	if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		MBCreate mb;
		mb.eType = LTMB_YESNO;
		mb.pFn = ExitFailureScreenCB;
		mb.pData = this;
		g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
		return;
	}

	//hack to rebuild our screen history
	m_pScreenMgr->ClearHistory();
	PlayInterfaceSound(IS_PAGE);
	m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);

	// Put mp history if mp game.
	if( IsMultiplayerGame( ))
	{
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_HOST);
	}
	// Put sp history if sp game.
	else
	{
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
	}

	OnFocus(LTFALSE);
	m_pScreenMgr->SetCurrentScreen(SCREEN_ID_LOAD);
}

LTBOOL CScreenFailure::HandleKeyDown(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenFailure::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenFailure::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenFailure::Render(HSURFACE hDestSurf)
{
	// RETAIL: 0x1005b170
	CBaseScreen::Render(hDestSurf);

	if (m_pString)
	{
		m_pString->Render();
	}

	g_fDuration += g_pGameClientShell->GetFrameTime();

	// Automatically trigger an escape if we're a local client.
	if( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && g_fDuration >= g_fDelay )
	{
		Escape();
	}

	return LTTRUE;
}





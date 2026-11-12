// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.cpp
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// RETAIL: Tron's (CShell.dll 0x1006fb90..0x1006fe20), written from the
// retail code. Its string, command and screen IDs are Tron's numbers (its
// ScreenCommands and ScreenEnum differ from NOLF2's).
#include "stdafx.h"
#include "ScreenMulti.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientSaveLoadMgr.h"
#include "InterfaceMgr.h"

namespace
{
	// RETAIL: Tron's command IDs
	const uint32 kCmdPlayer		= 0x1D;
	const uint32 kCmdHost		= 0x1E;
	const uint32 kCmdJoin		= 0x1F;
	const uint32 kCmdJoinLAN	= 0x20;

	// RETAIL: Tron's screen IDs
	const eScreenID kScreenPlayer	= (eScreenID)0x08;
	const eScreenID kScreenJoin		= (eScreenID)0x0A;
	const eScreenID kScreenHost		= (eScreenID)0x1F;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x1006fbb0
CScreenMulti::CScreenMulti()
{
}

// RETAIL: 0x1006fbf0
CScreenMulti::~CScreenMulti()
{
}

// Build the screen
// RETAIL: 0x1006fc00
LTBOOL CScreenMulti::Build()
{
	CreateTitle(0x17);

	AddTextItem(0xE0, kCmdPlayer, 0xE3);
	AddTextItem(0xE1, kCmdJoin, 0xE4);
	AddTextItem(0x257, kCmdJoinLAN, 0x25E);
	AddTextItem(0xE2, kCmdHost, 0xE5);

	// Make sure to call the base class
	return CBaseScreen::Build();
}

// RETAIL: 0x1006fce0. The join screen's +0x798 (Tron's CBaseScreenJoin,
// not ours yet) is whether it searches the internet
uint32 CScreenMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case kCmdPlayer:
		m_pScreenMgr->SetCurrentScreen(kScreenPlayer);
		break;
	case kCmdJoin:
		{
			CBaseScreen *pScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(kScreenJoin);
			if (pScreen)
				*(uint32*)((uint8*)pScreen + 0x798) = 1;
			m_pScreenMgr->SetCurrentScreen(kScreenJoin);
		} break;
	case kCmdJoinLAN:
		{
			CBaseScreen *pScreen = g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(kScreenJoin);
			if (pScreen)
				*(uint32*)((uint8*)pScreen + 0x798) = 0;
			m_pScreenMgr->SetCurrentScreen(kScreenJoin);
		} break;
	case kCmdHost:
		m_pScreenMgr->SetCurrentScreen(kScreenHost);
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

// Change in focus
// RETAIL: 0x1006fdc0
void CScreenMulti::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders(true);
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
}

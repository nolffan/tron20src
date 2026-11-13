// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.cpp
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMain.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "direct.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
// MERGE: Tron's side (build 69); NOLF2 1.3 had:
//| #include "ClientSaveLoadMgr.h"
//| #include "MissionMgr.h"
//| #include "ClientMultiplayerMgr.h"
//| #include "ResShared.h"
//| #include "ScreenMulti.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "TronScreenMgr.h"
#include "ResShared.h"	// MERGE: NOLF2's, which its code here uses
#include "ScreenMulti.h"	// MERGE: NOLF2's, which its code here uses

namespace
{
	// RETAIL: 0x1021a0f0; the name is ours
	bool s_bSkipDisconnect = true;

	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EXIT,0,0);
	};

}

extern bool g_bLAN;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// RETAIL: 0x1006ee60
CScreenMain::CScreenMain()
{
    m_pResume = LTNULL;
}

CScreenMain::~CScreenMain()
{
}


// Build the screen
// RETAIL: 0x1006eeb0
LTBOOL CScreenMain::Build()
{
#ifdef _TRON_E3_DEMO
	BuildCustomLevelsList();

	// Set a console variable based on m_bBackDemo
	WriteConsoleInt("E3BackroomDemo", m_bBackDemo ? 1 : 0);

	if (m_bBackDemo)
	{
		int index = 0;
		StringSet::iterator iter = m_Filenames.begin();
		while (iter != m_Filenames.end())
		{
			AddTextItem((char*)iter->c_str(),CMD_CUSTOM+index,LTNULL);
			++index;
			iter++;
		}
	}
	else
	{
		AddTextItem(IDS_TRON_E3_DEMO, CMD_CUSTOM, IDS_HELP_SINGLEPLAYER);
	}
#else

	CLTGUITextCtrl *pCtrl = AddTextItem(LoadTempWString(0x4B), CMD_SINGLE_PLAYER, 0x8C);
	pCtrl->SetPulse(true);

	m_pResume = AddTextItem(0x7C, CMD_CONTINUE_GAME, 0xD3);
	m_pResume->SetPulse(true);
	m_pResume->Enable(LTFALSE);

	pCtrl = AddTextItem(0x4C, CMD_MULTI_PLAYER, 0x8D);
	pCtrl->SetPulse(true);
	pCtrl = AddTextItem(0x4D, CMD_OPTIONS, 0x8E);
	pCtrl->SetPulse(true);
	pCtrl = AddTextItem(0x6E, CMD_PROFILE, 0x6F);
	pCtrl->SetPulse(true);
	pCtrl = AddTextItem(0x4E, CMD_CREDITS, 0x152);
	pCtrl->SetPulse(true);
	pCtrl = AddTextItem(0x4F, CMD_QUIT, 0x8F);
	pCtrl->SetPulse(true);
#endif

	// Put the version number at versionpos, centered on it.
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"VersionPos");
	uint8 nFont = g_pLayoutMgr->GetHelpFont();
	
	std::string sVersion = g_pVersionMgr->GetVersion() ? g_pVersionMgr->GetVersion() : "";
	sVersion += " - ";
	sVersion += g_pClientMultiplayerMgr->GetModName();

	std::wstring sText;
	LTGUIWiden(sText,sVersion.c_str());
	pCtrl = AddTextItem(sText,LTNULL,LTNULL,pos,LTTRUE,nFont);
	uint8 nSize = g_pLayoutMgr->GetHelpSize();
	pCtrl->SetFont(g_pInterfaceResMgr->GetFontForHeight(nSize),nSize);
	pos.x += (int)((float)pCtrl->GetWidth() / pCtrl->GetScale() * -0.5f);
	pCtrl->SetBasePos(pos);

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);

	return LTTRUE;
}

// RETAIL: 0x1006f300. Tron's disconnects through the multiplayer manager
// and lets Render finish it (s_bSkipDisconnect).
void CScreenMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientMultiplayerMgr->ForceDisconnect();
		}
		s_bSkipDisconnect = true;

		SetSelection(kNoSelection);

		// Initialize to the sp mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
			return;
		}

		// Always assume sp save/load when in the main screen.  This is so the
		// "continue game" and "quick load" assumes sp.
		g_pGameClientShell->SetGameType( eGameTypeSingle );
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
		g_pClientMultiplayerMgr->SetupServerSinglePlayer( );

		// Setup continue game selection.
		if (m_pResume)
			m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );

		// We don't go thru the main screen when joining from command line, so disable it if
		// we get here.
		g_pInterfaceMgr->SetCommandLineJoin( false );
	}

	CBaseScreen::OnFocus(bFocus);
}


// RETAIL: 0x1006f3e0
uint32 CScreenMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
#ifdef _TRON_E3_DEMO
	if (dwCommand >= CMD_CUSTOM)
	{
		int index = dwCommand - CMD_CUSTOM;
		StringSet::iterator iter = m_Filenames.begin();
		while (iter != m_Filenames.end() && index > 0)
		{
			iter++;
			--index;
		}
		if (iter != m_Filenames.end())
		{
			if( g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
				g_pMissionMgr->StartGameFromLevel(iter->c_str());
		}
		return 1;
	}
#endif
	switch(dwCommand)
	{
	case CMD_SINGLE_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SINGLE);
			break;
		}
	case CMD_MULTI_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
			break;
		}
	case CMD_OPTIONS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_OPTIONS);
			break;
		}
	case CMD_PROFILE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PROFILE);
			break;
		}
	case CMD_CREDITS:
		{
			if (g_pInterfaceMgr)
				g_pInterfaceMgr->GetCredits()->Init(CM_CREDITS,LTFALSE);
			break;
		}
	case CMD_QUIT:
		{

			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(0x297,&mb);
			break;
		}
	case CMD_EXIT:
		{
#if(defined(_DEMO) || defined(_TRON_E3_DEMO))
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case CMD_RESUME:
		{
			Escape();
			break;
		}
	case CMD_CONTINUE_GAME:
		{
			// Initialize to the sp mission bute.
			if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
			{
				g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
				return 0;
			}

			// Start the game from the continue save.
			if( !g_pMissionMgr->StartGameFromContinue( ))
				return 0;

			return 1;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Screen specific rendering
// RETAIL: 0x1006f580: the disconnect OnFocus asked for, a frame later
LTBOOL   CScreenMain::Render(HSURFACE hDestSurf)
{
	if (s_bSkipDisconnect)
		s_bSkipDisconnect = false;
	else if (g_pLTClient->IsConnected())
		g_pLTClient->Disconnect();

	return (CBaseScreen::Render(hDestSurf) != LTFALSE);
}

#ifdef _TRON_E3_DEMO
// Build the list of Custom Levels
void CScreenMain::BuildCustomLevelsList()
{
	// MERGE: Tron's side (build 69); NOLF2 1.3 had:
	//| if (m_pGameType->IsVisible())
	m_Filenames.clear();
	m_bBackDemo = false;

	// Get a list of world names and sort them alphabetically

    char pathBuf[128] = "worlds\\TronE3Demo";
	char Buf[255];

	FileEntry* pFilesArray = debug_new(FileEntry*);

	if (pFilesArray)
	{
        // MERGE: Tron's side (build 69); NOLF2 1.3 had:
        //| m_pGameType->SetSelection(kNoSelection);
        //| m_pGameType->Show(LTFALSE);
        //| m_pGameTypeFrame->Show(LTFALSE);
        pFilesArray = g_pLTClient->GetFileList(pathBuf);
		sprintf(Buf, "%s\\", pathBuf);
		AddFilesToFilenames(pFilesArray, Buf);
        g_pLTClient->FreeFileList(pFilesArray);
		debug_delete(pFilesArray);
	}
	if (m_Filenames.size() > 1)
		m_bBackDemo = true;
}

void CScreenMain::AddFilesToFilenames(FileEntry* pFiles, char* pPath)
{
	if (!pFiles || !pPath) return;

	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	FileEntry* ptr = pFiles;

	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
			pBaseName = strtok (strBaseName, ".");
			pBaseExt = strtok (NULL, "\0");
			if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
			{
				// add this to the array
				m_Filenames.insert(pBaseName);
			}
		}

		// MERGE: Tron's side (build 69); NOLF2 1.3 had:
		//| SetSelection(0);
		//| g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		//| return;
		//| }
		//| CBaseScreen::Escape();
		//| }
		ptr = ptr->m_pNext;
	}
}
#endif

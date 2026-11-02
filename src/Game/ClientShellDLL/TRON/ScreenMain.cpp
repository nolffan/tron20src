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

namespace
{
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

CScreenMain::CScreenMain()
{
    m_pResume = LTNULL;
    m_pGameType = LTNULL;
	m_pGameTypeFrame = LTNULL;

}

CScreenMain::~CScreenMain()
{
}


// Build the screen
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

	char szTmp[1024];
	FormatString(IDS_SINGLEPLAYER,szTmp,sizeof(szTmp));

	AddTextItem(szTmp, CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER);

	m_pResume = AddTextItem(IDS_CONTINUE_GAME, CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
	m_pResume->Enable(LTFALSE);

	// MERGE: Tron's side (build 69); NOLF2 1.3 had:
	//| CLTGUITextCtrl* pMP = AddTextItem(IDS_MULTIPLAYER, CMD_MULTI_PLAYER, IDS_HELP_MULTIPLAYER);
	//| CLTGUITextCtrl* pMPLan = AddTextItem(IDS_MULTIPLAYER_LAN, CMD_MULTI_PLAYER_LAN, IDS_HELP_MULTIPLAYER_LAN);
	//| CLTGUITextCtrl* pOptions = AddTextItem(IDS_OPTIONS, CMD_OPTIONS, IDS_HELP_OPTIONS);
	AddTextItem(IDS_MULTIPLAYER, CMD_MULTI_PLAYER, IDS_HELP_MULTIPLAYER);
	AddTextItem(IDS_OPTIONS, CMD_OPTIONS, IDS_HELP_OPTIONS);

	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_PROFILE, CMD_PROFILE, IDS_HELP_PROFILE);

#ifdef _TO2DEMO
	pMP->Enable( LTFALSE );
	pMPLan->Enable( LTFALSE );
#endif

	AddTextItem(IDS_EXIT, CMD_QUIT, IDS_HELP_EXIT, s_BackPos);
#endif

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);

	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"GameTypeRect");
	int nHeight = (rect.bottom - rect.top);
	int nWidth = (rect.right - rect.left);
	LTIntPt pos = LTIntPt(rect.left,rect.top);

	m_pGameTypeFrame = debug_new(CLTGUIFrame);
	m_pGameTypeFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pGameTypeFrame->SetBasePos(pos);
	m_pGameTypeFrame->Show(LTFALSE);
	AddControl(m_pGameTypeFrame);


	nWidth -= 16;
	m_pGameType = AddList(pos,nHeight, LTTRUE, nWidth);
	if (m_pGameType)
	{
		m_pGameType->SetIndent(LTIntPt(8,8));
		m_pGameType->SetFrameWidth(2);
		m_pGameType->Show(LTFALSE);


		CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_COOPERATIVE,	CMD_COOP, 0);
		m_pGameType->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_DEATHMATCH, CMD_DM, 0);
		m_pGameType->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_TEAMDEATHMATCH, CMD_TEAM_DM, 0);
		m_pGameType->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_DOOMSDAY, CMD_DOOM, 0);
		m_pGameType->AddControl(pCtrl);
	}


	// Put the version number at versionpos, but don't let it clip off the edge.
	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"VersionPos");
	uint8 nFont = g_pLayoutMgr->GetHelpFont();
	
	std::string sVersion = g_pVersionMgr->GetVersion();
	sVersion += " - ";
	sVersion += g_pClientMultiplayerMgr->GetModName();
	
	pCtrl= AddTextItem( const_cast<char*>(sVersion.c_str()),LTNULL,LTNULL,pos,LTTRUE,nFont);
	pCtrl->SetFont(LTNULL,g_pLayoutMgr->GetHelpSize());
	uint16 nUnScaledWidth = pCtrl->GetBaseWidth();
	if( pos.x + nUnScaledWidth >= 640 )
	{
		pos.x = 640 - nUnScaledWidth;
	}
	int nUnScaledHeight = pCtrl->GetBaseHeight();
	if( pos.y + nUnScaledHeight > 480 )
	{
		pos.y = 480 - nUnScaledHeight;
	}
	pCtrl->SetBasePos(pos);


	//Build multiplayer screen to force construction of DM mission file
	CScreenMulti *pMulti = (CScreenMulti *)m_pScreenMgr->GetScreenFromID(SCREEN_ID_MULTI);
	pMulti->Build();


	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);

	return LTTRUE;
}

void CScreenMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// MERGE: Tron's side (build 69); NOLF2 1.3 had:
		//| // Make sure we're disconnected from server.
		//| if(g_pLTClient->IsConnected())
		((CTronScreenMgr*)m_pScreenMgr)->PlayMenuMusic();

		SetSelection(-1);

		// Make sure we're always disconnected when we get to this screen.  You must abort to get here.
		if(g_pLTClient->IsConnected())
		{
			// MERGE: Tron's side (build 69); NOLF2 1.3 had:
			//| g_pInterfaceMgr->SetIntentionalDisconnect( true );
			//| g_pClientMultiplayerMgr->ForceDisconnect();
			g_pInterfaceMgr->StartingNewGame( );
			g_pLTClient->Disconnect();
		}
// MERGE: Tron's side (build 69); NOLF2 1.3 had:
//| //since can get here from various points in the game, let's make sure that we're not fading in or out...
//| g_pInterfaceMgr->AbortScreenFade();
//| SetSelection(-1);
//| // Always assume sp save/load when in the main screen.  This is so the
//| // "continue game" and "quick load" assumes sp.
//| g_pGameClientShell->SetGameType( eGameTypeSingle );
//| m_pGameType->Show(LTFALSE);
//| m_pGameType->SetSelection(kNoSelection);
//| m_pGameTypeFrame->Show(LTFALSE);
//| // Initialize to the sp mission bute.
//| if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
//| {
//| g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
//| return;
//| }

		// Setup continue game selection.
			m_pResume->SetHelpID(IDS_HELP_CONTINUE_GAME);
			m_pResume->CLTGUICtrl::Create(CMD_CONTINUE_GAME,0,0);
		m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );
		m_pResume->SetString( LoadTempString( IDS_CONTINUE_GAME ));

		// MERGE: Tron's side (build 69); NOLF2 1.3 had:
		//| g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
		//| g_pClientMultiplayerMgr->SetupServerSinglePlayer( );
		//| m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );
		// Always assume sp save/load when in the main screen.  This is so the 
		// "continue game" assumes sp.
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );

		// We don't go thru the main screen when joining from command line, so disable it if
		// we get here.
		g_pInterfaceMgr->SetCommandLineJoin( false );
	}

	CBaseScreen::OnFocus(bFocus);
}


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

	case CMD_DM:
		{
			char path[256];
			std::string sFN = _getcwd(path,sizeof(path));
			sFN += "\\";
			sFN += MISSION_DM_FILE;

			if (!CWinUtil::FileExist(sFN.c_str()))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_NO_DM_MAPS,&mb);
				return 0;
			}

			g_pGameClientShell->SetGameType( eGameTypeDeathmatch);

			if (!g_bLAN)
				g_pInterfaceResMgr->DrawMessage(IDS_INTERNET);
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_TEAM_DM:
		{
			char path[256];
			std::string sFN = _getcwd(path,sizeof(path));
			sFN += "\\";
			sFN += MISSION_DM_FILE;

			if (!CWinUtil::FileExist(sFN.c_str()))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_NO_DM_MAPS,&mb);
				return 0;
			}

			g_pGameClientShell->SetGameType( eGameTypeTeamDeathmatch );

			if (!g_bLAN)
				g_pInterfaceResMgr->DrawMessage(IDS_INTERNET);
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_COOP:
		{
			g_pGameClientShell->SetGameType(eGameTypeCooperative);
			if (!g_bLAN)
				g_pInterfaceResMgr->DrawMessage(IDS_INTERNET);
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_DOOM:
		{
			char path[256];
			std::string sFN = _getcwd(path,sizeof(path));
			sFN += "\\";
			sFN += MISSION_DD_FILE;

			if (!CWinUtil::FileExist(sFN.c_str()))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_NO_DM_MAPS,&mb);
				return 0;
			}

			g_pGameClientShell->SetGameType( eGameTypeDoomsDay);

			if (!g_bLAN)
				g_pInterfaceResMgr->DrawMessage(IDS_INTERNET);
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_MULTI_PLAYER:
		{
#ifndef _TO2DEMO
			g_bLAN = false;
			m_pGameType->Show(LTTRUE);
			m_pGameTypeFrame->Show(LTTRUE);
			SetSelection(GetIndex(m_pGameType));
#endif
			break;

		}
	case CMD_MULTI_PLAYER_LAN:
		{
#ifndef _TO2DEMO
			g_bLAN = true;
			m_pGameType->Show(LTTRUE);
			m_pGameTypeFrame->Show(LTTRUE);
			SetSelection(GetIndex(m_pGameType));
#endif
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
/*
	case CMD_SUBROUTINES:
		{
			// Temporarily go to the SubroutineMgr to populate the screen.
			// Remember to remove the #include when you take this out.
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SUBROUTINES);
			g_pSubroutineMgr->PopulateSubroutineScreen();
			break;
		}

	case CMD_RATINGS:
		{
			// Temporarily go to the SubroutineMgr to populate the screen.
			// Remember to remove the #include when you take this out.
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_RATINGS);
			g_pSubroutineMgr->PopulateSubroutineScreen();
			break;
		}
*/
	case CMD_QUIT:
		{

			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_SUREWANTQUIT,&mb);
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
				// MERGE: Tron's side (build 69); NOLF2 1.3 had:
				//| g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
				//| return 0;
				//| }
				//| // Start the game from the continue save.
				//| if( !g_pMissionMgr->StartGameFromContinue( ))
				//| return 0;
				//| return 1;
				g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
				return 0;
			}

			if( !g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
				return 0;

			g_pGameClientShell->SetGameType( eGameTypeSingle );

			// Start the game from the continue save.
			if( !g_pMissionMgr->StartGameFromContinue( ))
				return 0;

				return 1;
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Screen specific rendering
LTBOOL   CScreenMain::Render(HSURFACE hDestSurf)
{
	return CBaseScreen::Render(hDestSurf);

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

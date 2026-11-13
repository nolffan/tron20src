// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSave.cpp
//
// PURPOSE : Interface screen for saving games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionButeMgr.h"
#include "ScreenSave.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"


#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = SLMGR_MAX_SAVE_SLOTS;
	int	 g_nSaveSlot = -1;
	int	 g_nSaveIndex = -1;

	// RETAIL: 0x10076fa0
	void OverwriteCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SAVE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OVERWRITE,0,0);
	}
	CLTGUIColumnCtrl* g_pColCtrl = LTNULL;

	// RETAIL: Tron keeps the row's old text wide (0x1026eb50, 0x1026eb40)
	std::wstring g_sOldName;
	std::wstring g_sOldTime;

	int kNameWidth = 0;
	int kTimeWidth = 0;
	int kIndent = 0;
	int kSmallFontSize = 4;

	// RETAIL: 0x10076fd0
	void EditCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SAVE);

		if(pThisScreen)
		{
			if (bReturn)
			{
				pThisScreen->SendCommand(CMD_EDIT_NAME,(uint32)pData,0);
			}
			else
			{
				//the user cancelled we need to restore the original text, which has been saved in the
				//globals above.
				if(g_pColCtrl)
				{
					g_pColCtrl->SetString(1, g_sOldName);
					g_pColCtrl->SetString(2, g_sOldTime);
				}
			}
		}
	};

	// RETAIL: inline: a wide string's low bytes into a cleared buffer (the
	// edit box's text is wide); the name is ours
	inline void NarrowInto(char *pszOut, const std::wstring &sText, uint32 nSize)
	{
		memset(pszOut,0,nSize);
		for (uint32 i = 0; i < sText.length() && i < nSize; i++)
			pszOut[i] = (char)sText[i];
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSave::CScreenSave()
{
}

CScreenSave::~CScreenSave()
{

}

// RETAIL: 0x100771b0
LTBOOL CScreenSave::Build()
{
	CreateTitle(7);

	kNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"NameWidth");
	kTimeWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"TimeWidth");
	kIndent = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"Indent");
	kSmallFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"SmallFontSize");

	return CBaseScreen::Build();
}


void CScreenSave::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelList();
		SetSelection(0);
		UseBack(LTTRUE);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
		UseBack(LTFALSE);
	}
	CBaseScreen::OnFocus(bFocus);
}

// RETAIL: 0x10077290
uint32 CScreenSave::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM && dwCommand <= CMD_CUSTOM+kMaxSave)
	{
        uint32 slot = dwCommand - CMD_CUSTOM;

		g_nSaveSlot = slot;
		g_nSaveIndex = (int)dwParam1;

		if (g_pClientSaveLoadMgr->SlotSaveExists( g_nSaveSlot ))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = OverwriteCallBack,
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(0x391,&mb);
			return 1;
		}
		if (g_nSaveSlot > 0)
		{
			m_szSaveName[0] = LTNULL;		
			NameSaveGame(g_nSaveSlot,g_nSaveIndex);
		}
		else if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(0x387,&mb);
		}


	}
	else if (dwCommand == CMD_OVERWRITE)
	{
		NameSaveGame(g_nSaveSlot,g_nSaveIndex);
	}
	else if (dwCommand == CMD_EDIT_NAME)
	{
		// RETAIL: Tron's edit box hands back wide text
		char szName[40];
		NarrowInto(szName,std::wstring((wchar_t*)dwParam1),sizeof(szName));

		// '|' separates the fields of the save ini
		uint32 nBar;
		while ((nBar = strcspn(szName,"|")) < sizeof(szName) - 1 && szName[nBar] == '|')
		{
			szName[nBar] = '_';
		}
		szName[sizeof(szName) - 1] = 0;

		if (strlen(szName))
		{
			SAFE_STRCPY(m_szSaveName,szName);
		}
		ForceMouseUpdate();
		if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(0x387,&mb);
			return 0;
		}
	}

	else
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

// RETAIL: 0x10077570
void CScreenSave::BuildSavedLevelList()
{
	AddTextItem(0x123,CMD_CUSTOM,0xCC);

	int nFirstEmpty = -1;
	
	CLTGUITextCtrl *pCtrl = AddTextItem(0x22C,LTNULL,LTNULL,kDefaultPos,LTTRUE);
	pCtrl->Enable(LTFALSE);

	std::wstring sText;
	CUIFont *pFont = g_pInterfaceResMgr->GetFontForHeight(kSmallFontSize);

 	for (int i = 0; i < kMaxSave; i++)
	{
		LTBOOL bEmpty = LTTRUE;

		if (g_pClientSaveLoadMgr->SlotSaveExists( i+1 ))
		{
			char strTime[128] = "";

			char szSaveTitle[SLMGR_MAX_INISTR_LEN+1];
			char szWorldName[MAX_PATH*2];
			time_t saveTime;

			if( g_pClientSaveLoadMgr->ReadSaveINI( g_pClientSaveLoadMgr->GetSlotSaveKey( i+1 ), 
				szSaveTitle, ARRAY_LEN( szSaveTitle ), szWorldName, ARRAY_LEN( szWorldName ), &saveTime ))
			{
				struct tm* pTimeDate = localtime (&saveTime);
				if (pTimeDate)
				{
					if (g_pInterfaceResMgr->IsEnglish())
					{
						sprintf (strTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}
					else
					{
						sprintf (strTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}

				}

			}


			char *pWorld = LTNULL;
			if ( szSaveTitle[0] )
			{
				pWorld = szSaveTitle;
				bEmpty = LTFALSE;
			}
			else if ( szWorldName[0] )
			{
				pWorld = szWorldName;
				bEmpty = LTFALSE;
			}


			if (pWorld)
			{
				CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(CMD_CUSTOM+1+i, 0xCD);
				pCtrl->SetFont(pFont,kSmallFontSize);
				pCtrl->SetParam1( m_controlArray.size() - 1);

				// This is a spacer
				pCtrl->AddColumn(std::wstring(), kIndent);

				// The world name column
				LTGUIWiden(sText,pWorld);
				pCtrl->AddColumn(sText, kNameWidth);

				if (strlen(strTime))
				{
					// The column that contains the date/time
					LTGUIWiden(sText,strTime);
					pCtrl->AddColumn(sText, kTimeWidth);
				}
			}


		}


		if (bEmpty && nFirstEmpty < 0) 
			nFirstEmpty = i;

	}

	if (nFirstEmpty >= 0)
	{
		CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(CMD_CUSTOM+1+nFirstEmpty, 0xCD);
		pCtrl->SetFont(pFont,kSmallFontSize);
		pCtrl->SetParam1( m_controlArray.size() - 1);
		// This is a spacer
		pCtrl->AddColumn(L" ", kIndent);

		// The world name column
		pCtrl->AddColumn(LoadTempWString(0x295), kNameWidth);

		// The column that contains the date/time
		pCtrl->AddColumn(L" ", kTimeWidth);
	}

}

void CScreenSave::ClearSavedLevelList()
{
	RemoveAll();

}

LTBOOL CScreenSave::SaveGame(uint32 slot)
{
	if (slot > 0)
	{
		if( !g_pClientSaveLoadMgr->SaveGameSlot( slot, m_szSaveName ))
			return false;
		}
	else
	{
		if( !g_pClientSaveLoadMgr->QuickSave() )
			return false;
	}

//	g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_GAMESAVED);
	g_pInterfaceMgr->ChangeState(GS_PLAYING);
	return LTTRUE;
}


// RETAIL: 0x10077b30
LTBOOL CScreenSave::HandleKeyDown(int key, int rep)
{

	if (key == VK_F5)
	{
		SendCommand(CMD_CUSTOM,0,0);
        return LTTRUE;
	}
    return CBaseScreen::HandleKeyDown(key,rep);

}


// RETAIL: 0x10077b60
void CScreenSave::NameSaveGame(uint32 slot, int index)
{
	sprintf(m_szSaveName,"%s", g_pMissionMgr->GetCurrentWorldName());

	if (!g_pMissionMgr->IsCustomLevel())
	{
		int missionNum = g_pMissionMgr->GetCurrentMission();
		int sceneNum = g_pMissionMgr->GetCurrentLevel();
		MISSION* pMission = g_pMissionButeMgr->GetMission(missionNum);
		if (pMission)
		{
			int missionId = pMission->nNameId;
			char szMission[40] = "";
			char szLevel[40] = "";
			LoadString(missionId,szMission,sizeof(szMission));
			LEVEL* pLevel = g_pMissionButeMgr->GetLevel(missionNum,sceneNum);
			if (pLevel)
			{
				LoadString(pLevel->nNameId,szLevel,sizeof(szLevel));
			}

			if(strlen(szLevel))
				sprintf(m_szSaveName,"%s",szLevel);
			else
				sprintf(m_szSaveName,"%s",szMission);
		}
	}
	

	g_pColCtrl = (CLTGUIColumnCtrl*)GetControl(index);
	if (!g_pColCtrl)
		return;
	LTIntPt pos = g_pColCtrl->GetPos();

	g_sOldName = g_pColCtrl->GetString(1);
	g_sOldTime = g_pColCtrl->GetString(2);

	g_pColCtrl->SetString(1, L" ");
	g_pColCtrl->SetString(2, L" ");

	//show edit box here	
	MBCreate mb;
	mb.eType = LTMB_EDIT;
	mb.pFn = EditCallBack;
	mb.pString = m_szSaveName;
	mb.nMaxChars = 40;
	g_pInterfaceMgr->ShowMessageBox(0x44,&mb);

}

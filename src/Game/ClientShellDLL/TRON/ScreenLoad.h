// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLoad.h
//
// PURPOSE : Interface screen for loading saved games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_LOAD_H_)
#define _SCREEN_LOAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

struct SaveGameData
{
	SaveGameData() {szWorldName[0] = LTNULL;szUserName[0] = LTNULL;szTime[0] = LTNULL;}
	char szWorldName[128];
	char szUserName[128];
	char szTime[128];
};


class CScreenLoad : public CBaseScreen
{
public:
	CScreenLoad();
	virtual ~CScreenLoad();

    LTBOOL Build();
    void OnFocus(LTBOOL bFocus);
	virtual void	Escape();

	LTBOOL HandleKeyDown(int key, int rep);

protected:

    // MERGE: Tron's side (build 69); NOLF2 1.3 had:
    //| uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    //| void	BuildSavedLevelLists();
    //| void	ClearSavedLevelLists();
    //| void	ParseSaveString(char const* pszWorldName, char const* pszTitle, time_t const& saveTime, SaveGameData *pSG, bool bUserName);
    //| CLTGUIColumnCtrl* BuildSaveControls( char const* pszIniKey, uint32 nCommandId, uint32 nControlHelpStringId,
    //| bool bUserName );
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelLists();
	void	ClearSavedLevelLists();
	void	ParseSaveString(char const* pszWorldName, char const* pszTitle, time_t const& saveTime, SaveGameData *pSG, bool bUserName);
	// RETAIL: Tron's adds the row to pList, or to the screen without one
	// (0x1006e620)
	bool	BuildSaveControls( char const* pszIniKey, uint32 nCommandId, uint32 nControlHelpStringId, 
									bool bUserName, CLTGUIListCtrl *pList );

};

#endif // !defined(_SCREEN_LOAD_H_)
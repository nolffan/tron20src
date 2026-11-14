// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndMission.h
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_END_MISSION_H_)
#define _SCREEN_END_MISSION_H_

#include "BaseScreen.h"

class CScreenEndMission : public CBaseScreen
{
public:
	CScreenEndMission();
	virtual ~CScreenEndMission();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	OnLButtonDown(int x, int y);
    virtual LTBOOL	OnRButtonDown(int x, int y);

    virtual LTBOOL	Render(HSURFACE hDestSurf);


protected:

	// RETAIL: Tron's has NOLF2's mission statistics (retail CShell.dll
	// 0x1005a1a0..0x1005aaf0; ctor 0x1005a210)
	CLTGUITextCtrl		*m_pMission;		// +0x790
	CLTGUITextCtrl		*m_pAccuracy;		// +0x794
	CLTGUITextCtrl		*m_pMarksmanship;	// +0x798
	CLTGUITextCtrl		*m_pNonInjury;		// +0x79c
	CLTGUITextCtrl		*m_pTimesHit;		// +0x7a0
	CLTGUIListCtrl		*m_pList;			// +0x7a4


};

#endif // !defined(_SCREEN_END_MISSION_H_)
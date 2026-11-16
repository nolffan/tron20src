// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenGame.h
//
// PURPOSE : Interface screen for setting gameplay options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_GAME_H_
#define _SCREEN_GAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenGame : public CBaseScreen
{
public:
	CScreenGame();
	virtual ~CScreenGame();

	// Build the folder
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);


protected:


    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
	// RETAIL: Tron's (0x7c0 bytes; constructor 0x1005b240)
    uint8	            m_nSubtitles;				// +0x790
	uint8				m_nDifficulty;				// +0x791
	LTBOOL				m_bAlwaysRun;				// +0x794
	uint8				m_nLayout;					// +0x798
	int					m_nHeadBob;					// +0x79c
	int					m_nWeaponSway;				// +0x7a0
	int					m_nMsgDur;					// +0x7a4
	LTBOOL				m_bLCThirdPersonCam;		// +0x7a8
	LTBOOL				m_bDiscAutoAim;				// +0x7ac
	int					m_nLCCamSpeed;				// +0x7b0
	int					m_nLCCamLag;				// +0x7b4
	LTBOOL				m_bLCDialog;				// +0x7b8
	CLTGUICycleCtrl		*m_pDifficultyCtrl;			// +0x7bc The difficulty control
};

#endif // _SCREEN_GAME_H_
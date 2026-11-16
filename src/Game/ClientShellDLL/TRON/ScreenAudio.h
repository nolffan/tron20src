// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenAudio.cpp
//
// PURPOSE : Interface screen for setting audio options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_AUDIO_H_
#define _SCREEN_AUDIO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

// RETAIL: Tron's CScreenAudio (0x820 bytes; constructor 0x10054e30) adds
// an EAX toggle and its logo.
class CScreenAudio : public CBaseScreen
{
public:
	CScreenAudio();								// RETAIL: 0x10054e30
	virtual ~CScreenAudio();					// RETAIL: 0x10054ed0

	// Build the screen
    LTBOOL   Build();							// RETAIL: 0x10054ee0
    void    OnFocus(LTBOOL bFocus);				// RETAIL: 0x10055700
    LTBOOL   Render(HSURFACE hDestSurf);		// RETAIL: 0x100558f0

	// Override the left and right controls so that the volumes can be changed
    LTBOOL   OnLeft();							// RETAIL: 0x10055b40
    LTBOOL   OnRight();							// RETAIL: 0x10055c30
    LTBOOL   OnEnter();							// RETAIL: 0x10055d10
    LTBOOL   OnLButtonUp(int x, int y);			// RETAIL: 0x10055df0
    LTBOOL   OnRButtonUp(int x, int y);			// RETAIL: 0x10055ee0

	void	ScreenDimsChanged();				// RETAIL: 0x10055fd0

protected:
	void	ConfirmQualityChange(bool bMusic);	// RETAIL: 0x100559a0
	void	SaveSoundSettings();				// RETAIL: 0x10055a80

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);	// RETAIL: 0x10055670

protected:
	int		m_nSoundVolume;						// +0x790
	int		m_nMusicVolume;						// +0x794
	int		m_nSpeechVolume;					// +0x798
    LTBOOL  m_bSoundQuality;					// +0x79c
    LTBOOL  m_bMusicQuality;					// +0x7a0
    LTBOOL  m_bMusicEnabled;					// +0x7a4
	LTBOOL	m_bEAX;								// +0x7a8 RETAIL: Tron's

	uint8	m_nPerformance;						// +0x7ac


	CLTGUISlider	*m_pSoundVolumeCtrl;		// +0x7b0
	CLTGUISlider	*m_pMusicVolumeCtrl;		// +0x7b4
	CLTGUISlider	*m_pSpeechVolumeCtrl;		// +0x7b8
	CLTGUIToggle	*m_pSoundQualityCtrl;		// +0x7bc
	CLTGUIToggle	*m_pMusicQualityCtrl;		// +0x7c0
	CLTGUIToggle	*m_pEAXCtrl;				// +0x7c4 RETAIL: Tron's

	// RETAIL: Tron's EAX logo
	HTEXTURE		m_hEAXTexture;				// +0x7c8
	LT_POLYFT4		m_EAXPoly;					// +0x7cc
};

#endif // _SCREEN_AUDIO_H_
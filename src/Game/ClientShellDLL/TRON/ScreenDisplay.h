// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenDisplay.h
//
// PURPOSE : Interface screen for setting display options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_DISPLAY_H_
#define _SCREEN_DISPLAY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"


typedef struct ScreenDisplayResolution_t
{
    uint32 m_dwWidth;       // Screen width
    uint32 m_dwHeight;      // Screen height
    uint32 m_dwBitDepth;    // Screen bitdepth
} ScreenDisplayResolution;

typedef struct ScreenDisplayRenderer_t
{
    bool	m_bHWTnL;

	char	m_renderDll[200];		// The DLL name for the renderer
	char	m_internalName[200];	// This is what the DLLs use to identify a card
	char	m_description[200];		// The description of the renderer

	// An array of video resolutions
	CMoArray<ScreenDisplayResolution>	m_resolutionArray;
} ScreenDisplayRenderer;



class CScreenDisplay : public CBaseScreen
{
public:
	CScreenDisplay();
	virtual ~CScreenDisplay();

	// Build the screen
    LTBOOL   Build();

	// Handle input
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnEnter();

	void	Escape();
    void    OnFocus(LTBOOL bFocus);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	// Build the array of resolutions
	void	GetRendererData();

	// Setup the resolution control based on the currently selected resolution
	void	SetupResolutionCtrl();

	// Sort the render resolution based on screen width and height
	void	SortRenderModes();

	// Gets a RMode structure based on resolution index
	RMode	GetRendererModeStruct(int nResolutionIndex);

	// Returns the currently selected resolution
	ScreenDisplayResolution	GetCurrentSelectedResolution();

	// Set the resolution for the resolution control.  If it cannot be found the
	// next highest resolution is selected.
	void	SetCurrentCtrlResolution(ScreenDisplayResolution resolution);


private:
	// RETAIL: Tron's layout (Escape 0x10058c70 +0x794, OnFocus 0x10059440,
	// OnLeft 0x10059780): it has no hardware cursor control, and the vsync
	// flag comes last. +0x790 is not touched by the code written so far.
	LTBOOL							m_bHardwareCursor;		// +0x790
	LTBOOL							m_bEscape;				// +0x794
	int								m_nGamma;				// +0x798

	CLTGUICycleCtrl					*m_pResolutionCtrl;		// +0x79c The resolution control

	ScreenDisplayRenderer			m_rendererData;			// +0x7a0

	CLTGUISlider					*m_pGamma;				// +0xa10
	LTBOOL							m_bVSync;				// +0xa14



};

#endif // _SCREEN_DISPLAY_H_
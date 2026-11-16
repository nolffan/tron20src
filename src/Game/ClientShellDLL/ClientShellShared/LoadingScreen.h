// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.h
//
// PURPOSE : Background-thread loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000, 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOADINGSCREEN_H__
#define __LOADINGSCREEN_H__

// RETAIL: Tron's loading screen header does not bring in BaseScreen.h (it
// needs CBaseScreen only by pointer): BaseScreen.h's kDefaultPos has a
// dynamic initializer in exactly the client TUs that include BaseScreen.h
// themselves or through ScreenPreload.h/ScreenPostload.h (InterfaceMgr,
// BaseScreen, MissionMgr, ScreenMgr, LoadingScreen, ScreenPreload,
// ScreenPostload: CShell.dll 0x100c6180, 0x100d16e0, 0x100eb5c0,
// 0x10144f20, 0x10161750, 0x1016a830, 0x1016b150), not in the many that
// reach it through InterfaceMgr.h; so it is not in this project's
// precompiled header either (VC6 emits the initializers of a precompiled
// header's constants only in the /Yc object, StdAfx.obj, never linked).
// The headers BaseScreen.h brought in for this one are kept.
#include "LTGUIMgr.h"
#include "BaseScaleFX.h"
#include "ChainedFX.h"
#include "LTPoly.h"
#include "ScreenMgr.h"

class CBaseScreen;

class CLoadingScreen
{
// External functions
public:
	CLoadingScreen();
	// RETAIL: virtual: retail's CLoadingScreen has a vtable (0x101eb940,
	// two slots: this, then 0x10161f70, not yet identified), so its
	// members sit 4 bytes later (CInterfaceMgr's inline uses read the
	// state at +0x30: 0x100c8680)
	virtual ~CLoadingScreen();

	// Note : Init/Term will be called automatically when needed, so
	// you shouldn't ever actually need to call them.

	// Initialize and load the screen resources
	// Returns LTFALSE if not in STATE_NONE
	LTBOOL Init();
	// Terminate the screen and its resources
	// Returns LTFALSE if not in STATE_INIT
	LTBOOL Term();

	// Show the loading screen
	// Setting bRun to LTTRUE will start the update thread
	// Returns LTFALES if the state isn't STATE_NONE or STATE_INIT
	LTBOOL Show(LTBOOL bRun = LTTRUE);
	// Update the loading screen
	// Returns LTFALSE if the state isn't STATE_SHOW
	LTBOOL Update();
	// Take the loading screen out of threaded mode
	// Returns LTFALSE if the state isn't STATE_ACTIVE
	LTBOOL Pause();
	// Resume threaded mode
	// Returns LTFALSE if the state isn't STATE_SHOW
	LTBOOL Resume();
	// Hide the loading screen
	// Returns LTFALSE if the state isn't STATE_SHOW or STATE_ACTIVE
	LTBOOL Hide();

	// Returns whether or not the loading screen is currently being actively updated
	inline LTBOOL IsActive() const { return m_eCurState == STATE_ACTIVE; };
	// Returns whether or not the loading screen is visible
	inline LTBOOL IsVisible() const { return ((m_eCurState == STATE_ACTIVE) || (m_eCurState == STATE_SHOW)); };

	//is loading a new mission?
	bool	NeedsPostLoadScreen() const;

	void	UpdateMissionInfo();



// Internal functions
protected:
#ifdef NOPS2
	static unsigned long WINAPI ThreadBootstrap(void *pData);
	int RunThread();
#endif
private:
#ifdef NOPS2
	// The thread handle
	HANDLE m_hThreadHandle;
	// Events...
	HANDLE m_hEventEnd; // Stop rendering, damnit!
	HANDLE m_hEventThreadRunning; // Ok, the thread is done initializing, continue on

	CRITICAL_SECTION m_MissionUpdate;
	
#endif

	LTBOOL	m_bOldFogEnable;
	int		m_nOldFarZ;

	// State of the loading screen
	enum EScreenState { 
		STATE_NONE, // Not active in any way
		STATE_INIT, // It's been initialized, but that's it..
		STATE_SHOW, // It's on screen
		STATE_ACTIVE // It's in a thread and running
	};
	EScreenState m_eCurState;

	//////////////////////////////////////////////////////////////////////////////
	// Variables needed for updating (i.e. stuff only Init/Update/Term should touch)

	CUIFormattedPolyString 	*m_pMissionNameStr;
	CUIFormattedPolyString 	*m_pLevelNameStr;
	CUIFormattedPolyString 	*m_pBriefingStr;
	CUIFormattedPolyString 	*m_pHelpStr;
	std::string		m_missionname;
	std::string		m_levelname;
	std::string		m_briefing;
	std::string		m_help;
	std::string		m_layout;

	// RETAIL: Tron's has no mission photo (constructor 0x10161760: the
	// SFX array is at +0xc4)



	//layout info
	bool	m_bReadLayout;
	LTIntPt m_DefaultTitlePos;
	uint8	m_nDefaultTitleFont;
	uint8	m_nDefaultTitleFontSize;
	uint32	m_nDefaultTitleColor;
	LTIntPt m_DefaultLevelPos;
	uint8	m_nDefaultLevelFont;
	uint8	m_nDefaultLevelFontSize;
	uint32	m_nDefaultLevelColor;
	LTIntPt m_DefaultBriefingPos;
	uint16	m_nDefaultBriefingWidth;
	uint8	m_nDefaultBriefingFont;
	uint8	m_nDefaultBriefingFontSize;
	uint32	m_nDefaultBriefingColor;
	LTIntPt m_DefaultHelpPos;
	uint16	m_nDefaultHelpWidth;
	uint8	m_nDefaultHelpFont;
	uint8	m_nDefaultHelpFontSize;
	uint32	m_nDefaultHelpColor;

	// Array of SFX owned by the loading screen
	CMoArray<CSpecialFX *> m_SFXArray;

	// Position variables for internal use
    LTVector m_vPos, m_vU, m_vR, m_vF;
    LTRotation m_rRot;

	void CreateInterfaceSFX();

	// Functions for handling the character/attributes/sfx/etc
	// Note that these are basically copies of what's in BaseScreen
	void CreateScaleFX(char *szFXName);
	void CreateLightFX(char *szFXName);

	void RemoveInterfaceSFX();
	void UpdateInterfaceSFX();

	uint32 m_nFrameCounter; // Frame tracking
	float m_fLastFrameTime, m_fCurFrameDelta;
	
	// RETAIL: Tron's has no render screen (NOLF2 1.x's; UpdateInterfaceSFX
	// 0x10161f50)

};

#endif //__LOADINGSCREEN_H__
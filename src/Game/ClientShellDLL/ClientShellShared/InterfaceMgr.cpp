// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "ltquaduvutils.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "WeaponStringDefs.h"
#include "VKDefs.h"
#include "SoundMgr.h"
#include "InterfaceResMgr.h"
#include "VarTrack.h"
#include "ClientButeMgr.h"
#include "CharacterFX.h"
#include "GameButes.h"
#include "UberAssert.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"
#include "PlayerCamera.h"
#include "ClientSaveLoadMgr.h"
#include "ScreenPreload.h"
#include "ScreenPostload.h"
#include "timer.h"
#include "WaveFn.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"

CInterfaceMgr*  g_pInterfaceMgr = LTNULL;

#define IM_SPLASH_SOUND		"Interface\\Menu\\Snd\\theme_mp3.wav"

#ifdef _TO2DEMO
#define IM_SPLASH_SCREEN	"Interface\\demosplash.pcx"
#else
#define IM_SPLASH_SCREEN	"Interface\\Menu\\Art\\splash.pcx"
#endif

#define MAX_INTERFACE_SFX		100
#define MAX_INTERFACE_LIGHTS	5
#define INVALID_ANI			((HMODELANIM)-1)

// RETAIL: Tron's intro movies (GetCurrentMovie 0x100cec90; gMovieOrder at
// 0x102289f4 is 0, 1, 2). The names are ours.
static enum MovieOrderEnum 
{
	eDisney,
	eMonolith,
	eNVidia
};

//hack to track whether we are trying to host/join a LAN only or an internet game
bool g_bLAN = false;

// RETAIL: set by SwitchToScreen (0x100cc6a0) around the change to the screen
// state when the subroutines screen is the target, read by PreScreenState
// (0x100cb6aa): that screen keeps the game's music. The name is ours.
static LTBOOL s_bKeepGameMusic = LTFALSE;

static MovieOrderEnum gMovieOrder[] =
{
	eDisney, eMonolith, eNVidia
};

LTFLOAT      g_fSplashSndDuration = 0.0f;

LTFLOAT      g_fFovXTan = 0.0f;
LTFLOAT      g_fFovYTan = 0.0f;

LTVector     g_vOverlaySpriteScale(0.02f, 0.02f, 1.0f);
LTVector     g_vOverlayModelScale(1.0f, 1.0f, 1.0f);
LTFLOAT      g_fOverlaySpriteDist = 1.0f;
LTFLOAT      g_fOverlayModelDist = 0.25f;

LTVector     g_vBaseBackScale(0.8f, 0.6f, 1.0f);
LTFLOAT      g_fBackDist = 200.0f;

HSURFACE    g_hSplash = LTNULL;
HSURFACE    g_hDemo = LTNULL;
int			g_nDemo = 0;

#define NUM_DEMO_SCREENS 3
char  g_szDemoScreens[NUM_DEMO_SCREENS][64] =
{
	"Interface\\DemoEndSplash1.pcx",
	"Interface\\DemoEndSplash2.pcx",
	"Interface\\DemoEndSplash3.pcx",
};
uint8 g_nNumDemoScreens = sizeof(g_szDemoScreens)/sizeof(char*);


VarTrack	g_vtDrawInterface;
VarTrack	g_vtModelApplySun;
VarTrack	g_vtLetterBox;
VarTrack	g_vtLetterBoxFadeInTime;
VarTrack	g_vtLetterBoxFadeOutTime;
VarTrack	g_vtLetterBoxDisabled;
VarTrack	g_vtDisableMovies;
VarTrack	g_vtInterfaceFOVX;
VarTrack	g_vtInterfaceFOVY;
VarTrack	g_vtPauseTintAlpha;
VarTrack	g_vtSplashScreenFadeIn;
VarTrack	g_vtSplashScreenFadeOut;
// RETAIL: Tron has no SplashScreenTime console variable (Init, 0x100c75b0)
VarTrack	g_vtMainScreenFadeIn;
VarTrack	g_vtProgressBarScaleToSkills;
VarTrack	g_vtExitLevelScreenFadeTime;
VarTrack	g_vtOverlayModelDist;	// RETAIL: Tron's (0x1026f7e0)
VarTrack	g_vtMouseTrailFix;		// RETAIL: Tron's (0x1026f7d0)

extern CMoveMgr* g_pMoveMgr;
extern VarTrack g_vtFOVXNormal;
extern VarTrack g_vtFOVYNormal;
extern VarTrack g_vtFOVXCycle;
extern VarTrack g_vtFOVYCycle;

extern VarTrack g_vtScreenFadeInTime;

const char* c_GameStateNames[] =
{
	"GS_UNDEFINED",
	"GS_PLAYING",
	"GS_EXITINGLEVEL",
	"GS_LOADINGLEVEL",
	"GS_SPLASHSCREEN",
	"GS_MENU",
	"GS_POPUP",
	"GS_SCREEN",
	"GS_PAUSED",
	"GS_DEMOSCREEN",
	"GS_MOVIE",
};


static LTMatrix GetCameraTransform(HOBJECT hCamera)
{
    LTVector vPos, vRight, vUp, vForward;
    LTRotation rRot;

	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

	vPos.x = -vPos.x;
	vPos.y = -vPos.y;
	vPos.z = -vPos.z;

    LTMatrix mTran, mRot, mFull;

	mRot.SetBasisVectors((LTVector*)&rRot.Right(), (LTVector*)&rRot.Up(), (LTVector*)&rRot.Forward());
	MatTranspose3x3(&mRot);

	Mat_Identity(&mTran);
	mTran.m[0][3] = vPos.x;
	mTran.m[1][3] = vPos.y;
	mTran.m[2][3] = vPos.z;

	MatMul(&mFull, &mRot, &mTran);

	return mFull;
}


namespace
{
	LTBOOL g_bInGameFogEnabled = LTTRUE;

	HLTCOLOR hBackColor;

	//**********************************************************
	//*** definitions needed for rendering FX in the interface
	//**********************************************************
	typedef struct FXRenderInfo_t
	{
		HOBJECT hObj;
		uint8	nLayer;
	} FXRenderInfo;

	const int kMaxFX = MAX_INTERFACE_SFX + MAX_INTERFACE_LIGHTS + 2;
	static FXRenderInfo sRenderInfo[kMaxFX];
	typedef std::list<FXRenderInfo *> FXRenderList;
	FXRenderList renderList;


	bool g_bLockPopup = true;

	CTimer g_SplashScreenTimer;
}


static LTBOOL HandleDebugKey(int key )
{
#ifdef _FINAL
	return LTFALSE;
#endif

#ifdef _TO2DEMO
	//don't bother checking in demo build
	return LTFALSE;
#endif

	char szTempStr[256];
	char szVarName[256];

	int iID = -1;

	// Go through each debug key and see if it matches the key hit.
	// Favor modified versions of keys (e.g. shift F11).

	for( int i = 0; i < g_pClientButeMgr->GetNumDebugKeys(); ++i )
	{
		if( g_pClientButeMgr->GetDebugKeyId(i) == key )
		{
			if( IsKeyDown( g_pClientButeMgr->GetDebugModifierId(i) ) )
			{
				iID = i;	
			}
			else if( iID == -1 )
			{
				iID = i;
			}
		}
	}

	if( iID != -1 )
	{
		szVarName[0] = 0;
		g_pClientButeMgr->GetDebugName(iID,szVarName,256);

		if( !szVarName[0] )
		{
			return LTFALSE;
		}

		// Get the console variable associated with this debug key.
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szVarName);

		// Initialize the variable if it does not exist.
		if( !hVar )
		{
			sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, 0.0f );
			g_pLTClient->RunConsoleString( szTempStr );

			hVar = g_pLTClient->GetConsoleVar(szVarName);
			ASSERT( hVar );

			if( !hVar )
			{
				return LTFALSE;
			}
		}

		// Increment the current debug level, recycling to zero if it is above max levels.
		LTFLOAT fCurrentLevel = g_pLTClient->GetVarValueFloat(hVar);
		++fCurrentLevel;

		if( int(fCurrentLevel) >= g_pClientButeMgr->GetNumDebugLevels(iID) )
		{
			fCurrentLevel = 0.0f;
		}
		
		// Set the new debug level.
		sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, fCurrentLevel );
		g_pLTClient->RunConsoleString( szTempStr );

		g_pLTClient->CPrint("%s %2f", szVarName, fCurrentLevel );

		szTempStr[0] = 0;
		g_pClientButeMgr->GetDebugString(iID,uint8(fCurrentLevel), szTempStr, 256);
		if( szTempStr )
		{
			g_pLTClient->RunConsoleString( szTempStr );
#ifdef _DEBUG
			g_pLTClient->CPrint( szTempStr );
#endif
		}

		// Display message
		szTempStr[0] = 0;
		g_pClientButeMgr->GetDebugTitle(iID,uint8(fCurrentLevel), szTempStr, 256);
		if( szTempStr[0] )
		{
			g_pChatMsgs->AddMessage( szTempStr );
		}
		else
		{
			sprintf(szTempStr, "%s set to level %1.0f", szVarName, fCurrentLevel );

			g_pChatMsgs->AddMessage( szTempStr );
		}

		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TronIntelRecord
//
//	PURPOSE:	Tron's record of an intel item (RETAIL: 0x100c6650,
//				0x100c6660, 0x100c66b0, 0x100c6710, 0x100c6760)
//
// ----------------------------------------------------------------------- //

TronIntelRecord::TronIntelRecord()
{
	m_nTextId = 0;
	m_nPopupId = 0;
	m_bIsIntel = false;
	m_szName[0] = 0;
}

TronIntelRecord::TronIntelRecord(uint32 nTextId, uint8 nPopupId, bool bIsIntel, const char* pszName)
{
	m_nTextId = nTextId;
	m_nPopupId = nPopupId;
	m_bIsIntel = bIsIntel;
	if (pszName)
		SAFE_STRCPY(m_szName, pszName)
	else
		SAFE_STRCPY(m_szName, "")
}

TronIntelRecord::TronIntelRecord(const TronIntelRecord& other)
{
	m_nTextId = other.m_nTextId;
	m_nPopupId = other.m_nPopupId;
	m_bIsIntel = other.m_bIsIntel;
	SAFE_STRCPY(m_szName, other.m_szName);
}

TronIntelRecord::~TronIntelRecord()
{
}

void TronIntelRecord::Show(bool& bHideHUD, CPopupText& popup)
{
	bHideHUD = m_bIsIntel;
	popup.Show(m_nTextId, m_nPopupId, m_szName);
}

void TronIntelRecord::Save(ILTMessage_Write *pMsg)
{
	pMsg->Writeuint32(m_nTextId);
	pMsg->Writeuint8(m_nPopupId);
	pMsg->Writebool(m_bIsIntel);
	pMsg->WriteString(m_szName);
}

void TronIntelRecord::Load(ILTMessage_Read *pMsg)
{
	m_nTextId = pMsg->Readuint32();
	m_nPopupId = pMsg->Readuint8();
	m_bIsIntel = pMsg->Readbool();
	pMsg->ReadString(m_szName, sizeof(m_szName));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CInterfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::CInterfaceMgr()
{
	// RETAIL: layout anchors from retail's constructor (0x100c67b0): fail
	// to compile when a member's size changes (adjust the pads)
	typedef char TronCheckInitialized[offsetof(CInterfaceMgr, m_bInitialized) == 0xf980 ? 1 : -1];
	typedef char TronCheckSettings[offsetof(CInterfaceMgr, m_Settings) == 0xe764 ? 1 : -1];
	typedef char TronCheckFullScreenTint[offsetof(CInterfaceMgr, m_FullScreenTint) == 0xf40c ? 1 : -1];
	typedef char TronCheckInterfaceSFX[offsetof(CInterfaceMgr, m_InterfaceSFX) == 0xf530 ? 1 : -1];
	typedef char TronCheckLoadingScreen[offsetof(CInterfaceMgr, m_pLoadingScreen) == 0xf8e4 ? 1 : -1];
	typedef char TronCheckLetterBox[offsetof(CInterfaceMgr, m_LetterBox) == 0xf900 ? 1 : -1];
	typedef char TronCheckSize[sizeof(CInterfaceMgr) == 0x10108 ? 1 : -1];
	typedef char TronCheckCursorMgr[offsetof(CInterfaceMgr, m_CursorMgr) == 0x40 ? 1 : -1];
	typedef char TronCheckMessageBox[offsetof(CInterfaceMgr, m_MessageBox) == 0x7c ? 1 : -1];
	typedef char TronCheckInterfaceResMgr[offsetof(CInterfaceMgr, m_InterfaceResMgr) == 0x624 ? 1 : -1];
	typedef char TronCheckMenuMgr[offsetof(CInterfaceMgr, m_MenuMgr) == 0xebc ? 1 : -1];
	typedef char TronCheckScreenSpriteMgr[offsetof(CInterfaceMgr, m_ScreenSpriteMgr) == 0xe73c ? 1 : -1];

	g_pInterfaceMgr	= this;

	m_eGameState			= GS_UNDEFINED;
	m_eLastGameState		= GS_UNDEFINED;


	m_dwAdvancedOptions		= AO_DEFAULT_ENABLED;
	m_dwOrignallyEnabled	= 0;

	m_nClearScreenCount		= 0;
    m_bClearScreenAlways    = LTFALSE;

	m_fMenuSaveFOVx			= 0.0f;
	m_fMenuSaveFOVy			= 0.0f;

    m_bSwitchingModes       = LTFALSE;

    m_hSplashSound          = LTNULL;
	g_fSplashSndDuration	= 0.0f;

    m_nFailStringId         = LTNULL;

	m_bUseInterfaceCamera	= LTTRUE;
	m_hInterfaceCamera		= LTNULL;

	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
        m_hOverlays[i] = LTNULL;
		m_fOverlayScaleMult[i] = 1.0f;
	}
	m_nOverlayCount = 0;

    m_hScubaSound		= LTNULL;
	m_hSound			= LTNULL;

    m_hFadeSurface      = LTNULL;
    m_hLetterBoxSurface = LTNULL;
/*
	m_nBorderSize		= 0;
*/
	m_fLetterBoxAlpha	= 0.0f;
	m_bLetterBox		= LTFALSE;
	m_bWasLetterBox		= LTFALSE;
	m_fLetterBoxFadeEndTime = 0.0f;

    m_bScreenFade       = LTFALSE;
    m_bFadeInitialized  = LTFALSE;
	m_fTotalFadeTime	= 0.0f;
	m_fCurFadeTime		= 0.0f;
    m_bFadeIn           = LTTRUE;
	m_bTronUnknownFadeF4A0 = false;

	m_bSavedGameMusic   = LTFALSE;

	m_eNextSound		= IS_NONE;

	m_bQuitAfterDemoScreens = LTFALSE;
	m_bSeenDemoScreens = LTFALSE;

	m_hMovie			= LTNULL;
	m_nCurMovie			= gMovieOrder[0];

	m_bLoadFailed		= LTFALSE;
	m_bCommandLineJoin = false;
	m_eLoadFailedScreen = SCREEN_ID_MAIN;
	m_nLoadFailedMsgId	= -1;

	m_hGamePausedSurface = LTNULL;

	m_fLastUpdateRequestTime = 0.0f;

	m_MouseFX.ClearLink();
	m_SelectFX.ClearLink();

	m_bInitialized = false;
	m_bTronUnknownF988 = false;
	m_bHideHUDInPopup = false;
	m_ePrePopupHUDRenderLevel = kHUDRenderFull;

	m_bSuppressNextFlip = false;

	m_bEnteredScreenState = false;
	m_bIntentionalDisconnect = false;

	m_fTronUnknownF98C = -1.0f;
	m_szTronUnknownF994[0] = '\0';
	m_szTronUnknownFA14[0] = '\0';
	m_bTronUnknownFA54 = true;
	m_pSpectatorText = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::~CInterfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::~CInterfaceMgr()
{
	if (m_hSplashSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
	}

	if (m_hScubaSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hScubaSound);
	}

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
	}

	if (m_hFadeSurface)
	{
        g_pLTClient->DeleteSurface(m_hFadeSurface);
	}

	if (m_hLetterBoxSurface)
	{
        g_pLTClient->DeleteSurface(m_hLetterBoxSurface);
	}


    g_pInterfaceMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
LTBOOL CInterfaceMgr::Init()
{
	if (!m_ScreenSpriteMgr.Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize ScreenSpriteMgr!");
		return LTFALSE;
	}

    if (!m_CursorMgr.Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!");
        return LTFALSE;
	}

    g_vtDrawInterface.Init(g_pLTClient, "DrawInterface", NULL, 1.0f);
    g_vtModelApplySun.Init(g_pLTClient, "ModelApplySun", NULL, 1.0f);


	g_vtLetterBox.Init(g_pLTClient, "LetterBox", NULL, 0.0f);
	g_vtLetterBoxDisabled.Init(g_pLTClient, "LetterBoxDisabled", NULL, 0.0f);
    g_vtLetterBoxFadeInTime.Init(g_pLTClient, "LetterBoxFadeInTime", NULL, 0.5f);
    g_vtLetterBoxFadeOutTime.Init(g_pLTClient, "LetterBoxFadeOutTime", NULL, 1.0f);
    g_vtDisableMovies.Init(g_pLTClient, "NoMovies", NULL, 0.0f);

    g_vtInterfaceFOVX.Init(g_pLTClient, "FovXInterface", NULL, 90.0f);
    g_vtInterfaceFOVY.Init(g_pLTClient, "FovYInterface", NULL, 75.0f);

	g_vtPauseTintAlpha.Init(g_pLTClient, "PauseTintAlpha", NULL, 0.65f);

	g_vtSplashScreenFadeIn.Init(g_pLTClient, "SplashScreenFadeInTime", NULL, 3.0f);	// RETAIL: 0x100c7bc6
	g_vtSplashScreenFadeOut.Init(g_pLTClient, "SplashScreenFadeOutTime", NULL, 5.0f);	// RETAIL: 0x100c7c53
	g_vtMainScreenFadeIn.Init(g_pLTClient, "MainScreenFadeInTime", NULL, 3.0f);
	
	g_vtProgressBarScaleToSkills.Init( g_pLTClient, "ProgressBarScaleToSkills", LTNULL, 1.0f );
	
	g_vtExitLevelScreenFadeTime.Init( g_pLTClient, "ExitLevelScreenFadeTime", LTNULL, 1.0f );
	// RETAIL: 0x100c7e81, Tron's
	g_vtOverlayModelDist.Init( g_pLTClient, "OverlayModelDist", LTNULL, 5.01f );
	g_vtMouseTrailFix.Init( g_pLTClient, "MouseTrailFix", LTNULL, 1.0f );

	ProcessAdvancedOptions();

	// Create the Interface camera...
    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_ObjectType = OT_CAMERA;

    m_hInterfaceCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hInterfaceCamera);

    g_pLTClient->SetCameraRect(m_hInterfaceCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraFOV(m_hInterfaceCamera, DEG2RAD(g_vtInterfaceFOVX.GetFloat()), DEG2RAD(g_vtInterfaceFOVY.GetFloat()));

	// read in the settings
    m_Settings.Init (g_pLTClient, g_pGameClientShell);


    if (!g_pLayoutMgr || !g_pLayoutMgr->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize LayoutMgr!");
        return LTFALSE;
	}
	hBackColor =  g_pLayoutMgr->GetBackColor();

    if (!m_InterfaceResMgr.Init())
	{
		// If we couldn't init, something critical must have happened (like no render dlls)

        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize InterfaceResMgr!");
        return LTFALSE;
	}

    if (!GetScreenMgr( )->Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize ScreenMgr!");
        return LTFALSE;
	}

    if (!m_MenuMgr.Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize MenuMgr!");
        return LTFALSE;
	}

	// RETAIL: 0x100c80e0, the teams first
	m_TeamMgr.Init();

    m_ClientInfo.Init();

	if (!GetPlayerStats( )->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Player Stats!");
        return LTFALSE;
	}
	if (!GetHUDMgr()->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize HUDMgr!");
        return LTFALSE;
	}

	g_pPaused->Init();

	m_PopupText.Init();
	GetWeaponChooser()->Init();
	m_AmmoChooser.Init();


	m_CursorPos.x = 0;
	m_CursorPos.y = 0;

	g_fFovXTan = (float)tan(DEG2RAD(g_vtInterfaceFOVX.GetFloat())/2);
	g_fFovYTan = (float)tan(DEG2RAD(g_vtInterfaceFOVY.GetFloat())/2);

	m_MessageBox.Init();
	m_FullScreenTint.Init();

	if (!m_PerformanceMgr.Init()) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CPerformanceMgr::Init():  Could not initialize PerformanceMgr!");
		m_PerformanceMgr.Term();
		return LTFALSE;
	}


	if (!m_ProfileMgr.Init()) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CProfileMgr::Init():  Could not initialize ProfileMgr!");
		m_ProfileMgr.Term();
		return LTFALSE;
	}


	// Create the surface used for making letterboxed cinematics...
    LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);
    HLTCOLOR hTransColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTTRUE);

	m_hLetterBoxSurface = g_pLTClient->CreateSurface(2, 2);

	g_pLTClient->SetSurfaceAlpha(m_hLetterBoxSurface, 0.0f);
    g_pLTClient->FillRect(m_hLetterBoxSurface, &rcSrc, kBlack);
    g_pLTClient->OptimizeSurface(m_hLetterBoxSurface, hTransColor);

	if (!m_InterfaceFXMgr.Init(g_pLTClient,LTFALSE)) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CClientFXMgr::Init():  Could not initialize interface FX mgr!");
		m_InterfaceFXMgr.Term();
		return LTFALSE;
	}
	m_InterfaceFXMgr.SetCamera(m_hInterfaceCamera);

	m_szMouseFXName[0] = 0;
	m_szSelectFXName[0] = 0;

	// Consider ourselves initialized.
	m_bInitialized = true;

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Term()
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::Term()
{
	// Turn off the loading screen if it's currently up
	if (m_pLoadingScreen && m_pLoadingScreen->IsActive())
		m_pLoadingScreen->Hide();

	if (m_hGamePausedSurface)
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = LTNULL;
	}

	if (m_hInterfaceCamera)
	{
        g_pLTClient->RemoveObject(m_hInterfaceCamera);
        m_hInterfaceCamera = LTNULL;
	}

	//make sure that the interface manager isn't still holding onto a reference
	m_InterfaceFXMgr.SetCamera(NULL);

	// ABM TODO term the subroutinemgr and ratingsmgr

	GetScreenMgr( )->Term();
	m_MenuMgr.Term();
	if (g_pLayoutMgr)
		g_pLayoutMgr->Term();
	GetPlayerStats( )->Term();
	GetHUDMgr()->Term();
	GetWeaponChooser()->Term();
	m_AmmoChooser.Term();
	m_InterfaceResMgr.Term();

	m_MessageBox.Term();
	m_FullScreenTint.Term();
	m_PopupText.Term();
	m_Credits.Term();

	m_CursorMgr.Term();

	m_ScreenSpriteMgr.Term();

	if ((m_dwOrignallyEnabled & AO_SOUND) && !(m_dwAdvancedOptions & AO_SOUND))
	{
        g_pLTClient->RunConsoleString("SoundEnable 1");
	}

	if ((m_dwOrignallyEnabled & AO_MUSIC) && !(m_dwAdvancedOptions & AO_MUSIC))
	{
        g_pLTClient->RunConsoleString("MusicEnable 1");
	}

	if (m_hScubaSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hScubaSound);
		m_hScubaSound = LTNULL;
	}

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = LTNULL;
	}

	// Unititialized.
	m_bInitialized = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering new world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnEnterWorld(LTBOOL bRestoringGame)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->OnEnterWorld(bRestoringGame);

	// RETAIL: 0x100c8540: Tron adds the HUD's items again, and does not
	// enable the menus
	GetHUDMgr()->AddItems();

	// Update every HUD element so they display accurate info
	GetHUDMgr()->QueueUpdate( kHUDAll );

	m_fTronUnknownF98C = -1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnExitWorld()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	// RETAIL: 0x100c85a2: Tron takes the HUD items away, and empties the
	// client list and the teams
	GetHUDMgr()->RemoveItems();
	GetPlayerStats( )->OnExitWorld();
/*
	g_pMissionText->Clear();
	g_pChatInput->Show(LTFALSE, LTFALSE);
	g_pSubtitles->Clear();
*/
	GetClientInfoMgr()->RemoveAllClients();
	GetTeamMgr()->Init();
	m_Credits.Term();
	m_PopupText.Close();
	m_InterfaceTimer.SetTime(0.0f,false);
	m_FullScreenTint.TurnOn(false);
	
	g_pDisplayMeter->SetValue( 0 );

	if (m_AmmoChooser.IsOpen())
	{
		m_AmmoChooser.Close();
	}
	if (GetWeaponChooser()->IsOpen())
	{
		GetWeaponChooser()->Close();
	}


	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
            g_pLTClient->RemoveObject(m_hOverlays[i]);

        m_hOverlays[i] = LTNULL;
	}

	// RETAIL: 0x100c865c
	g_pTransmission->Term();

    m_bFadeInitialized = LTFALSE;


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreUpdate()
//
//	PURPOSE:	Handle pre-updates
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreUpdate()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Don't clear the screen if the loading screen's up
	if (m_pLoadingScreen && m_pLoadingScreen->IsVisible())
		return LTTRUE;

	if (m_bClearScreenAlways)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	}
	else if (m_nClearScreenCount)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
		m_nClearScreenCount--;
	}
	else
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_RENDER);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostUpdate()
//
//  PURPOSE:    Handle post-updates (return LTTRUE to FLIP
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostUpdate()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	if (m_eGameState != GS_LOADINGLEVEL)
	{
		if (!m_bSuppressNextFlip)
			g_pLTClient->FlipScreen(0);
		else
			m_bSuppressNextFlip = false;
	}

	m_CursorMgr.CheckForReinit();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Update()
//
//	PURPOSE:	Handle updating the interface
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::Update()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Update based on the game state...
    LTBOOL bHandled = LTFALSE;
	switch (m_eGameState)
	{
		case GS_PLAYING:
		{
			UpdatePlayingState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_MENU:
		{
			UpdateMenuState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_POPUP:
		{
			UpdatePopupState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_EXITINGLEVEL:
		{
			UpdateExitingLevelState();
            bHandled = LTFALSE;
		}
		break;

		case GS_LOADINGLEVEL:
		{
			UpdateLoadingLevelState();
            bHandled = LTTRUE;
		}
		break;

		case GS_SCREEN :
		{
			UpdateScreenState();
            bHandled = LTTRUE;
		}
		break;

		case GS_PAUSED:
		{
			UpdatePausedState();
            bHandled = LTTRUE;
		}
		break;

		case GS_SPLASHSCREEN:
		{
			UpdateSplashScreenState();
            bHandled = LTTRUE;
		}
		break;

		case GS_MOVIE:
		{
			UpdateMovieState();
            bHandled = LTTRUE;
		}
		break;

		case GS_DEMOSCREEN:
		{
			UpdateDemoScreenState();
            bHandled = LTTRUE;
		}
		break;


	}

	//in playing state, message box is drawn in InterfaceMgr::Draw(),
	// otherwise draw it here
	if (bHandled)
	{
		if (m_MessageBox.IsVisible())
		{
			g_pLTClient->Start3D();
			g_pLTClient->StartOptimized2D();
			m_MessageBox.Draw();
			m_ScreenSpriteMgr.Update();	// RETAIL: Tron's draws the sprites here (retail 0x100c8894)
			g_pLTClient->EndOptimized2D();
			g_pLTClient->End3D();
		}
		m_CursorMgr.Update();
	}

	// RETAIL: Tron's advances the sprites here (retail 0x100c88bc)
	m_ScreenSpriteMgr.TronUnknownAdvance();

	return bHandled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawSFX()
//
//	PURPOSE:	Renders the currently active special effects
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::DrawSFX()
{
    LTFLOAT fApplySun = g_vtModelApplySun.GetFloat();
	g_vtModelApplySun.SetFloat(0.0f);
	UpdateInterfaceSFX();
	g_vtModelApplySun.SetFloat(fApplySun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateScreenState()
//
//	PURPOSE:	Update screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenState()
{
	if (GetScreenMgr( )->GetCurrentScreenID() == SCREEN_ID_NONE)
	{
		SwitchToScreen(SCREEN_ID_MAIN);
	}
	
	// [KLS 9/2/02] FogEnabled is cleared in PreScreenState, however it is possible for
	// it to get set to 1 while in the screen state.  This will ensure that we never
	// render the screen state with fog enabled.  PostScreenState will return FogEnabled
	// to whatever it was before the screen state was entered.
	WriteConsoleInt("FogEnable", 0);

	// RETAIL: 0x100c89c0: Tron draws the light cycle match's countdown, the
	// credits and the cursor lines here, and the sprites unless a message
	// box is up
	if (!GetScreenMgr( )->UpdateInterfaceSFX())
	{
		m_bSuppressNextFlip = true;
		return;
	}

	g_pLTClient->Start3D();

	DrawSFX();

	g_pLTClient->StartOptimized2D();

	if (GetGameType() == eGameTypeMultiplayerLightcycles && g_pLTClient->IsConnected())
	{
		g_pTransmission->Update();
		DrawTronUnknownMessage();
		g_pTransmission->Render();
	}

	if (m_Credits.IsInited())
	{
		if (m_Credits.IsDone())
		{
			m_Credits.Term();
		}
		else
		{
			m_Credits.Update();
		}
	}
	else
	{
		m_InterfaceResMgr.DrawScreen();
	}

	m_CursorMgr.RenderCursorLines();

	UpdateScreenFade();

	if (!m_MessageBox.IsVisible())
	{
		m_ScreenSpriteMgr.Update();
	}

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayingState()
{
	GetPlayerStats()->Update();

	// Update auto chooser switching...
	GetWeaponChooser()->Update();
	m_AmmoChooser.Update();

	// Update the player stats...
	GetHUDMgr()->Update();
	g_pPaused->Update();

	// RETAIL: 0x100c8b11: show the intel picked up, the last first, when
	// no popup shows
	if (!m_PopupText.IsVisible() && !m_IntelRecords.empty())
	{
		m_IntelRecords.back().Show(m_bHideHUDInPopup, m_PopupText);
		m_IntelRecords.pop_back();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMenuState()
//
//	PURPOSE:	Update menu state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMenuState()
{
	if (!m_MenuMgr.GetCurrentMenu())
	{
		SwitchToMenu(MENU_ID_SYSTEM);
	}

	m_MenuMgr.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePopupState()
//
//	PURPOSE:	Update dialogue state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePopupState()
{
	if (m_PopupText.IsVisible())
	{
		m_PopupText.Update();
	}
	else
	{
		ChangeState(m_eLastGameState);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateExitingLevelState()
//
//	PURPOSE:	Update exiting level state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateExitingLevelState()
{
	// Update the screen fade.
	UpdateScreenFade();

	// RETAIL: 0x100c8bc0: Tron has no deathmatch end screen; its disc and
	// light cycle multiplayer games go through the preload screen, a finished
	// single player game back to the main screen. A multiplayer game does
	// not wait for the fade.
	// Check if screen is done fading out or is fading in at all.
	if( ScreenFadedOut( ) || FadingScreenIn( ) || IsMultiplayerGame( ))
	{
		if (g_pMissionMgr->IsExitingMission())
		{
			switch( g_pGameClientShell->GetGameType( ))
			{
				case eGameTypeCooperative:
					SwitchToScreen(SCREEN_ID_END_COOP_MISSION);
					break;

				case eGameTypeDiscArena:
				case eGameTypeDiscTournament:
				case eGameTypeMultiplayerLightcycles:
				{
					CScreenPreload *pPreload = (CScreenPreload *) (GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
					if (pPreload)
						pPreload->SetWaitingToExit(true);
					g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
				}
				break;

				default:
				{
					if (g_pMissionMgr->IsGameOver())
					{
						g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
					}
					else
					{
						CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
						if (pPreload)
							pPreload->SetWaitingToExit(true);
						g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
					}
				}
				break;
			}
		}
		else
		{
			CScreenPreload *pPreload = (CScreenPreload *) (GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
			if (pPreload)
			{
				pPreload->SetWaitingToExit(true);
				SwitchToScreen(SCREEN_ID_PRELOAD);
			}
			else
			{
				g_pMissionMgr->FinishExitLevel();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateLoadingLevelState()
//
//	PURPOSE:	Update loading level state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateLoadingLevelState()
{
	if (g_pPlayerMgr->IsPlayerInWorld())
	{
		// RETAIL: 0x100c8d4a: not while the level is being left again
		if (g_pMissionMgr->IsExitingLevel())
			return;

		// Make sure the loading screen isn't busy
		if (m_pLoadingScreen)
			m_pLoadingScreen->Pause();
		// Let the game client shell do stuff that's time consuming so we're not stuck on a black screen
		g_pGameClientShell->PostLevelLoadFirstUpdate();
		// Turn off the loading screen
		if (m_pLoadingScreen)
			m_pLoadingScreen->Hide();


		// RETAIL: 0x100c8d30: no team selection screen here in Tron, and no
		// NeedsPostLoadScreen check
		if (m_pLoadingScreen && GetConsoleInt("LoadScreenWait",1) )
		{
			SwitchToScreen( SCREEN_ID_POSTLOAD );
		}
		else 
		{
			// Tell the client we're ready to play.
			g_pGameClientShell->SendClientLoadedMessage( );

			ChangeState(GS_PLAYING);
		}
	}
	else if ((m_bLoadFailed) || (g_pLTClient->IsConnected() && IsKeyDown(VK_ESCAPE)))
	{
		m_bLoadFailed = LTFALSE;
		if (m_pLoadingScreen)
			m_pLoadingScreen->Hide();

		// We joined using the command line, we need to visit the main screen before
		// any other screen, since we didn't go through the normal screen
		// progression.
		if( GetCommandLineJoin( ))
		{
			SwitchToScreen( SCREEN_ID_MAIN );
		}
		else
		{
			SwitchToScreen(m_eLoadFailedScreen);
		}

		MBCreate mb;
		// RETAIL: 0x100c8e87: Tron's string 0x386
		if( m_nLoadFailedMsgId == ( uint32 )-1 )
			m_nLoadFailedMsgId = 0x386;

        ShowMessageBox( m_nLoadFailedMsgId, &mb );
	}
	else if (m_pLoadingScreen && !m_pLoadingScreen->IsActive() && m_pLoadingScreen->IsVisible())
	{
		// Update the loading screen if it's not active, but it is visible
		// so we can see the console on failure conditions.
		m_pLoadingScreen->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HideLoadScreen()
//
//	PURPOSE:	Called externally to hide the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::HideLoadScreen()
{
	// RETAIL: 0x100c8eb0
	if (m_pLoadingScreen)
		m_pLoadingScreen->Hide();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePausedState()
//
//	PURPOSE:	Update paused state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePausedState()
{
    g_pLTClient->Start3D();

	UpdateInterfaceSFX();

    g_pLTClient->StartOptimized2D();

	LTRect rcFull(0,0,g_pInterfaceResMgr->GetScreenWidth()-1,g_pInterfaceResMgr->GetScreenHeight()-1);
	g_pLTClient->FillRect(g_pLTClient->GetScreenSurface(),&rcFull,hBackColor);


	if (m_hGamePausedSurface)
	{
        g_pLTClient->ScaleSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hGamePausedSurface, &rcFull, LTNULL);
	}

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateSplashScreenState()
//
//	PURPOSE:	Update splash screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateSplashScreenState()
{
    g_fSplashSndDuration -= g_pGameClientShell->GetFrameTime();

	if(m_hSplashSound)
	{
		if((g_fSplashSndDuration <= 0.0f) || g_pLTClient->IsDone(m_hSplashSound))
		{
			g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());
			g_pLTClient->CPrint("Splash sound done playing...");

			g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
			m_hSplashSound = LTNULL;
		}
	}

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nWidth = 0;
    uint32 nHeight = 0;

    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

    LTRect rcDst;
	rcDst.Init(0, 0, nWidth, nHeight);

    g_pLTClient->GetSurfaceDims(g_hSplash, &nWidth, &nHeight);

    LTRect rcSrc;
	rcSrc.Init(0, 0, nWidth, nHeight);

    g_pLTClient->Start3D();
    g_pLTClient->StartOptimized2D();

    g_pLTClient->ScaleSurfaceToSurface(hScreen, g_hSplash, &rcDst, &rcSrc);

	UpdateScreenFade();

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();

    static LTBOOL bDidFadeOut = LTFALSE;

	if (!m_bScreenFade)
	{
		if (!bDidFadeOut && !m_hSplashSound)
		{
			// RETAIL: 0x100c911c: Tron fades out as soon as the sound is
			// done (no splash screen timer)
			StartScreenFadeOut(g_vtSplashScreenFadeOut.GetFloat());
			bDidFadeOut = LTTRUE;
		}
	}
	else if (bDidFadeOut && m_fCurFadeTime <= 0.0f)
	{
		EndSplashScreen( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::EndSplashScreen
//
//	PURPOSE:	Ends the splash screen.
//
// ----------------------------------------------------------------------- //

// RETAIL: 0x100c9326 -- Tron's string table has "Can't connect to the server"
// at 1155, not at NOLF2's 1283 (the name is ours).
#define IDS_TRON_CANT_CONNECT_TO_SERVER	1155

void CInterfaceMgr::EndSplashScreen()
{
	// If the user didn't start a join from the command line, proceed
	// like normal to the movies.
	if( !GetCommandLineJoin( ))
	{
		ChangeState(GS_MOVIE);
		return;
	}

	// If user is joining a mp game from the command line,
	// start the connection here.
	char szIP[256] = "";
	LTStrCpy( szIP, GetConsoleTempString( "join", "" ), ARRAY_LEN( szIP ));

	char szPassword[256];
	LTStrCpy( szPassword, GetConsoleTempString( "password", "" ), ARRAY_LEN( szPassword ));

	char szGameType[256] = "";
	LTStrCpy( szGameType, GetConsoleTempString( "gametype", "" ), ARRAY_LEN( szGameType ));

	// RETAIL: 0x100c9270: Tron has no default game type: a game type that
	// it does not know fails the join
	GameType eGameType;
	if( !GameStringTypeToGameType( szGameType, eGameType ))
	{
		// drop them into the join menu
		LoadFailed( SCREEN_ID_MAIN );

		MBCreate mb;
		ShowMessageBox(IDS_TRON_CANT_CONNECT_TO_SERVER, &mb);
		return;
	}

	g_pGameClientShell->SetGameType( eGameType );

	if( !szIP[0] || !g_pClientMultiplayerMgr->SetupClient( szIP, szPassword ) || !g_pMissionMgr->StartGameAsClient( ))
	{
		// drop them into the join menu
		LoadFailed( SCREEN_ID_MAIN );

		MBCreate mb;
		ShowMessageBox(IDS_TRON_CANT_CONNECT_TO_SERVER, &mb);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMovieState()
//
//	PURPOSE:	Update movie state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMovieState()
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr || !m_hMovie)
	{
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

	// Update and check if we're finished...

	if (pVideoMgr->UpdateVideo(m_hMovie) != LT_OK ||
		pVideoMgr->GetVideoStatus(m_hMovie) == LT_FINISHED)
	{
		NextMovie();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateDemoScreenState()
//
//	PURPOSE:	Update demo screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateDemoScreenState()
{
	if (m_bSeenDemoScreens)
	{
		if (m_bQuitAfterDemoScreens)
		{
			g_pLTClient->Shutdown();
		}
		else
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
		return;
	}

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nWidth = 0;
    uint32 nHeight = 0;

    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	// RETAIL: 0x100c9406: Tron reads the demo screen's size, then scales
	// the whole screen to the whole screen
    g_pLTClient->GetSurfaceDims(g_hDemo, &nWidth, &nHeight);

    g_pLTClient->Start3D();
    g_pLTClient->StartOptimized2D();

    g_pLTClient->ScaleSurfaceToSurface(hScreen, g_hDemo, LTNULL, LTNULL);

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetupMusic()
//
//	PURPOSE:	Handles changes to the state of the menu / level music
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SetupMusic()
{
	CMusic* pMusic = g_pGameClientShell->GetMusic();
    if (!pMusic) return LTFALSE;

	// RETAIL: 0x100c949a: Tron sets the music volume first
	m_ProfileMgr.ImplementMusicVolume();

	// Stop the music if it isn't enabled...

	if (!m_Settings.MusicEnabled() || !GetConsoleInt("MusicActive",0))
	{
		pMusic->Stop();
		return LTTRUE;
	}



	// Use the current level's music if we have a level loaded...

	// RETAIL: 0x100c9505: not after an intentional disconnect
	if ( g_pLTClient->IsConnected() && !m_bIntentionalDisconnect )
	{
		// Re-initialize music if necessary...

		if (!pMusic->IsInitialized())
		{
			if (!pMusic->Init(g_pLTClient))
			{
				return LTFALSE;
			}
		}
 
		g_pGameClientShell->RestoreMusic();

		pMusic->Play();
		
		return LTTRUE;
	}


	// Handle using the menu music if necessary...this should only occur if we
	// aren't in a level...

	CMusicState* pMS = pMusic->GetMusicState();

	int nIntensity = g_pLayoutMgr->GetScreenMusicIntensity(GetScreenMgr( )->GetCurrentScreenID());

	if (pMusic->IsPlaying())
	{
		if (pMS->nIntensity != nIntensity)
		{
			pMusic->ChangeIntensity(nIntensity);
		}
		return LTTRUE;
	}

	// Setup the menu music...

	char szFile[128] = "";
	g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicCtrlFile",szFile,sizeof(szFile));
    if (!strlen(szFile)) return LTFALSE;

	if (!pMusic->IsInitialized())
	{
        if (!pMusic->Init(g_pLTClient))
		{
            return LTFALSE;
		}
	}

	if (!pMusic->IsLevelInitialized())
	{
		char szDir[128] = "";
		g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicDir",szDir,sizeof(szDir));

		if (!strlen(szDir))
		{
            return LTFALSE;
		}

		CMusicState MusicState;
		MusicState.nIntensity = nIntensity;
		strcpy(MusicState.szDirectory, szDir);
		strcpy(MusicState.szControlFile, szFile);

		if (!pMusic->RestoreMusicState(MusicState))
		{
			return LTFALSE;
		}
	}

	pMusic->Play();

	if (pMS->nIntensity != nIntensity)
	{
		pMusic->ChangeIntensity(nIntensity);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnMessage()
//
//	PURPOSE:	Handle interface messages
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
	{
		return LTFALSE;
	}

	switch(messageID)
	{
		case MID_PLAYER_INFOCHANGE :
		{
            uint8 nThing    = pMsg->Readuint8();
            uint8 nType1    = pMsg->Readuint8();
            uint8 nType2    = pMsg->Readuint8();
            LTFLOAT fAmount  = pMsg->Readfloat();

			UpdatePlayerStats(nThing, nType1, nType2, fAmount);

            return LTTRUE;
		}
		break;

		case MID_PLAYER_SKILLS :
		{
			GetPlayerStats( )->UpdateSkills(pMsg);
			if (m_eGameState == GS_MENU)
			{
				m_MenuMgr.GetCurrentMenu()->OnCommand(MC_UPDATE,0,0);
            }
			return LTTRUE;
		}
		break;

		case MID_PLAYER_SUMMARY :
		{
            uint32 nID = pMsg->Readuint32();
            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);


			if (!IsMultiplayerGame() || nLocalID == nID)
			{
				GetPlayerStats( )->UpdateMissionStats(pMsg);
			}
			else
			{
				CLIENT_INFO *pClient;

				if(pClient = m_ClientInfo.GetClientByID(nID))
					pClient->sStats.ReadData(pMsg);

			}

//			if (m_eGameState == GS_MENU)
//				m_MenuMgr.GetCurrentMenu()->OnCommand(MC_UPDATE,0,0);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_SCORE :
		{
            uint32 nID = pMsg->Readuint32();
			CLIENT_INFO *pClient;

			if(pClient = m_ClientInfo.GetClientByID(nID))
			{
				pClient->sScore.ReadData(pMsg);
				m_ClientInfo.UpdateClientSort(pClient);
			}


            return LTTRUE;
		}
		break;


		case MID_GEAR_PICKEDUP :
		{
            uint8 nGearId = pMsg->Readuint8();
			bool bPickedUp = pMsg->Readbool();

			g_pGameClientShell->HandleGearPickup(nGearId, bPickedUp);

			if (bPickedUp)
			{
				GetPlayerStats( )->UpdateGear(nGearId);
			}
            return LTTRUE;
		}
		break;

		case MID_INTEL_PICKEDUP :
		{
			// [KLS 6/20/02] - Removed due to Craig request
			// g_pChatMsgs->AddMessage(LoadTempString(IDS_INTELLIGENCE));

			// RETAIL: Tron's message (0x100c9ab5): the type, a byte that
			// is not used, a flag and the item's name; no popup
			uint32 nTextId = pMsg->Readuint32();
			uint8 nType = pMsg->Readuint8();
			uint8 nPopupId = pMsg->Readuint8();
			uint8 nIsIntel = pMsg->Readuint8();
			pMsg->Readuint8();
			bool bNoHelpPing = pMsg->Readbool();
			char szName[128];
			pMsg->ReadString(szName, sizeof(szName));

			GetPlayerStats( )->UpdateIntel(nTextId,nPopupId,nIsIntel,nType,szName,bNoHelpPing);
		}
		break;


		case MID_DISPLAY_METER :
		{
            uint8 val = pMsg->Readuint8();
			g_pDisplayMeter->SetValue( val );
			g_pHUDMgr->QueueUpdate( kHUDDisplayMeter );
            return LTTRUE;
		}
		break;

		case MID_DECISION :
		{
			g_pDecision->Show(pMsg);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CONNECTED:
		{
			char szName[MAX_PLAYER_NAME];
            pMsg->ReadString( szName, ARRAY_LEN( szName ));
            uint32 nID = (uint32) pMsg->Readuint32();
			g_pLTClient->CPrint("MID_PLAYER_CONNECTED : Client  %d (%s)", nID, szName);
			// RETAIL: Tron adds the client here (0x100c9bf2)
			m_ClientInfo.AddClient( szName, nID );
            return LTTRUE;
		}
		break;


		case MID_PLAYER_INFO:
		{
			// Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

			char szName[MAX_PLAYER_NAME];
            pMsg->ReadString( szName, ARRAY_LEN( szName ));
            uint32 nID = pMsg->Readuint32();
			HOBJECT	hObj = pMsg->ReadObject();
			ModelId eModelId = (ModelId)pMsg->Readuint8();
			// RETAIL: 0x100c9c44, Tron's: the multiplayer model and its
			// color (no team, no admin flag); the names are ours
			uint8	nMPModel = pMsg->Readuint8();
			uint8	nColor = pMsg->Readuint8();
            uint8 nInfoType = pMsg->Readuint8();

            uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID (&nLocalID);


			// RETAIL: Tron's (0x100c9c01) has no team change handling here: the
			// client info keeps no team, and a join of the local client is
			// not added.
			switch (nInfoType)
			{
			case MID_PI_JOIN:
				{
					//new player entering
					if( nLocalID != nID )
					{
						g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_JOIN - Adding client %d (%s)", nID, szName);
						m_ClientInfo.AddClient(szName, nID);
						g_pGameClientShell->CSPrint(FormatTempString(IDS_JOINEDGAME, szName));
					}
				} break;
			case MID_PI_EXIST:
				{
					//we are joining and this is info about a player already in the game
					g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_EXIST - Adding client %d (%s)", nID, szName);
					m_ClientInfo.AddClient(szName, nID);
				} break;
			case MID_PI_UPDATE:
				{
					//this is an update about a player already in the game
					g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_UPDATE - Updating client %d (%s)", nID, szName);
					m_ClientInfo.UpdateClient(szName, nID);
				} break;
			};

			CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	        if (psfxMgr)
			{
				CCharacterFX* pCFX = (CCharacterFX*)psfxMgr->FindSpecialFX(SFX_CHARACTER_ID, hObj);
				if (pCFX)
				{
					// RETAIL: Tron's (0x100c9d6a) makes the attach fx again,
					// then changes the model.
					pCFX->RecreateAttachClientFX();
					pCFX->ChangeModel(eModelId, nMPModel, nColor);
				}
			}


            return LTTRUE;
		}
		break;

		case MID_PLAYER_REMOVED:
		{
			// Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

            uint32 nID = pMsg->Readuint32();

			//if it's us we know we're leaving
			if (nID == nLocalID)
				return LTTRUE;

			//if we don't know who it is, let's not display our ignorance...
			if (!m_ClientInfo.GetClientByID(nID,false))
				return LTTRUE;

            g_pGameClientShell->CSPrint(FormatTempString(IDS_LEFTGAME, m_ClientInfo.GetPlayerName(nID)));

			m_ClientInfo.RemoveClient(nID);

            return LTTRUE;
		}
		break;

		case MID_PINGTIMES:
		{
			while(1)
			{
                uint16 id, ping;
				CLIENT_INFO *pClient;

                id = pMsg->Readuint16();
				if(id == 0xFFFF)
					break;

                ping = pMsg->Readuint16();
				if(pClient = m_ClientInfo.GetClientByID(id,false))
					pClient->nPing = ping;
			}

            return LTTRUE;
		}
		break;

		case MID_PLAYER_SCORED:
		{
			//Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID(&nLocalID);

			bool bFragged = pMsg->Readbool( );
            uint32 nVictim = pMsg->Readuint32();
            uint32 nScorer = pMsg->Readuint32();

			CLIENT_INFO *pVictim = m_ClientInfo.GetClientByID(nVictim);
			CLIENT_INFO *pScorer = m_ClientInfo.GetClientByID(nScorer);

            char szTmp[128] = "";
			if (nVictim == nLocalID)
			{
				// Scored againsts self.
				if (nVictim == nScorer)
				{
					if( bFragged )
						FormatString(IDS_KILLEDMYSELF,szTmp,sizeof(szTmp));
					else
	                    FormatString(IDS_TAGGEDMYSELF,szTmp,sizeof(szTmp));
				}
				// Scored by ai.
				else if( nScorer == -1 )
				{
					if( bFragged )
	                    FormatString(IDS_IWASKILLED,szTmp,sizeof(szTmp));
				}
				// Killed by player.
				else
				{
					if( bFragged )
	                    FormatString(IDS_HEKILLEDME, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer));
					else
						FormatString(IDS_HETAGGEDME, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer));
				}

				g_pChatMsgs->AddMessage(szTmp);
			}
			else if (nScorer == nLocalID)
			{
				if( bFragged )
	                FormatString(IDS_IKILLEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nVictim));
				else
					FormatString(IDS_ITAGGEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nVictim));

                g_pLTClient->CPrint(szTmp);
				g_pChatMsgs->AddMessage(szTmp);//,eType);
			}
			else
			{
				if (nVictim == nScorer)
				{
					if( bFragged )
		                FormatString(IDS_HEKILLEDHIMSELF, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
					else
						FormatString(IDS_HETAGGEDHIMSELF, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
				}
				else if( nScorer == -1 )
				{
					if( bFragged )
	                    FormatString(IDS_HEWASKILLED, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
				}
				else
				{
					if( bFragged )
	                    FormatString(IDS_HEKILLEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
					else
						FormatString(IDS_HETAGGEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
				}

                g_pLTClient->CPrint(szTmp);
				g_pChatMsgs->AddMessage(szTmp);
			}

            return LTTRUE;
		}
		break;


		case MID_PLAYER_MESSAGE :
		case MID_PLAYER_GHOSTMESSAGE :
		{
			// retrieve the string from the message, play the chat sound, and display the message

			char szMessage[256];
			pMsg->ReadString(szMessage, sizeof(szMessage));
			uint32 clientID = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);


			if( IsTeamGameType() && nTeam != INVALID_TEAM )
			{
				CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
				if( !pLocalCI )
					return LTFALSE;

				// PORT: Tron's CLIENT_INFO has no nTeamID; the team manager has it
				if( (uint8)g_pInterfaceMgr->GetTeamMgr()->GetTeamOfClient(pLocalCI->nID) != nTeam )
					return LTFALSE;
			}

            g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
			
			if (IsMultiplayerGame())
			{
				// The dedicated server will have a ClientID of -1...

				if( clientID == -1 )
				{
					g_pChatMsgs->AddMessage( szMessage, kMsgChat );
				}
				else
				{

					char szTemp[256];
					sprintf(szTemp,"%s : %s", m_ClientInfo.GetPlayerName(clientID), szMessage);

					if( IsTeamGameType() && nTeam != INVALID_TEAM )
					{
						g_pChatMsgs->AddMessage(szTemp,kMsgTeam);
					}
					else
					{
						g_pChatMsgs->AddMessage(szTemp,kMsgChat);
					}
				}
			}
			else
			{
				g_pChatMsgs->AddMessage(szMessage,kMsgChat);
			};

			
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CREDITS :
		{
            uint8 nMsg = pMsg->Readuint8();
			switch (nMsg)
			{
			case 0:
				m_Credits.Term();
				break;
			case 1:
				m_Credits.Init();
				break;
			case 2:
				m_Credits.Init(CM_INTRO);
				break;
			}
		}
		break;


		case MID_PLAYER_TRANSMISSION :
		{
			// retrieve the string from the message, play the chat sound, and display the message
			uint32 nActivePlayerClientId = pMsg->Readuint32( );
            uint32 dwId = pMsg->Readuint32();
            uint32 nSound = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

			if( IsTeamGameType() && nTeam != INVALID_TEAM )
			{
				CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
				if( !pLocalCI )
					return LTFALSE;

				// PORT: Tron's CLIENT_INFO has no nTeamID; the team manager has it
				if( (uint8)g_pInterfaceMgr->GetTeamMgr()->GetTeamOfClient(pLocalCI->nID) != nTeam )
					return LTFALSE;
			}


			if (nSound)
			{
				char szStr[128] = "";
				g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nSound, szStr, sizeof(szStr));

				g_pClientSoundMgr->PlaySoundLocal(szStr,SOUNDPRIORITY_PLAYER_HIGH);
			}
			else
			{
				g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\transmission.wav");
			}

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

			// Check if there wasn't an active player for this transmission.
			if( nActivePlayerClientId == -1 || nActivePlayerClientId == nLocalID )
			{
				g_pTransmission->Show(dwId);
				g_pChatMsgs->AddMessage(dwId,kMsgTransmission);
			}
			// There was an active player with this transmission.  Format it like this:
			// "<PlayerName>: <Message>".
			else
			{
				CString sTransmission;
				sTransmission.Format( "%s: %s", m_ClientInfo.GetPlayerName( nActivePlayerClientId ),
					LoadTempString( dwId ));
				
				g_pTransmission->Show( sTransmission );
				g_pChatMsgs->AddMessage(sTransmission,kMsgTransmission);

			}

            return LTTRUE;
		}
		break;


		case MID_PLAYER_OVERLAY :
		{
			// retrieve the string from the message, play the chat sound, and display the message

            uint8 nId = pMsg->Readuint8();
            bool bOn = !!pMsg->Readuint8();

			if (nId < NUM_OVERLAY_MASKS)
			{
				if (bOn)
					CreateOverlay((eOverlayMask)nId);
				else
					RemoveOverlay((eOverlayMask)nId);
				return LTTRUE;
			}
			return LTFALSE;
		}
		break;

		case MID_LOAD_FAILED:
		{
			LoadFailed();
			return LTFALSE; 
		}
		break;
		
		// RETAIL: no MID_DOOMSDAY_MESSAGE, MID_PLAYER_TEAM or MID_TEAM_INFO in
		// Tron (retail 0x100c9770's cases: 142 and 127 are Tron's own ids)


		default : break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	switch (dwEventID)
	{
		// Called when the renderer has switched into
		// the new mode but before it reloads all the textures
		// so you can display a loading screen.

		case LTEVENT_RENDERALMOSTINITTED :
		{
			if (m_bSwitchingModes)
			{
				ClearAllScreenBuffers();


				// RETAIL: 0x100ca56e: Tron's string 0x38f
				m_InterfaceResMgr.DrawMessage(0x38f);

				// because of driver bugs, we need to wait a frame after reinitializing the renderer and
				// reinitialize the cursor
				m_CursorMgr.ScheduleReinit(2.0f);
			}
		}
		break;

		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			// RETAIL: 0x100ca595: NOLF2 reset its radar and its client list
			// here; Tron does neither (OnExitWorld empties the list)
			if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			{
				Disconnected(dwParam);
			}
			m_bIntentionalDisconnect = false;
		}
		break;

		// Engine shutting down.  dwParam will be a error
		// flag found in de_codes.h.

		case LTEVENT_SHUTDOWN :
		break;

		// The renderer was initialized.  This is called if
		// you call SetRenderMode or if focus was lost and regained.

		case LTEVENT_RENDERINIT :
		{
			if (m_pLoadingScreen && m_pLoadingScreen->IsVisible())
			{
				// Hide it to get rid of any resources that may have been around
				m_pLoadingScreen->Hide();
				// And then start it up again
				m_pLoadingScreen->Show();
			}
		}
		break;

		// The renderer is being shutdown.  This happens when
		// ShutdownRender is called or if the app loses focus.

		case  LTEVENT_RENDERTERM :
		{
			// Stop drawing the loading screen
			if (m_pLoadingScreen && m_pLoadingScreen->IsVisible())
				m_pLoadingScreen->Pause();
		}
		break;

        case LTEVENT_LOSTFOCUS:
		case LTEVENT_GAINEDFOCUS:
		break;


		default :
		{
            uint32 nStringID = IDS_UNSPECIFIEDERROR;
			SwitchToScreen(SCREEN_ID_MAIN);
			//DoMessageBox(nStringID, TH_ALIGN_CENTER);
		}
		break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

// ABM TODO move this function out of base class into TronInterfaceMgr.
// RETAIL: Tron's "out of ammo" string id (0x100ca87c; the name is ours)
#define IDS_TRON_OUTOFAMMO	601

// RETAIL: 0x100ca82a -- Tron's GetWeaponString loads the weapon's name
// from its string id into a buffer (0x1027c720). The name is ours.
inline char* GetTronWeaponString(uint8 nWeaponId)
{
	static char s_szName[128];

    if (!g_pLTClient || !g_pWeaponMgr) return WS_ERROR_STRING;

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return WS_ERROR_STRING;

	LoadString(pWeapon->nNameId, s_szName, sizeof(s_szName));
	return s_szName;
}

void CInterfaceMgr::UpdatePlayerStats(uint8 nThing, uint8 nType1,
                                      uint8 nType2, LTFLOAT fAmount)
{
	switch (nThing)
	{
		case IC_WEAPON_OBTAIN_ID :
		{
            GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE, LTFALSE);
		}
		break;

		case IC_WEAPON_PICKUP_ID :
		{
			if (nType2 != WMGR_INVALID_ID)
			{
				GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE);
			}
			else
			{
				g_pGameClientShell->HandleWeaponPickup(nType1,false);
			}
		}
		break;

		case IC_MOD_PICKUP_ID :
		{
			bool bPickedUp = (bool) !!(nType1);
			bool bDisplayMsg = (bool) !!(fAmount);

			if (bDisplayMsg)
			{
				g_pGameClientShell->HandleModPickup(nType2, bPickedUp);
			}

			if (bPickedUp)
			{
				GetPlayerStats( )->UpdateMod(nType2);
			}
		}
		break;

		case IC_DEFAULTWEAPON_ID :
		{
			CClientWeaponMgr* pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			pClientWeaponMgr->SetDefaultWeapon( nType1 );
		}
		break;

		case IC_AMMO_ID :
		{
            GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount);
		}
		break;

		case IC_MAX_HEALTH_ID :
		{
            GetPlayerStats( )->UpdateMaxHealth((uint32)fAmount);
		}
		break;

		case IC_MAX_ENERGY_ID :
		{
           // PORT: UpdateMaxEnergy is not in Tron's CPlayerStats interface (Tron's
			// energy is CTronPlayerStats'); NOLF2's pickup code stands in
			// GetPlayerStats( )->UpdateMaxEnergy((uint32)fAmount);
		}
		break;

		case IC_ENERGY_ID :
		{
           // PORT: as UpdateMaxEnergy (above)
			// GetPlayerStats( )->UpdateEnergy((uint32)fAmount);
		}
		break;

		case IC_MAX_ARMOR_ID :
		{
            GetPlayerStats( )->UpdateMaxArmor((uint32)fAmount);
		}
		break;

		case IC_HEALTH_ID :
		{
            GetPlayerStats( )->UpdateHealth((uint32)fAmount);
		}
		break;

		case IC_ARMOR_ID :
		{
            GetPlayerStats( )->UpdateArmor((uint32)fAmount);
		}
		break;

		case IC_AIRLEVEL_ID :
		{
			GetPlayerStats( )->UpdateAir(fAmount);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{
			// RETAIL: 0x100ca82a -- Tron's weapon name is its string id
			g_pChatMsgs->AddMessage(FormatTempString(IDS_TRON_OUTOFAMMO, GetTronWeaponString(nType1)));
		}
		break;

		case IC_OBJECTIVE_ID :
		case IC_OPTION_ID :
		case IC_PARAMETER_ID :
		{
            GetPlayerStats( )->UpdateObjectives(nThing, nType1, (uint32)fAmount);
		}
		break;

		case IC_KEY_ID :
		{
            GetPlayerStats( )->UpdateKeys(nType1, (uint16)fAmount);
		}
		break;

		case IC_SKILLS_ID :
		{
            // RETAIL: 0x100ca8f6: the reward only
            GetPlayerStats( )->GainSkills(nType1);
		}
		break;

		case IC_FADE_SCREEN_ID :
		{
			// Fade the screen in or out...

			// RETAIL: 0x100ca913 -- Tron passes nType2 on as the fades' flag
			if (nType1)
			{
				StartScreenFadeIn(fAmount, nType2 != 0);
			}
			else
			{
				StartScreenFadeOut(fAmount, nType2 != 0);
			}
		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			GetPlayerStats( )->DropInventory((LTBOOL)nType1);
		}
		break;


		case IC_MISSION_TEXT_ID :
		{
			if (fAmount > 0.0f)
				g_pMissionText->Start((int)fAmount);
			else
				g_pMissionText->Clear();
		}
		break;

		case IC_MISSION_FAILED_ID :
		{
			MissionFailed((int)fAmount);
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnCommandOn(int command)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;
     
	// Take appropriate action
	if (	g_pDecision->IsVisible()	&&
			command >= COMMAND_ID_CHOOSE_1 &&
			command <= COMMAND_ID_CHOOSE_6 )
	{
		uint8 nChoice = command - COMMAND_ID_CHOOSE_1;
		g_pDecision->Choose(nChoice);
        return LTTRUE;
	}


	if (GS_MENU == m_eGameState || GS_PLAYING == m_eGameState)
	{
		if (m_MenuMgr.OnCommandOn(command) || GS_MENU == m_eGameState)
			return LTTRUE;
	}


	switch (command)
	{

		case COMMAND_ID_HOLSTER :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (GetWeaponChooser()->IsOpen())
			{
				GetWeaponChooser()->Close();
			}

            return LTTRUE;

		}
		break;

		// RETAIL: no last weapon command in Tron (0x100caa9b's table)


		case COMMAND_ID_PREV_WEAPON :
		{
			// RETAIL: 0x100cab01: no weapons enabled check and no gadget
			// disabling to wait for in Tron

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (GetWeaponChooser()->Open(0))
			{
				GetWeaponChooser()->PrevWeapon();
			}

            return LTTRUE;

		}
		break;

		case COMMAND_ID_NEXT_WEAPON	:
		{
			NextWeapon(command);
            return LTTRUE;
		} break;

		case COMMAND_ID_NEXT_WEAPON_1 :
		case COMMAND_ID_NEXT_WEAPON_2 :
		case COMMAND_ID_NEXT_WEAPON_3 :
		case COMMAND_ID_NEXT_WEAPON_4 :
		case COMMAND_ID_NEXT_WEAPON_5 :
		case COMMAND_ID_NEXT_WEAPON_6 :
		{
			if (!g_pDecision->IsVisible())
				NextWeapon(command);
            return LTTRUE;
		}
		break;

		case COMMAND_ID_ACTIVATE :
		{
			// RETAIL: 0x100cab6d: activate closes an unlocked popup
			if (m_PopupText.IsVisible() && !g_bLockPopup)
			{
				m_PopupText.Close();
				return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();

			// RETAIL: no gadget disabling to wait for in Tron

			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

			if (GetWeaponChooser()->IsOpen())
			{
				GetWeaponChooser()->Close();
			}
			if (m_AmmoChooser.Open())
			{
				m_AmmoChooser.NextAmmo();
			}
            return LTTRUE;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			// RETAIL: 0x100cac03: firing chooses the decision's 0xfd
			if (g_pDecision->IsVisible())
			{
				g_pDecision->Choose(0xfd);
			}

			if (IsChoosingWeapon() || IsChoosingAmmo())
			{
                return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_NEXT_LAYOUT :
		{
			if (m_eGameState == GS_PLAYING)
			{
				// RETAIL: 0x100cac4e: no sound in Tron
				GetHUDMgr()->NextLayout();
			}
            return LTTRUE;

		}
		break;
		case COMMAND_ID_PREV_LAYOUT :
		{
			if (m_eGameState == GS_PLAYING)
			{
				// RETAIL: 0x100cac4e: no sound in Tron
				GetHUDMgr()->PrevLayout();
			}
            return LTTRUE;

		}
		break;

		// RETAIL: Tron's scores are the light cycle match manager's to show
		// (NOLF2's mission key showed them here)

		default :
		break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnCommandOff(int command)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Only process if not editing a message...

    if (g_pChatInput->IsVisible()) return LTTRUE;

	if (m_MenuMgr.OnCommandOff(command))
		return LTTRUE;


	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
			// RETAIL: 0x100cad75: activate up unlocks the popup
			g_bLockPopup = false;
            return LTFALSE;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsChoosingWeapon())
			{
                uint8 nCurrWeapon = GetWeaponChooser()->GetCurrentSelection();
				GetWeaponChooser()->Close();
				g_pPlayerMgr->ChangeWeapon( nCurrWeapon );

                return LTTRUE;

			}
			else if (IsChoosingAmmo())
			{
                uint8 nCurrAmmo = m_AmmoChooser.GetCurrentSelection();
				m_AmmoChooser.Close();

				IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
				if ( pClientWeapon )
				{
					pClientWeapon->ChangeAmmoWithReload( nCurrAmmo );
				}

                return LTTRUE;
			}

		}
		break;

		case COMMAND_ID_HOLSTER :
		{
			ASSERT( 0 != g_pPlayerMgr );
			g_pPlayerMgr->ToggleHolster( true );
			return LTTRUE;
		}
		break;

		// RETAIL: no last weapon command in Tron (0x100caa10)

		case COMMAND_ID_NEXT_WEAPON :
		case COMMAND_ID_NEXT_WEAPON_1 :
		case COMMAND_ID_NEXT_WEAPON_2 :
		case COMMAND_ID_NEXT_WEAPON_3 :
		case COMMAND_ID_NEXT_WEAPON_4 :
		case COMMAND_ID_NEXT_WEAPON_5 :
		case COMMAND_ID_NEXT_WEAPON_6 :
		{
			// End the timer for quick weapon changing...
			GetWeaponChooser()->EndAutoSwitch();
		}
		break;

		case COMMAND_ID_PREV_WEAPON :
		{
			// End the timer for quick weapon changing...
			GetWeaponChooser()->EndAutoSwitch(false);
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
			// End the timer for quick ammo changing...
			m_AmmoChooser.EndAutoSwitch();
		}
		break;

		// RETAIL: Tron's scores are the light cycle match manager's to show
		// (NOLF2's mission key showed them here)

		default : break;
	}

    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnKeyDown(int key, int rep)
{
	// Make sure we're initialized.
	if ( !IsInitialized( ) )
	{
		return LTFALSE;
	}

	if (m_MessageBox.IsVisible())
	{
		return m_MessageBox.HandleKeyDown(key,rep);
	}

	switch (m_eGameState)
	{
		case GS_SCREEN :
		{
			// RETAIL: 0x100caf39: escape stops the credits
			if (m_Credits.IsInited())
			{
				if (key == VK_ESCAPE)
				{
					m_Credits.Term();
				}
			}
			else
			{
				GetScreenMgr( )->HandleKeyDown(key,rep);
			}
            return LTTRUE;
		}
		break;

		case GS_PAUSED :
		{
			if( HandleDebugKey(key) )
			{
				return LTTRUE;
			}

			// They pressed a key - unpause the game

			ChangeState(GS_PLAYING);
            return LTTRUE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
			// They pressed a key - end splash screen...
 			EndSplashScreen( );
			return LTTRUE;
		}
		break;

		case GS_MOVIE :
		{
			// They pressed a key - end splash screen...

			NextMovie();
            return LTTRUE;
		}
		break;

		case GS_DEMOSCREEN :
		{
			// They pressed a key - go to next screen...
			NextDemoScreen();
            return LTTRUE;
		}
		break;


		case GS_POPUP :
		{
			// RETAIL: 0x100caffe: only escape closes the popup
			if(m_PopupText.IsVisible() && key == VK_ESCAPE)
			{
				m_PopupText.Close();
				return LTTRUE;
			}
		}
		break;

		case GS_MENU :
		{
			m_MenuMgr.HandleKeyDown(key,rep);
			return LTTRUE;
		}
		break;

		// ABM TODO write handlers for the subroutine and ratings states

		case GS_PLAYING :
		{
			// Are We Broadcasting a Message
			if (g_pChatInput->IsVisible())
			{
				g_pChatInput->HandleKeyDown(key, rep);
				return LTTRUE;
			}

			switch (key)
			{
				case VK_PAUSE:
				{
					if (IsMultiplayerGame()) return LTFALSE;

					if (!g_pGameClientShell->IsGamePaused())
					{
						ChangeState(GS_PAUSED);
					}

					g_pGameClientShell->PauseGame(!g_pGameClientShell->IsGamePaused(), LTTRUE);
					return LTTRUE;
				}
				break;

				// Escape Key Handling
				case VK_ESCAPE:
				{
					bool bHandled = false;
					if (!g_pPlayerMgr->IsPlayerDead() && g_pPlayerMgr->IsPlayerInWorld())
					{
						if (IsChoosingWeapon())
						{
							// RETAIL: Tron's keeps the HUD weapon (0x100cb0ba)
							GetWeaponChooser()->Close(false);
							bHandled = true;
						}
						if (IsChoosingAmmo())
						{
							m_AmmoChooser.Close();
							bHandled = true;
						}
					}

					if (g_pDecision->IsVisible())
					{
						g_pDecision->Choose(MAX_DECISION_CHOICES);
						// RETAIL: 0x100cb0f2
						return LTTRUE;
					}

					if (bHandled)
					{
						return LTTRUE;
					}
				}
				break;

				default : break;
			}

			if ( m_MenuMgr.HandleKeyDown(key,rep) )
			{
				return LTTRUE;
			}

			if ( HandleDebugKey(key) )
			{
				return LTTRUE;
			}

		}
		break;

		default : break;
	}


	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnKeyUp(int key)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing
	if (key == VK_TILDE)
	{
		AddToClearScreenCount();
        return LTTRUE;
	}

	if (g_pChatInput->IsVisible())
	{
        return LTTRUE;
	}

	switch (m_eGameState)
	{
	case GS_SCREEN:
		{
			// RETAIL: 0x100cb24a: not while the credits run
			if (!m_Credits.IsInited())
				GetScreenMgr( )->HandleKeyUp(key);
			return LTTRUE;
		}
		break;

	case GS_MENU:
		{
			m_MenuMgr.HandleKeyUp(key);
			return LTTRUE;
		}
		break;

	case GS_LOADINGLEVEL :
		{
            return LTTRUE;
		}
		break;
	case GS_POPUP:
		{
			// RETAIL: 0x100cb283: any key up unlocks the popup (Tron keeps
			// no list of the keys held down)
			g_bLockPopup = false;
		} break;

	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnChar()
//
//	PURPOSE:	Handle OnChar messages
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnChar(unsigned char c)
{
	// RETAIL: 0x100cb2bb compares signed
	if ((char)c < ' ') return;


	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.HandleChar(c);
		return;
	}

	if (g_pChatInput->IsVisible())
	{
		g_pChatInput->HandleChar(c);
		return;
	}
	// RETAIL: 0x100cb2fb: not while the credits run
	if (m_eGameState == GS_SCREEN && !m_Credits.IsInited())
	{
		GetScreenMgr( )->HandleChar(c);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Draw()
//
//	PURPOSE:	Draws any interface stuff that may need to be drawn
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::Draw()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Tint the screen if necessary...

	m_FullScreenTint.SetAlpha(g_vtPauseTintAlpha.GetFloat());
	m_FullScreenTint.Draw(g_pLTClient->GetScreenSurface());

	// Check for interface drawing override...

	// RETAIL: 0x100cb320: Tron draws the light cycle countdown and the
	// spectator's text with the HUD, the letter box after them, the paused
	// item in a multiplayer game too and the subtitles last; its message
	// box needs no 3D block, and the sprites update here
	if (g_vtDrawInterface.GetFloat())
	{
		if (GetGameState() == GS_PLAYING || GetGameState() == GS_POPUP)
		{
			// Draw the player stats (health,armor,ammo) if appropriate...
			GetHUDMgr()->Render();

			DrawTronUnknownMessage();

			if (m_Credits.IsInited())
			{
				if (m_Credits.IsDone())
				{
					m_Credits.Term();
				}
				else
				{
					m_Credits.Update();
				}
			}

			if (m_InterfaceTimer.GetTime() > 0.0f)
			{
				m_InterfaceTimer.Draw();
			}

			DrawSpectatorText();
		}

		// Update letter box...

		UpdateLetterBox();

		if( g_pDamageFXMgr->IsDamageActive( DamageTypeToFlag(DT_SLEEPING) ))
		{
			if (IsChoosingWeapon())
			{
				GetWeaponChooser()->Close();
			}

			if (IsChoosingAmmo())
			{
				m_AmmoChooser.Close();
			}
		}

		if (GetGameState() == GS_PLAYING || GetGameState() == GS_POPUP || GetGameState() == GS_MENU)
		{
			g_pPaused->Render();
		}

		// Update the screen fade alpha only if we are in the playing state and not paused...

		bool bUpdateAlpha = (GetGameState() == GS_PLAYING ? !g_pGameClientShell->IsGamePaused() : false);

		UpdateScreenFade(bUpdateAlpha);

		g_pSubtitles->Draw();
	}

	if (GetGameState() == GS_MENU)
	{
		m_MenuMgr.Render();
	}
	if (GetGameState() == GS_POPUP)
	{
		m_PopupText.Draw();
	}

	//this should be last so it is always on top.
	if (m_MessageBox.IsVisible())
	{
        g_pLTClient->StartOptimized2D();
        m_MessageBox.Draw();
        g_pLTClient->EndOptimized2D();
	}

	m_CursorMgr.Update();
	m_ScreenSpriteMgr.Update();

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AbortScreenFade()
//
//	PURPOSE:	Cancel the screen fade. RETAIL: inline (PreScreenState,
//				0x100cb63b): retail has no copy of it.
//
// --------------------------------------------------------------------------- //

inline void CInterfaceMgr::AbortScreenFade()
{
	if (m_hFadeSurface)
	{
        g_pLTClient->DeleteSurface(m_hFadeSurface);
        m_hFadeSurface = LTNULL;
	}

	m_bFadeInitialized  = LTFALSE;
	m_bScreenFade       = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreScreenState()
//
//	PURPOSE:	Initialize the Screen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreScreenState(GameState eCurState)
{
    if (eCurState == GS_SCREEN) return LTFALSE;

	m_InterfaceResMgr.Setup();

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->Disable();
	g_pGameClientShell->GetScreenTintMgr()->ClearAll();


	CreateInterfaceBackground();

    SetHUDRenderLevel(kHUDRenderNone);
	ClearScreenAlways();

	// No fog in the menus...
	g_bInGameFogEnabled = (LTBOOL) GetConsoleInt("FogEnable", 1);
	WriteConsoleInt("FogEnable", 0);

	// Make sure menus and screens are full screen...
    memset(&m_rcMenuRestoreCamera, 0, sizeof (LTRect));

    uint32 nWidth, nHeight;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);

	m_bUseInterfaceCamera = LTTRUE;

	if (eCurState == GS_LOADINGLEVEL) 
	{
		AbortScreenFade();
	}
	else
	{
		ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
	}

	m_bEnteredScreenState = true;
	
	// Initialize the music...

	if (!s_bKeepGameMusic)
		SetupMusic();

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostScreenState()
//
//	PURPOSE:	Handle leaving the Screen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostScreenState(GameState eNewState)
{
    if (eNewState == GS_SCREEN) return LTFALSE;

	GetScreenMgr( )->ExitScreens();

	if (eNewState != GS_LOADINGLEVEL && eNewState != GS_DEMOSCREEN)
	{
		int nGameMode = GAMEMODE_NONE;
        g_pLTClient->GetGameMode(&nGameMode);
        if (nGameMode == GAMEMODE_NONE) return LTFALSE;
	}

    ClearScreenAlways(LTFALSE);
	AddToClearScreenCount();


	RemoveAllInterfaceSFX();

	// RETAIL: 0x100cb73d: Tron restores the full HUD, dead or not
	SetHUDRenderLevel(kHUDRenderFull);

	// m_InterfaceResMgr.Clean();

    g_pLTClient->ClearInput();


	// Reset fog value...

	WriteConsoleInt("FogEnable", (int) g_bInGameFogEnabled);

	m_bSuppressNextFlip = true;

	// RETAIL: 0x100cb76c: no unpause; Tron holds the music stopped while a
	// level loads, and plays it again after
	if (eNewState == GS_LOADINGLEVEL)
	{
		CMusic* pMusic = g_pGameClientShell->GetMusic();
		if (pMusic)
		{
			pMusic->Stop();
			pMusic->m_bSuspended = true;
		}
	}
	else
	{
		CMusic* pMusic = g_pGameClientShell->GetMusic();
		if (pMusic && pMusic->m_bSuspended)
		{
			pMusic->m_bSuspended = false;
			pMusic->Play();
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePauseState()
//
//	PURPOSE:	Initialize the Pause state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePauseState(GameState eCurState)
{
    if (eCurState == GS_PAUSED) return LTFALSE;

	CreateInterfaceBackground();

	// Create the "paused" surface...
	m_hGamePausedSurface = g_pInterfaceResMgr->GetSharedSurface("interface\\pause.pcx");

	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPauseState()
//
//	PURPOSE:	Handle leaving the Pause state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPauseState(GameState eNewState)
{
    if (eNewState == GS_PAUSED) return LTFALSE;

	RemoveAllInterfaceSFX();

	// Remove the "paused" surface...
	if (m_hGamePausedSurface)
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = LTNULL;
	}



    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePlayingState()
//
//	PURPOSE:	Initialize the Playing state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePlayingState(GameState eCurState)
{
    if (eCurState == GS_PLAYING) return LTFALSE;

	if (eCurState != GS_MENU)
	{
		// Unpause the game...

        g_pGameClientShell->PauseGame(LTFALSE);
	}

	// Clear whatever input got us here...
	g_pLTClient->ClearInput();

	// Eanble light scaling...

	g_pGameClientShell->GetLightScaleMgr()->Enable();
	g_pPlayerMgr->RestorePlayerModes();

	g_pMissionText->Pause(LTFALSE);

    m_bUseInterfaceCamera = LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPlayingState()
//
//	PURPOSE:	Handle leaving the Playing state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPlayingState(GameState eNewState)
{
    if (eNewState == GS_PLAYING) return LTFALSE;

	g_pGameClientShell->GetScreenTintMgr()->ClearAll();
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

    g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);

	g_pMissionText->Pause(LTTRUE);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMenuState()
//
//	PURPOSE:	Initialize the menu state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreMenuState(GameState eCurState)
{
    if (eCurState == GS_MENU) return LTFALSE;

	if (!m_MenuMgr.GetCurrentMenu())
	{
		if (!m_MenuMgr.GetLastMenu())
			return LTFALSE;

		m_MenuMgr.SetCurrentMenu(m_MenuMgr.GetLastMenu()->GetMenuID());
	}

	m_bUseInterfaceCamera = LTFALSE;

	// [KLS 7/28/02] Shouldn't need to fade in for the menu, screen fade
	// is drawn behind the menu...
	// ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);
	g_pLTClient->SetInputState(LTTRUE);

	// If we're currently viewing a cinematic disable all the menu bar menus
	// except the system menu...

	// RETAIL: 0x100cb9b5: and in multiplayer games
	if (g_pPlayerMgr->IsUsingExternalCamera() || IsMultiplayerGame())
	{
		m_MenuMgr.EnableMenuBar(false, (MB_ALL & ~MB_SYSTEM));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMenuState()
//
//	PURPOSE:	Handle leaving the menu state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostMenuState(GameState eNewState)
{
    if (eNewState == GS_MENU) return LTFALSE;

	m_MenuMgr.ExitMenus();

	// Unpause the game...
    g_pGameClientShell->PauseGame(LTFALSE);
	g_pLTClient->ClearInput();

	// If we're currently viewing a cinematic re-enable the menu bar...

	if (g_pPlayerMgr->IsUsingExternalCamera())
	{
		m_MenuMgr.EnableMenuBar(true);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePopupState()
//
//	PURPOSE:	Initialize the Popup state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePopupState(GameState eCurState)
{
    if (eCurState == GS_POPUP) return LTFALSE;

	// Pause the game...
	if ( !IsMultiplayerGame( ) )
	{
		g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);
	}

	g_pPlayerMgr->AllowPlayerMovement(LTFALSE);
	g_pPlayerMgr->ClearPlayerModes(LTTRUE);

	m_bUseInterfaceCamera = LTFALSE;

	m_ePrePopupHUDRenderLevel = GetHUDMgr()->GetRenderLevel();

	if (m_bHideHUDInPopup)
	{	
		SetHUDRenderLevel(kHUDRenderNone);
	}

	// RETAIL: 0x100cbabc: Tron locks the popup until a key comes up (no
	// list of the keys held down)
	g_bLockPopup = true;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPopupState()
//
//	PURPOSE:	Handle leaving the Popup state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPopupState(GameState eNewState)
{
    if (eNewState == GS_POPUP) return LTFALSE;

	g_pPlayerMgr->AllowPlayerMovement(LTTRUE);
	g_pLTClient->ClearInput();

	SetHUDRenderLevel(m_ePrePopupHUDRenderLevel);
	m_bHideHUDInPopup = false;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreLoadingLevelState()
//
//	PURPOSE:	Initialize the LoadingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreLoadingLevelState(GameState eCurState)
{
    if (eCurState == GS_LOADINGLEVEL) return LTFALSE;

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

	RemoveInterfaceFX();
	m_InterfaceFXMgr.UseSystemTime(true);

	m_bUseInterfaceCamera = LTTRUE;

	if (m_pLoadingScreen)
		m_pLoadingScreen->Show();
		
	// Turn off the music (this will be turned on when we start the
	// next level...
	//CMusic* pMusic = g_pGameClientShell->GetMusic();
    //if (pMusic)
	//{
	//	pMusic->Stop();
	//}

	// Turn off sound
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetVolume(0);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostLoadingLevelState()
//
//	PURPOSE:	Handle leaving the LoadingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostLoadingLevelState(GameState eNewState)
{
    if (eNewState == GS_LOADINGLEVEL) return LTFALSE;

	// Don't allow the loading state to go away until the loading screen has been hidden
	if (m_pLoadingScreen && m_pLoadingScreen->IsVisible())
		return LTFALSE;

	RemoveAllInterfaceSFX();
	m_InterfaceFXMgr.UseSystemTime(false);

	// Turn back on the sound
	m_ProfileMgr.ImplementSoundVolume();


//	ClearAllScreenBuffers();
	m_bLoadFailed = LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreExitingLevelState()
//
//	PURPOSE:	Initialize the ExitingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreExitingLevelState(GameState eCurState)
{
    if (eCurState == GS_EXITINGLEVEL) return LTFALSE;

	// RETAIL: 0x100cbc12: Tron fades out in single player only, from any
	// state
	if (!IsMultiplayerGame())
	{
		StartScreenFadeOut( g_vtExitLevelScreenFadeTime.GetFloat() );
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostExitingLevelState()
//
//	PURPOSE:	Handle leaving the ExitingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostExitingLevelState(GameState eNewState)
{
    if (eNewState == GS_EXITINGLEVEL) return LTFALSE;

	// Tell the missionmgr we're leaving exit level.
//	g_pMissionMgr->FinishExitLevel( );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreSplashScreenState()
//
//	PURPOSE:	Initialize the SplashScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreSplashScreenState(GameState eCurState)
{
    if (eCurState == GS_SPLASHSCREEN) return LTFALSE;

	// Since we're going to always go to the menu state next, load the
	// surfaces here...

	m_InterfaceResMgr.Setup();


	// Play splash screen sound...

    uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
	m_hSplashSound = g_pClientSoundMgr->PlayInterfaceSound(IM_SPLASH_SOUND, PLAYSOUND_CLIENT | PLAYSOUND_GETHANDLE);	// RETAIL: 0x100cbc93 (0x210)

	if (m_hSplashSound)
	{
		g_pLTClient->SoundMgr()->GetSoundDuration(m_hSplashSound, g_fSplashSndDuration);
		g_pLTClient->CPrint("Splash sound duration: %.4f", g_fSplashSndDuration);
		g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());
	}

	// Create the splash screen...
    g_hSplash = g_pLTClient->CreateSurfaceFromBitmap(IM_SPLASH_SCREEN);
    if (!g_hSplash) return LTFALSE;

    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);

	// Fade into the splash screen...

	StartScreenFadeIn(g_vtSplashScreenFadeIn.GetFloat());
	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostSplashScreenState()
//
//	PURPOSE:	Handle leaving the SplashScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostSplashScreenState(GameState eNewState)
{
    if (eNewState == GS_SPLASHSCREEN) return LTFALSE;

	if (g_hSplash)
	{
        g_pLTClient->DeleteSurface(g_hSplash);
        g_hSplash = LTNULL;
	}

	// Stop splash screen sound (if playing)...

	if (m_hSplashSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
        m_hSplashSound = LTNULL;
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMovieState()
//
//	PURPOSE:	Initialize the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreMovieState(GameState eCurState)
{
    if (eCurState == GS_MOVIE) return LTFALSE;

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
	m_bUseInterfaceCamera = LTTRUE;

	m_nCurMovie = gMovieOrder[0];
	NextMovie();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMovieState()
//
//	PURPOSE:	Handle leaving the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostMovieState(GameState eNewState)
{
    if (eNewState == GS_MOVIE) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreDemoScreenState()
//
//	PURPOSE:	Initialize the DemoScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreDemoScreenState(GameState eCurState)
{
    if (eCurState == GS_DEMOSCREEN) return LTFALSE;

	g_nDemo = -1;
	NextDemoScreen();

	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostDemoScreenState()
//
//	PURPOSE:	Handle leaving the DemoScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostDemoScreenState(GameState eNewState)
{
    if (eNewState == GS_DEMOSCREEN) return LTFALSE;

	if (g_hDemo)
	{
        g_pLTClient->DeleteSurface(g_hDemo);
        g_hDemo = LTNULL;
	}

    return LTTRUE;
}







// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ProcessAdvancedOptions
//
//	PURPOSE:	Process the advanced options
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ProcessAdvancedOptions()
{
	// Check advanced options...

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DisableMusic");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MUSIC;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableSound");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUND;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableMovies");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MOVIES;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableFog");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_FOG;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableLines");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_LINES;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableJoystick");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_JOYSTICK;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableTripBuf");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TRIPLEBUFFER;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableTJuncs");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TJUNCTIONS;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableSoundFilters");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUNDFILTERS;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableHardwareSound");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_HARDWARESOUND;
	}

	// Record the original state of sound and music

    hVar = g_pLTClient->GetConsoleVar("SoundEnable");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_SOUND;
	}

    hVar = g_pLTClient->GetConsoleVar("MusicEnable");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_MUSIC;
	}

	// Implement any advanced options here (before renderer is started)
	if ( !(m_dwAdvancedOptions & AO_SOUNDFILTERS) )
	{
        g_pLTClient->RunConsoleString("SoundFilters 0");
	}
	else
	{
		// only set this on, if there isn't a current setting
		hVar = g_pLTClient->GetConsoleVar("SoundFilters");
		if ( !hVar )
	        g_pLTClient->RunConsoleString("SoundFilters 1");
	}

    hVar = g_pLTClient->GetConsoleVar("SoundEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_SOUND))
	{
        g_pLTClient->RunConsoleString("SoundEnable 1");
	}

    hVar = g_pLTClient->GetConsoleVar("MusicEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_MUSIC))
	{
        g_pLTClient->RunConsoleString("MusicEnable 1");
	}


	if (!(m_dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
        // RETAIL: 0x100cc2b5 runs "TripleBuffer 0" (as Build 51), not NOLF2's "BackBufferCount 1".
        g_pLTClient->RunConsoleString("TripleBuffer 0");
	}

	if (!(m_dwAdvancedOptions & AO_FOG))
	{
        g_pLTClient->RunConsoleString("FogEnable 0");
	}

	if (!(m_dwAdvancedOptions & AO_JOYSTICK))
	{
        g_pLTClient->RunConsoleString("JoystickDisable 1");
	}

	if (!(m_dwAdvancedOptions & AO_CURSOR))
	{
        g_pLTClient->RunConsoleString("HardwareCursor 0");
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ChangeState
//
//	PURPOSE:	Change the game state.  This allows us to do pre/post state
//				change handling
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ChangeState(GameState eNewState)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	DebugChangeState(eNewState);

	GameState eCurState = m_eGameState;

	// First make sure we change change to the new state from the the
	// state we are currently in...

	if (PreChangeState(eCurState, eNewState))
	{
		m_eGameState = eNewState;

		// Since the state changed, update the cursor
		UpdateCursorState();

//	    g_pLTClient->ClearInput();
		m_eLastGameState = eCurState;

        return LTTRUE;

	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DebugChangeState
//
//	PURPOSE:	Print out debugging info about changing states
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DebugChangeState(GameState eNewState)
{
#ifdef _DEBUG
//#define _DEBUG_INTERFACE_MGR
#ifdef _DEBUG_INTERFACE_MGR
    g_pLTClient->CPrint("CInterfaceMgr::ChangeState() :");
    g_pLTClient->CPrint("  Old State: %s", c_GameStateNames[m_eGameState]);
    g_pLTClient->CPrint("  New State: %s", c_GameStateNames[eNewState]);
#endif
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreChangeState
//
//	PURPOSE:	Handle pre setting of game state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreChangeState(GameState eCurState, GameState eNewState)
{
	// Do any clean up of the old (current) state...

	switch (eCurState)
	{
		case GS_PLAYING :
		{
            if (!PostPlayingState(eNewState)) return LTFALSE;
		}
		break;

		case GS_MENU :
		{
            if (!PostMenuState(eNewState)) return LTFALSE;
		}
		break;

		case GS_POPUP :
		{
            if (!PostPopupState(eNewState)) return LTFALSE;
		}
		break;

		case GS_EXITINGLEVEL:
		{
            if (!PostExitingLevelState(eNewState)) return LTFALSE;
		}
		break;

		case GS_LOADINGLEVEL:
		{
            if (!PostLoadingLevelState(eNewState)) return LTFALSE;
		}
		break;

		case GS_SCREEN :
		{
            if (!PostScreenState(eNewState)) return LTFALSE;
		}
		break;

		case GS_PAUSED :
		{
            if (!PostPauseState(eNewState)) return LTFALSE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
            if (!PostSplashScreenState(eNewState)) return LTFALSE;
		}
		break;

		case GS_MOVIE :
		{
            if (!PostMovieState(eNewState)) return LTFALSE;
		}
		break;

		case GS_DEMOSCREEN :
		{
            if (!PostDemoScreenState(eNewState)) return LTFALSE;
		}
		break;

		default : break;
	}

	// Do any initial setup for the new state...

	switch (eNewState)
	{
		case GS_PLAYING :
		{
			return PrePlayingState(eCurState);
		}
		break;

		case GS_MENU :
		{
			return PreMenuState(eCurState);
		}
		break;

		case GS_POPUP :
		{
			return PrePopupState(eCurState);
		}
		break;

		case GS_EXITINGLEVEL:
		{
			return PreExitingLevelState(eCurState);
		}
		break;

		case GS_LOADINGLEVEL:
		{
			return PreLoadingLevelState(eCurState);
		}
		break;

		case GS_SCREEN :
		{
			return PreScreenState(eCurState);
		}
		break;

		case GS_PAUSED :
		{
			return PrePauseState(eCurState);
		}
		break;

		case GS_SPLASHSCREEN :
		{
			return PreSplashScreenState(eCurState);
		}
		break;

		case GS_MOVIE :
		{
			return PreMovieState(eCurState);
		}
		break;

		case GS_DEMOSCREEN :
		{
			return PreDemoScreenState(eCurState);
		}
		break;

		default : break;
	}

    return LTTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

// PORT: Tron's (0x100cc550) also writes its +0x100fc records (see the
// header); not yet written.
void CInterfaceMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->Save(pMsg);

	// TRON: the intel items (RETAIL: 0x100cc550)
	pMsg->Writeuint8(m_IntelRecords.size());
	std::vector<TronIntelRecord>::iterator iter;
	for (iter = m_IntelRecords.begin(); iter != m_IntelRecords.end(); iter++)
		iter->Save(pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Load(ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->Load(pMsg);

	// TRON: the intel items (RETAIL: 0x100cc5d0)
	uint8 nNumRecords = pMsg->Readuint8();
	for (int i = 0; i < nNumRecords; i++)
	{
		TronIntelRecord record;
		record.Load(pMsg);
		m_IntelRecords.push_back(record);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentScreen()
//
//	PURPOSE:	Finds out what the current screen is
//				- returns SCREEN_ID_NONE if not in a screen state
//
// --------------------------------------------------------------------------- //

eScreenID CInterfaceMgr::GetCurrentScreen()
{
	if (m_eGameState != GS_SCREEN)
	{
		return SCREEN_ID_NONE;
	}
	return GetScreenMgr( )->GetCurrentScreenID();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToScreen
//
//	PURPOSE:	Go to the specified screen
//
// --------------------------------------------------------------------------- //

// RETAIL: 0x100cc6a0: Tron's takes bAddToHistory and passes it on to the
// screen manager
LTBOOL CInterfaceMgr::SwitchToScreen(eScreenID screenID, bool bAddToHistory)
{
	if (m_eGameState != GS_SCREEN)
	{
		if (((m_eGameState == GS_SPLASHSCREEN) || (m_eGameState == GS_DEMOSCREEN)) && (screenID == SCREEN_ID_MAIN))
		{
			StartScreenFadeIn(g_vtMainScreenFadeIn.GetFloat());
		}
		else if (screenID == SCREEN_ID_SUBROUTINES)
		{
			s_bKeepGameMusic = LTTRUE;
		}

		LTBOOL bChanged = ChangeState(GS_SCREEN);
		s_bKeepGameMusic = LTFALSE;
        if (!bChanged) return LTFALSE;
	}

	GetScreenMgr( )->SetCurrentScreen(screenID, bAddToHistory);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToMenu
//
//	PURPOSE:	Go to the specified menu
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SwitchToMenu(eMenuID menuID)
{
	if (!m_MenuMgr.SetCurrentMenu(menuID)) return LTFALSE;

	if (m_eGameState != GS_MENU)
	{	
        if (!ChangeState(GS_MENU)) return LTFALSE;
		m_MenuMgr.SlideIn();
	}
	

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceScreenUpdate
//
//	PURPOSE:	Force the current screen to update
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ForceScreenUpdate(eScreenID screenID)
{
    if (m_eGameState != GS_SCREEN) return LTFALSE;

	return GetScreenMgr( )->ForceScreenUpdate(screenID);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::MissionFailed
//
//	PURPOSE:	Go to the mission failure state
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::MissionFailed(int nFailStringId)
{
	// The player is no longer in the world...
	g_pGameClientShell->SetWorldNotLoaded();

	m_nFailStringId = nFailStringId;
	SwitchToScreen(SCREEN_ID_FAILURE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ScreenDimsChanged
//
//	PURPOSE:	Handle the screen dims changing
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::ScreenDimsChanged()
{
	m_InterfaceResMgr.ScreenDimsChanged();
	GetScreenMgr( )->ScreenDimsChanged();
	m_MenuMgr.ScreenDimsChanged();
	GetHUDMgr()->ScreenDimsChanged();
	// RETAIL: 0x100cc819
	m_CursorMgr.ScheduleReinit(5.0f);

	// Update the camera rect...
    uint32 dwWidth = 640, dwHeight = 480;

    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	// This may need to be changed to support in-game cinematics...

	ResetMenuRestoreCamera(0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraRect (m_hInterfaceCamera, LTTRUE, 0, 0, dwWidth, dwHeight);

	UpdateInterfaceBackground();

}



//mouse handling
void CInterfaceMgr::OnLButtonUp(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		// RETAIL: 0x100cc8e0: Tron clears the input first
		g_pLTClient->ClearInput();
		m_MessageBox.OnLButtonUp(x,y);
		return;
	}

	switch (m_eGameState)
	{
	case GS_LOADINGLEVEL:
		{
		} break;
	case GS_SCREEN:
		{
			// RETAIL: 0x100cc90a: not while the credits run
			if (!m_Credits.IsInited())
				GetScreenMgr( )->OnLButtonUp(x,y);
		} break;
	case GS_MENU:
		{
			m_MenuMgr.OnLButtonUp(x,y);
		} break;
	case GS_POPUP:
		{
			// RETAIL: 0x100cc930: no popup lock in Tron
			if(m_PopupText.IsVisible())
			{
				m_PopupText.Close();
			}
		} break;
	}
}

void CInterfaceMgr::OnLButtonDown(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonDown(x,y);
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			// RETAIL: 0x100cc9af: not while the credits run
			if (!m_Credits.IsInited())
				GetScreenMgr( )->OnLButtonDown(x,y);
		}	
		break;

		case GS_MENU:
		{
			m_MenuMgr.OnLButtonDown(x,y);
		} 
		break;

		case GS_SPLASHSCREEN:
		{
			// They pressed a mouse button - end splash screen...
			EndSplashScreen( );
		}	
		break;

		case GS_MOVIE:
		{
			// They pressed a mouse button - next movie
			NextMovie();
		}	
		break;

		case GS_DEMOSCREEN:
		{
			// They pressed a mouse button - next demo screen
			NextDemoScreen();
		}	
		break;

	}
}


void CInterfaceMgr::OnLButtonDblClick(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ) )
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonDblClick(x,y);
		} 
		break;

		case GS_MENU:
		{
			m_MenuMgr.OnLButtonDblClick(x,y);
		} 
		break;
	}

}

void CInterfaceMgr::OnRButtonUp(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonUp(x,y);
		} 
		break;
	
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonUp(x,y);
		}
		// RETAIL: 0x100ccac5: Tron falls through to the popup case
		// (no break, no popup lock)

		case GS_POPUP:
		{
			if(m_PopupText.IsVisible())
			{
				m_PopupText.Close();
			}
		} break;
	}


}

void CInterfaceMgr::OnRButtonDown(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDown(x,y);
		}	
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonDown(x,y);
		} 
		break;
		
		case GS_SPLASHSCREEN:
		{
			// They pressed a button - end splash screen...
			EndSplashScreen( );
		}	
		break;
	
		case GS_MOVIE:
		{
			// They pressed a button - next movie...
			NextMovie();
		}	
		break;
		
		case GS_DEMOSCREEN:
		{
			// They pressed a button - next demo screen...
			NextDemoScreen();
		}	
		break;
	
	}
}

void CInterfaceMgr::OnRButtonDblClick(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDblClick(x,y);
		} 
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonDblClick(x,y);
		} 
		break;
	}

}

void CInterfaceMgr::OnMouseMove(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	m_CursorPos.x = x;
	m_CursorPos.y = y;

	if (GetConsoleInt("ShowMousePos",0) > 0)
	{
		float fx = (float)x / g_pInterfaceResMgr->GetXRatio();
		float fy = (float)y / g_pInterfaceResMgr->GetYRatio();
		g_pLTClient->CPrint("MousePos (%0.0f,%0.0f)",fx,fy);
	}

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnMouseMove(x,y);
		return;
	}

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			// RETAIL: 0x100ccc75: not while the credits run
			if (!m_Credits.IsInited())
				GetScreenMgr( )->OnMouseMove(x,y);
		}
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnMouseMove(x,y);
		} 
		break;
	}
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateOverlays()
//
//	PURPOSE:	Update the overlay used as a scope crosshair
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateOverlays()
{
	if (!m_nOverlayCount) return;


    LTVector vPos(0, 0, 0), vU, vR, vF, vTemp;
    LTRotation rRot;

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

    LTBOOL bDrawnOne = LTFALSE;
	for (int i = 0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
		{
			if ((!bDrawnOne || i >= OVM_NON_EXCLUSIVE) && GS_PLAYING == GetGameState())
			{
				if (i < OVM_NON_EXCLUSIVE)
				{
                    bDrawnOne = LTTRUE;
				}

				if (g_pLayoutMgr->IsMaskSprite((eOverlayMask)i))
				{
					VEC_COPY(vTemp,g_vOverlaySpriteScale);
				}
				else
				{
					VEC_COPY(vTemp,g_vOverlayModelScale);
				}
				VEC_MULSCALAR(vTemp, vTemp, m_fOverlayScaleMult[i]);

				g_pLTClient->SetObjectScale(m_hOverlays[i], &vTemp);

				// RETAIL: 0x100ccd90 -- Tron sets the overlay's distance
				// every update.
				LTVector vOverlayPos(0.0f, 0.0f, g_vtOverlayModelDist.GetFloat());
				g_pLTClient->SetObjectPos(m_hOverlays[i], &vOverlayPos, LTTRUE);

				g_pCommonLT->SetObjectFlags(m_hOverlays[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

			}
			else
			{
				g_pCommonLT->SetObjectFlags(m_hOverlays[i], OFT_Flags, 0, FLAG_VISIBLE);
			}
		}
	}
}

void CInterfaceMgr::CreateOverlay(eOverlayMask eMask)
{
	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

	// Already created this mask
	if (m_hOverlays[eMask]) return;


	m_fOverlayScaleMult[eMask] = g_pLayoutMgr->GetMaskScale(eMask);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;

	// RETAIL: 0x100ccfda: Tron's overlays sit at a fixed spot in front of
	// the camera (a sprite at the sprite distance, a model at its layout
	// position); a model has a render style and an animation too
	if (g_pLayoutMgr->IsMaskSprite(eMask))
	{
		LTVector vPos(0.0f, 0.0f, g_fOverlaySpriteDist);
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITE_NOZ | FLAG_FOGDISABLE | FLAG_NOLIGHT | FLAG_REALLYCLOSE;
		createStruct.m_Pos = vPos;
		createStruct.m_Rotation.Init();

		char sprName[128] = "";
		g_pLayoutMgr->GetMaskSprite(eMask,sprName,sizeof(sprName));

		if (sprName[0])
		{
			SAFE_STRCPY(createStruct.m_Filename, sprName);
			m_hOverlays[eMask] = g_pLTClient->CreateObject(&createStruct);
			m_nOverlayCount++;

			LTFLOAT r, g, b, a;
			g_pLTClient->GetObjectColor(m_hOverlays[eMask], &r, &g, &b, &a);

			a = g_pLayoutMgr->GetMaskAlpha(eMask);

			g_pLTClient->SetObjectColor(m_hOverlays[eMask], r, g, b, a);
		}
	}
	else
	{
		LTVector vPos = g_pLayoutMgr->GetMaskPosition(eMask);
		createStruct.m_Pos = vPos;

		char modName[128] = "";
		char skinName[128] = "";
		char animName[128] = "";
		char rsName[128] = "";
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_REALLYCLOSE;
		createStruct.m_Rotation.Init();
		g_pLayoutMgr->GetMaskModel(eMask,modName,sizeof(modName));
		g_pLayoutMgr->GetMaskSkin(eMask,skinName,sizeof(skinName));
		g_pLayoutMgr->GetMaskRenderStyle(eMask,rsName,sizeof(rsName));
		g_pLayoutMgr->GetMaskAnim(eMask,animName,sizeof(animName));
		LTFLOAT fAlpha = g_pLayoutMgr->GetMaskAlpha(eMask);

		createStruct.m_ObjectType = OT_MODEL;

		if (modName[0] && skinName[0])
		{
			SAFE_STRCPY(createStruct.m_Filename, modName);
			SAFE_STRCPY(createStruct.m_SkinName, skinName);
			if (rsName[0])
			{
				SAFE_STRCPY(createStruct.m_RenderStyleName, rsName);
			}
			m_hOverlays[eMask] = g_pLTClient->CreateObject(&createStruct);

			LTFLOAT r, g, b, a;
			g_pLTClient->GetObjectColor(m_hOverlays[eMask], &r, &g, &b, &a);
			g_pLTClient->SetObjectColor(m_hOverlays[eMask], r, g, b, fAlpha);

			if (animName[0])
			{
				g_pLTClient->SetModelAnimation(m_hOverlays[eMask], g_pLTClient->GetAnimIndex(m_hOverlays[eMask], animName));
			}

			m_nOverlayCount++;
		}
	}


	// Update to make sure everything is scaled correctly before the next
	// frame is rendered...

	UpdateOverlays();
}

void CInterfaceMgr::RemoveOverlay(eOverlayMask eMask)
{
	if (m_hOverlays[eMask])
	{
		m_nOverlayCount--;
        g_pLTClient->RemoveObject(m_hOverlays[eMask]);
        m_hOverlays[eMask] = LTNULL;
	}
}



void CInterfaceMgr::BeginScope(LTBOOL bVisionMode, LTBOOL bCamera)
{
	if (bVisionMode)
	{
		CreateOverlay(OVM_STATIC);
	}

	CreateOverlay(OVM_SCOPE);

	if (bCamera)
	{
		CreateOverlay(OVM_CAMERA);
	}
}

void CInterfaceMgr::EndScope()
{
	RemoveOverlay(OVM_SCOPE);
	RemoveOverlay(OVM_STATIC);
	RemoveOverlay(OVM_CAMERA);
	RemoveOverlay(OVM_CAMERA_TARGET);

}

void CInterfaceMgr::BeginZoom(LTBOOL bIn)
{
	CreateOverlay(bIn ? OVM_ZOOM_IN : OVM_ZOOM_OUT);
}

void CInterfaceMgr::EndZoom()
{
	// RETAIL: 0x100cd4d0: Tron's zoom overlays too
	RemoveOverlay(OVM_ZOOM_IN);
	RemoveOverlay(OVM_ZOOM_OUT);
	RemoveOverlay(OVM_TRON_9);
	RemoveOverlay(OVM_TRON_10);
	RemoveOverlay(OVM_TRON_12);
	RemoveOverlay(OVM_TRON_13);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateScreenFade()
//
//	PURPOSE:	Update the screen fade
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenFade(bool bUpdateAlpha /*=true*/)
{
	if (!m_bScreenFade || m_fTotalFadeTime < 0.1f) return;

    LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);

	// RETAIL: 0x100cd586: Tron fades to white when the fade's second
	// argument was true
    HLTCOLOR hTransColor = m_bTronUnknownFadeF4A0 ? SETRGB(0,0,0) : SETRGB(255,255,255);
    HLTCOLOR hFadeColor = m_bTronUnknownFadeF4A0 ? SETRGB(255,255,255) : SETRGB(0,0,0);
	
	// Initialize the surface if necessary...

	if (!m_bFadeInitialized)
	{
		if (m_hFadeSurface)
		{
            g_pLTClient->DeleteSurface(m_hFadeSurface);
		}

        m_hFadeSurface = g_pLTClient->CreateSurface(2, 2);
		if (!m_hFadeSurface) return;

        g_pLTClient->SetSurfaceAlpha(m_hFadeSurface, 1.0f);
        g_pLTClient->FillRect(m_hFadeSurface, &rcSrc, hFadeColor);
        g_pLTClient->OptimizeSurface(m_hFadeSurface, hTransColor);

		m_fCurFadeTime = m_fTotalFadeTime;
        m_bFadeInitialized = LTTRUE;
	}


	// Fade the screen if the surface is around...

	if (m_hFadeSurface)
	{
		// RETAIL: 0x100cd665
		g_pLTClient->StartOptimized2D();

		LTFLOAT fTimeDelta = m_bFadeIn ? m_fCurFadeTime : (m_fTotalFadeTime - m_fCurFadeTime);

		LTFLOAT fLinearAlpha = (fTimeDelta / m_fTotalFadeTime);
		fLinearAlpha = fLinearAlpha < 0.0f ? 0.0f : (fLinearAlpha > 1.0f ? 1.0f : fLinearAlpha);

		LTFLOAT fAlpha = 1.0f - WaveFn_SlowOn(1.0f - fLinearAlpha);
		fAlpha = fAlpha < 0.0f ? 0.0f : (fAlpha > 1.0f ? 1.0f : fAlpha);

		g_pLTClient->SetSurfaceAlpha(m_hFadeSurface, fAlpha);

		LTRect rcDest;
		rcDest.Init(0, 0, dwWidth, dwHeight);

		g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hFadeSurface, &rcDest, &rcSrc, hTransColor);

		// The alpha value is based on the fade time, only update it if
		// we want the alpha value to change...

		if (bUpdateAlpha)
		{
			m_fCurFadeTime -= g_pGameClientShell->GetFrameTime();
		}

		g_pLTClient->EndOptimized2D();
	}

	// See if we're done...

	if (ScreenFadeDone())
	{
		if (m_hFadeSurface)
		{
            g_pLTClient->DeleteSurface(m_hFadeSurface);
            m_hFadeSurface = LTNULL;
		}

		// If we faded in we're done...

		if (m_bFadeIn)
		{
            m_bFadeInitialized  = LTFALSE;
            m_bScreenFade       = LTFALSE;
		}
		else
		{
			// We need to keep the screen black until it is faded
			// back in, so we'll just make sure the screen is clear
			// (and not set m_bScreenFade so we'll be called to
			// clear the screen every frame until we fade back in)...

	        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateLetterBox()
//
//	PURPOSE:	Update the letterbox
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateLetterBox()
{
	LTBOOL bOn = (g_vtLetterBoxDisabled.GetFloat() ? LTFALSE : (m_bLetterBox || g_vtLetterBox.GetFloat()));

	LTFLOAT fTime = g_pLTClient->GetTime();

	if (bOn && !m_bWasLetterBox)
	{
		// Handle being turned on...

		m_fLetterBoxAlpha = 0.0f;
		m_fLetterBoxFadeEndTime = fTime + g_vtLetterBoxFadeInTime.GetFloat();
	}
	else if (!bOn && m_bWasLetterBox)
	{
		// Handle being turned off...

		m_fLetterBoxAlpha = 1.0f;
		m_fLetterBoxFadeEndTime = fTime + g_vtLetterBoxFadeOutTime.GetFloat();
	}

	// Store the current state of being on or off...

	m_bWasLetterBox = bOn;


	LTFLOAT fTimeLeft = m_fLetterBoxFadeEndTime - fTime;
	if (fTimeLeft < 0.0f) fTimeLeft = 0.0f;

	// Calculate the current alpha...

	if (bOn)
	{
		// Fading in...

		LTFLOAT fFadeTime = g_vtLetterBoxFadeInTime.GetFloat();

		if (fFadeTime <= 0.0f || fTimeLeft <= 0.0f)
		{
			m_fLetterBoxAlpha = 1.0f;
		}
		else
		{
			m_fLetterBoxAlpha = 1.0f - (fTimeLeft / fFadeTime);
		}
	}
	else
	{
		// Fading out...

		LTFLOAT fFadeTime = g_vtLetterBoxFadeOutTime.GetFloat();

		if (fFadeTime <= 0.0f || fTimeLeft <= 0.0f)
		{
			m_fLetterBoxAlpha = 0.0f;
		}
		else
		{
			m_fLetterBoxAlpha = fTimeLeft / fFadeTime;
		}
	}


	// If not using letter box, and we faded it out, we're done...

	if (!bOn && m_fLetterBoxAlpha <= 0.0f) return;


	// Determine the border size...

	float fBorderPercent = 0.0f;
	float fLetterBoxPercent = g_pClientButeMgr->GetCameraAttributeFloat(CAMERA_BUTE_LETTERBOXPERCENT);

	if (fLetterBoxPercent > 0.01f && fLetterBoxPercent <= 1.0f)
	{
		fBorderPercent = (1.0f - fLetterBoxPercent) / 2.0f;
	}
	// RETAIL: 0x100cda78: Tron draws the borders with draw primitives,
	// sliding them in with the alpha
	uint32 dwWidth = m_InterfaceResMgr.GetScreenWidth();
	uint32 dwHeight = m_InterfaceResMgr.GetScreenHeight();

	uint8 nAlpha = (uint8)(255.0f * m_fLetterBoxAlpha);
	uint32 nColor = SET_ARGB(nAlpha, 0, 0, 0);
	g_pDrawPrim->SetRGBA(&m_LetterBox[0], nColor);
	g_pDrawPrim->SetRGBA(&m_LetterBox[1], nColor);

	int nBorderSize = (int) (float(dwHeight) * fBorderPercent * m_fLetterBoxAlpha);

	g_pDrawPrim->SetXYWH(&m_LetterBox[0], -1.0f, -1.0f, (float)(dwWidth + 2), (float)(nBorderSize + 1));
	g_pDrawPrim->SetXYWH(&m_LetterBox[1], -1.0f, (float)(dwHeight - nBorderSize), (float)(dwWidth + 2), (float)(nBorderSize + 1));

	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	// draw the two polys
	g_pDrawPrim->DrawPrim(m_LetterBox, 2);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AllowCameraMovement
//
//	PURPOSE:	Can the camera move (while in this state)
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::AllowCameraMovement()
{
	switch (m_eGameState)
	{
		case GS_MENU :
		case GS_POPUP :
            return LTFALSE;
		break;

		case GS_PLAYING:
			// RETAIL: 0x100cdbf0: the light cycle moves while chatting
			if (g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
				return LTTRUE;
			return  !g_pChatInput->IsVisible();
		break;

		default :
		break;
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CreateInterfaceBackground
//
//	PURPOSE:	Create the sprite used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::CreateInterfaceBackground()
{
	char sprName[128] = "";

	BSCREATESTRUCT bcs;

    LTVector vTemp, vScale;
	LTVector vPos, vU, vR, vF;
    LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hInterfaceCamera, &vPos);
	g_pLTClient->GetObjectRotation(m_hInterfaceCamera, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	if( !g_pLayoutMgr )
	{
		UBER_ASSERT( 0, "CInterfaceMgr::CreateInterfaceBackground: No LayoutMgr" );
		return;
	}

	vScale = g_vBaseBackScale * g_pLayoutMgr->GetBackSpriteScale();


	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
//	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
    bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);

	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;


	g_pLayoutMgr->GetBackSprite(sprName,sizeof(sprName));
	if ( sprName[0] )
	{
		bcs.pFilename = sprName;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
		bcs.nType = OT_SPRITE;
		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.fLifeTime = 1000000.0f;

		if (m_BackSprite.Init(&bcs))
		{
			m_BackSprite.CreateObject(g_pLTClient);
		}
	}

//TEST: Mouse Light
/*
	if (!g_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE | FLAG_ONLYLIGHTOBJECTS;

        g_pLTClient->GetObjectPos(m_hInterfaceCamera, &createStruct.m_Pos);
		//vPos -= (vF * (g_fBackDist / 2.0f));
		//createStruct.m_Pos = vPos;

		createStruct.m_Pos.x += 10.0f;
		createStruct.m_Pos.y += 25.0f;
		createStruct.m_Pos.z -= 300.0f;

        g_hLight = g_pLTClient->CreateObject(&createStruct);

        g_pLTClient->SetLightColor(g_hLight, 1.0f, 1.0f, 1.0f);
        g_pLTClient->SetLightRadius(g_hLight, 1500.0f);
	}
*/
// END TEST!!!

}

void CInterfaceMgr::UpdateInterfaceBackground()
{

/*
	if (GetConsoleInt("MouseLight",0))
	{
		LTVector vPos;
		LTRotation rRot;

		g_pLTClient->GetObjectPos(m_hInterfaceCamera, &vPos);
		g_pLTClient->GetObjectRotation(m_hInterfaceCamera, &rRot);

		vPos.z -= 300.0f;

		float x = ((float)m_CursorPos.x - ((float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f));
		float y = (((float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f) - (float)m_CursorPos.y);

		g_pLTClient->CPrint("light pos: (%0.2f,%0.2f)",x,y);
		vPos.x+=x;
		vPos.y+=y;

		g_pLTClient->SetObjectPos(g_hLight, &vPos);
	//	g_pLTClient->SetObjectPos(m_BackSprite.GetObject(), &vPos);
	}
*/
}

void CInterfaceMgr::RemoveInterfaceBackground()
{
	m_BackSprite.Term();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AddInterfaceSFX
//
//	PURPOSE:	Add a SFX object to the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType)
{
	if (!pSFX) return;

	SFXArray::iterator iter = m_InterfaceSFX.begin();
	uint32 index = 0;
	while (iter != m_InterfaceSFX.end() && (*iter) != pSFX)
	{
		iter++;
		index++;
	}
	if (index >= m_InterfaceSFX.size())
	{
		if (m_InterfaceSFX.size() < MAX_INTERFACE_SFX)
		{
			m_InterfaceSFX.push_back(pSFX);
		}
	}

	HOBJECT hObj = pSFX->GetObject();
	if (!hObj)
		return;

	if (GetObjectType(hObj)== OT_MODEL)
	{
		char* pAniName = LTNULL;
		switch (eType)
		{
			case IFX_NORMAL :
				pAniName = "Interface";
			break;

			case IFX_ATTACH :
				pAniName = "Hand";
			break;

			case IFX_WORLD :
				pAniName = "World";
			break;

			case IFX_MENU_ATTACH:	// abm 3/7/02 new fx type for when an ani is prespecified
				pAniName = LTNULL;
			break;

			default :
				pAniName = "World";
			break;


		}

		if (pAniName)
		{
			uint32 dwAni = g_pLTClient->GetAnimIndex(hObj, pAniName);
			if (dwAni != INVALID_ANI)
			{
				g_pLTClient->SetModelAnimation(hObj, dwAni);
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::RemoveInterfaceSFX
//
//	PURPOSE:	Remove a SFX object from the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::RemoveInterfaceSFX(CSpecialFX* pSFX)
{
	SFXArray::iterator iter = m_InterfaceSFX.begin();
	uint32 index = 0;
	while (iter != m_InterfaceSFX.end() && (*iter) != pSFX)
	{
		iter++;
	}
	if (iter != m_InterfaceSFX.end())
	{
		m_InterfaceSFX.erase(iter);	
	}
}

void CInterfaceMgr::AddInterfaceFX(CLIENTFX_LINK* pLink, char *pFXName, LTVector vPos, LTBOOL bLoop)
{
	uint32 dwFlags = 0;
	if (bLoop)
		dwFlags |= FXFLAG_LOOP;

	CLIENTFX_CREATESTRUCT	fxInit(pFXName , dwFlags, vPos ); 
	CLIENTFX_LINK fxLink;
	if (!pLink)
		pLink = &fxLink;

	m_InterfaceFXMgr.CreateClientFX(pLink, fxInit, LTTRUE );

	if (pLink->IsValid())
	{
		pLink->GetInstance()->SetPos(vPos,vPos);
		pLink->GetInstance()->Show();
	}
	else
	{
		g_pLTClient->CPrint("ERROR: Failed to create interface FX %s",pFXName);
	}

}
void CInterfaceMgr::RemoveInterfaceFX()
{
	m_InterfaceFXMgr.ShutdownAllFX();
	m_SelectFX.ClearLink();
	m_MouseFX.ClearLink();
}

void CInterfaceMgr::RemoveInterfaceFX(CLIENTFX_LINK	*pLink)
{
	m_InterfaceFXMgr.ShutdownClientFX(pLink);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveAllInterfaceSFX
//
//	PURPOSE:	Remove the 3D objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::RemoveAllInterfaceSFX()
{
	m_InterfaceSFX.clear();
	RemoveInterfaceBackground();
	RemoveInterfaceLights();

	m_InterfaceFXMgr.ShutdownAllFX();
	m_SelectFX.ClearLink();
	m_MouseFX.ClearLink();
}

void CInterfaceMgr::AddInterfaceLight(HOBJECT hLight)
{
	if (!hLight) return;

	ObjectArray::iterator iter = m_InterfaceLights.begin();
	while (iter != m_InterfaceLights.end() && (*iter) != hLight)
	{
		iter++;
	}
	if (iter == m_InterfaceLights.end())
	{
		if (m_InterfaceLights.size() < MAX_INTERFACE_LIGHTS)
		{
			m_InterfaceLights.push_back(hLight);
		}
	}

}

void CInterfaceMgr::RemoveInterfaceLights()
{
	ObjectArray::iterator iter = m_InterfaceLights.begin();
	while (iter != m_InterfaceLights.end())
	{
		if( *iter )
		{
		g_pLTClient->RemoveObject((*iter));
		}

		iter++;
	}
	m_InterfaceLights.clear();
}


void CInterfaceMgr::UpdateModelAnimations(LTFLOAT fCurFrameDelta)
{
    for (uint32 i = 0; i < m_InterfaceSFX.size(); i++)
	{
		uint32 nObjType;
		g_pCommonLT->GetObjectType(m_InterfaceSFX[i]->GetObject(), &nObjType);
		if (nObjType == OT_MODEL)
		{
			g_pModelLT->UpdateMainTracker(m_InterfaceSFX[i]->GetObject(), fCurFrameDelta);
		}
	}

	CLinkList<CLIENTFX_INSTANCE *>* pInstList = m_InterfaceFXMgr.GetActiveFXList();
	CLIENTFX_INSTANCE	*pInst;

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = pInstList->GetHead();
	while( pInstNode )
	{
		pInst	= pInstNode->m_Data;

		CLinkListNode<FX_LINK>				*pActiveNode;
		CBaseFX								*pFX;

		pActiveNode = pInst->m_collActiveFX.GetHead();
		while( pActiveNode )
		{
			pFX = pActiveNode->m_Data.m_pFX;
			if( pFX) 
			{
				uint32 nObjType;
				g_pCommonLT->GetObjectType(pFX->GetFXObject(), &nObjType);
				if (nObjType == OT_MODEL)
				{
					g_pModelLT->UpdateMainTracker(pFX->GetFXObject(), fCurFrameDelta);
				}
			}
			pActiveNode = pActiveNode->m_pNext;
		}
		pInstNode = pInstNode->m_pNext;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateInterfaceSFX
//
//	PURPOSE:	Update the 3D Objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateInterfaceSFX()
{

//TEST: Mouse Light
//	UpdateInterfaceBackground();

	static LTIntPt s_oldPos = m_CursorPos;
	int dist = abs(s_oldPos.x - m_CursorPos.x) + abs(s_oldPos.y - m_CursorPos.y);
	// RETAIL: 0x100ce6e5 -- Tron's flag at +0xf988 keeps the trail, and
	// MouseTrailFix removes a hidden one.
	if (dist > 1 || m_bTronUnknownF988)
		ShowMouseFX();
	else
	{
		if (m_MouseFX.IsValid())
		{
			m_MouseFX.GetInstance()->Hide();
			if (g_vtMouseTrailFix.GetFloat())
			{
				m_InterfaceFXMgr.ShutdownClientFX(&m_MouseFX);
			}
		}
	}
	s_oldPos = m_CursorPos;

    uint32 numSfx = m_InterfaceSFX.size();
	HLOCALOBJ objs[kMaxFX];

	int next = 0;
	int infoIndex = 0;
    for (uint32 i = 0; i < numSfx; i++)
	{
		if (m_InterfaceSFX[i]->Update())
		{
			FXRenderList::iterator iter = renderList.begin();

			while (iter != renderList.end() && (*iter)->nLayer < m_InterfaceSFX[i]->GetMenuLayer())
			{
				iter++;
			}

			if(infoIndex < kMaxFX)
			{
				sRenderInfo[infoIndex].hObj = m_InterfaceSFX[i]->GetObject();
				sRenderInfo[infoIndex].nLayer = m_InterfaceSFX[i]->GetMenuLayer();
				renderList.insert(iter,&sRenderInfo[infoIndex]);
				infoIndex++;
			}
		}
	}
	CLinkList<CLIENTFX_INSTANCE *>* pInstList = m_InterfaceFXMgr.GetActiveFXList();
	CLIENTFX_INSTANCE	*pInst;

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = pInstList->GetHead();

	while( pInstNode )
	{
		pInst	= pInstNode->m_Data;

		CLinkListNode<FX_LINK>				*pActiveNode;
		CBaseFX								*pFX;

		pActiveNode = pInst->m_collActiveFX.GetHead();
		while( pActiveNode )
		{
			pFX = pActiveNode->m_Data.m_pFX;
			if( pFX) 
			{
				FXRenderList::iterator iter = renderList.begin();

				while (iter != renderList.end() && (*iter)->nLayer < pFX->GetMenuLayer())
				{
					iter++;
				}

				if(infoIndex < kMaxFX)
				{
					sRenderInfo[infoIndex].hObj = pFX->GetFXObject();
					sRenderInfo[infoIndex].nLayer = pFX->GetMenuLayer();
					renderList.insert(iter,&sRenderInfo[infoIndex]);
					infoIndex++;
				}
			}
			pActiveNode = pActiveNode->m_pNext;
		}
		pInstNode = pInstNode->m_pNext;
	}


	// Render the effects
	m_InterfaceFXMgr.RenderAllActiveFX( true );

	//step through the render list
	FXRenderList::iterator iter = renderList.begin();
	static char szTmp[256];
	static char szNum[16];
	while (iter != renderList.end())
	{
		next = 0;

		//get the menu layer of the first item
		uint8 layer = (*iter)->nLayer;

		//add each item of that same layer to the list of objects to be rendered during this pass
		while (iter != renderList.end() && next < kMaxFX && (*iter)->nLayer == layer)
		{
			objs[next] = (*iter)->hObj;
			next++;
			iter++;
		}

		//add the lights
		for ( ObjectArray::iterator iter = m_InterfaceLights.begin( ); iter !=  m_InterfaceLights.end() && 
			next < kMaxFX; iter++ )
		{
			HOBJECT hObj = *iter;
			if ( hObj )
			{
				objs[next] = hObj;
				next++;
			}
		}


		g_pLTClient->RenderObjects(m_hInterfaceCamera, objs, next,g_pLTClient->GetFrameTime());
	
	}


	//the updating of the active effects comes at the end of the update so that way chained
	//effects will be able to setup prior to the next rendering and prevent popping issues
	m_InterfaceFXMgr.UpdateAllActiveFX( true );

	renderList.clear();	
}


// RETAIL: Tron's (0x100cea40; the name is ours): one object drawn by
// the interface camera with the interface lights. The array has room for
// the lights, but the test is the fx list's (see docs/RETAIL_BUGS.md)
void CInterfaceMgr::RenderInterfaceObject(HOBJECT hObj)
{
	if (!g_pInterfaceMgr->GetInterfaceCamera() || !hObj)
		return;

	HLOCALOBJ objs[MAX_INTERFACE_LIGHTS + 1];
	objs[0] = hObj;
	int next = 1;

	for ( ObjectArray::iterator iter = m_InterfaceLights.begin( ); iter !=  m_InterfaceLights.end() && 
		next < kMaxFX; iter++ )
	{
		HOBJECT hLight = *iter;
		if ( hLight )
		{
			objs[next] = hLight;
			next++;
		}
	}

	g_pLTClient->RenderObjects(g_pInterfaceMgr->GetInterfaceCamera(), objs, next, g_pLTClient->GetFrameTime());
}


/******************************************************************/
void CInterfaceMgr::RequestInterfaceSound(InterfaceSound eIS)
{
	if (m_eNextSound <= eIS)
	{
//		ClearInterfaceSound();

		m_eNextSound = eIS;

//		m_hSound = UpdateInterfaceSound();
		UpdateInterfaceSound();
	}
}

void CInterfaceMgr::ClearInterfaceSound()
{
	m_eNextSound = IS_NONE;

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = LTNULL;
	}
}

HLTSOUND CInterfaceMgr::UpdateInterfaceSound()
{
	HLTSOUND hSnd = LTNULL;

	switch (m_eNextSound)
	{
		case IS_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_CHANGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundChange(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_PAGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundPageChange(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_UP:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowUp(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_DOWN:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowDown(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_LEFT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowLeft(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_RIGHT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowRight(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_NO_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundUnselectable(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		default :
		break;
	}
	m_eNextSound = IS_NONE;

	return hSnd;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextMovie
//
//	PURPOSE:	Go to the next movie, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextMovie(bool bEndMovies /*=false*/)
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr) return;

	if (!(GetAdvancedOptions() & AO_MOVIES) || g_vtDisableMovies.GetFloat())
	{
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

	if (m_hMovie)
	{
		pVideoMgr->StopVideo(m_hMovie);
		m_hMovie = LTNULL;
	}

	char* pMovie = (bEndMovies ? LTNULL : GetCurrentMovie());

	if (!pMovie || pVideoMgr->StartOnScreenVideo(pMovie, PLAYBACK_FULLSCREEN, m_hMovie) != LT_OK)
	{
		m_nCurMovie = 0;
		m_hMovie = LTNULL;
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

 	m_nCurMovie++;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentMovie
//
//	PURPOSE:	Get the name of the current movie to play...
//
// --------------------------------------------------------------------------- //

char* CInterfaceMgr::GetCurrentMovie()
{
	char* pMovie = LTNULL;

	switch (m_nCurMovie)
	{
		case eDisney :
			pMovie = "Movies\\DisnLogo.bik";
		break;

		case eMonolith :
			pMovie = "Movies\\lithLogo.bik";
		break;

		case eNVidia :
			pMovie = "Movies\\nvidia.bik";
		break;

		default : break;
	}

	return pMovie;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextDemoScreen
//
//	PURPOSE:	Go to the next marketing screen, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextDemoScreen()
{
    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);

	if (g_hDemo)
	{
		g_pLTClient->DeleteSurface(g_hDemo);
		g_hDemo = LTNULL;
	}
	g_nDemo++;
	if (g_nDemo < g_nNumDemoScreens)
	{
		g_hDemo = g_pLTClient->CreateSurfaceFromBitmap(g_szDemoScreens[g_nDemo]);
	}
	if (!g_hDemo)
	{
		m_bSeenDemoScreens = LTTRUE;
	}
}


void CInterfaceMgr::ShowDemoScreens(LTBOOL bQuit)
{
	m_bQuitAfterDemoScreens = bQuit;
	m_bSeenDemoScreens = LTFALSE;
	ChangeState(GS_DEMOSCREEN);
}



// Find the filename in the missing filename result..
const char *FindFilename(const char *pFileName)
{
	// Jump to the end..
	const char *pResult = &pFileName[strlen(pFileName)];
	// Look for a backslash
	while ((pResult != pFileName) && (pResult[-1] != '\\'))
		--pResult;
	// Return that..
	return pResult;
}

void CInterfaceMgr::Disconnected(uint32 nDisconnectFlag)
{
	// RETAIL: 0x100ced40: Tron's string ids (CRes.dll) and screens
	eScreenID destination = SCREEN_ID_MULTI;
	// Get the disconnect code.
	uint32 nDisconnectCode = g_pClientMultiplayerMgr->GetDisconnectCode();
	uint32 nMsgId = 0;
	switch (nDisconnectCode)
	{
		case eDisconnect_NotSameGUID:
			nMsgId = 0x3ef;
			break;
		case eDisconnect_WrongPassword:
			nMsgId = 0x3f2;
			// Tron has one join screen
			destination = SCREEN_ID_JOIN_SERVERLIST;
			break;
		// RETAIL: a third disconnect code of Tron's (not yet in our tree's
		// eDisconnectCodes), to the join screen too
		case 3:
			nMsgId = 0x3fc;
			destination = SCREEN_ID_JOIN_SERVERLIST;
			break;

		case 0 : // Avoid the warning..
		default :
		{
			if( nDisconnectFlag == LT_REJECTED )
			{
				nMsgId = 0x2fc;
			}
			else
			{
				nMsgId = 0x482;
			}

			g_pLTClient->CPrint("CInterfaceMgr::Disconnected(%d)", nDisconnectCode);
		}
		break;
	}

	// We joined using the command line, we need to visit the main screen before
	// any other screen, since we didn't go through the normal screen
	// progression.
	if( GetCommandLineJoin( ))
	{
		destination = SCREEN_ID_MAIN;
	}

	g_pClientMultiplayerMgr->ClearDisconnectCode();

	if (GetGameState() == GS_LOADINGLEVEL)
	{
		LoadFailed( destination, nMsgId );
	}
	else
	{
		if (!m_bIntentionalDisconnect)
		{
			MBCreate mb;
			m_MessageBox.Close(LTTRUE);
			ShowMessageBox(nMsgId,&mb);
		}

		ClearAllScreenBuffers();
		SwitchToScreen(destination);
	}

	// RETAIL: 0x100cee5e
	if (g_pLTClient->IsConnected())
	{
		g_pClientMultiplayerMgr->ForceDisconnect();
	}
}

void CInterfaceMgr::ConnectionFailed(uint32 nConnectionError)
{
	if (GetGameState() == GS_LOADINGLEVEL)
	{
		// RETAIL: 0x100cee90: back to the multiplayer screen, as below
		if (IsMultiplayerGame() && !GetCommandLineJoin( ))
		{
			LoadFailed(SCREEN_ID_MULTI);
		}
		else
		{
			LoadFailed(SCREEN_ID_MAIN);
		}
	}
	else
	{
		ClearAllScreenBuffers();
		if (IsMultiplayerGame() && !GetCommandLineJoin( ))
		{
			SwitchToScreen(SCREEN_ID_MULTI);
		}
		else
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
	}

	// RETAIL: Tron's string ids (CRes.dll): 1013 "Unable to join. The
	// server is full.", 1003 "ERROR - Unable to join the selected game
	// session." (NOLF2's IDS_SERVERFULL, IDS_NETERR_JOINSESSION)
	MBCreate mb;
	if (nConnectionError == LT_REJECTED)
	{
		ShowMessageBox(1013,&mb);
	}
	else
	{
		ShowMessageBox(1003,&mb);
	}

}

// hides or shows cursor based on current game state
void CInterfaceMgr::UpdateCursorState()
{
	switch (m_eGameState)
	{
		case GS_EXITINGLEVEL:
		case GS_PLAYING:
		{	
			// 2nd param specifiies whether or not to lock the cursor to the
			// center of the screen
			m_CursorMgr.UseCursor(m_MessageBox.IsVisible(),!m_MessageBox.IsVisible());
		}
		break;

		case GS_LOADINGLEVEL:
		case GS_SPLASHSCREEN:
		case GS_POPUP:
		case GS_PAUSED:
		case GS_DEMOSCREEN:
		case GS_MOVIE:
		case GS_UNDEFINED:
		{
			m_CursorMgr.UseCursor(FALSE);
		}
		break;
	
		case GS_MENU:
		case GS_SCREEN:
		default:
		{
			m_CursorMgr.UseCursor(TRUE);
		}
	}
}



void CInterfaceMgr::UpdateClientList()
{
	if (IsMultiplayerGame())
	{
		// Don't send update requests more than once per second
		if ((g_pLTClient->GetTime() - m_fLastUpdateRequestTime) > 1.0f)
		{
			g_pLTClient->CPrint("Asking for clientlist update");
			SendEmptyServerMsg(MID_MULTIPLAYER_UPDATE, MESSAGE_GUARANTEED);
			m_fLastUpdateRequestTime = g_pLTClient->GetTime();
		}
	}
}



LTVector CInterfaceMgr::GetWorldFromScreenPos(LTIntPt pos, LTFLOAT fDepth)
{
	LTVector vPos;

	LTFLOAT fx = (2.0f * ((LTFLOAT)pos.x / (LTFLOAT)g_pInterfaceResMgr->GetScreenWidth() )) - 1.0f;
	LTFLOAT fy = 1.0f - (2.0f * ((LTFLOAT)pos.y / (LTFLOAT)g_pInterfaceResMgr->GetScreenHeight()) );

	vPos.x =  fx * g_fFovXTan * fDepth;
	vPos.y =  fy * g_fFovYTan * fDepth;
	vPos.z =  fDepth;

	return vPos;
}

//transforms world space to camera space to screen coordinates
//return vector's x and y are screen coordinates
//return vector's z is distance from the camera

LTVector CInterfaceMgr::GetScreenFromWorldPos(LTVector vPos, HOBJECT hCamera)
{
	LTVector tmp,pos;
	LTMatrix mCam, mInvCam;
	mCam = GetCameraTransform(hCamera);
	mCam.Normalize();

	mInvCam = mCam.MakeInverseTransform();
	mInvCam.Normalize();

	MatVMul((LTVector*)&tmp, &mCam, (LTVector*)&vPos);


	LTFLOAT fx = tmp.x / (g_fFovXTan * tmp.z);
	LTFLOAT fy = tmp.y / (g_fFovYTan * tmp.z);

	pos.x =  ((LTFLOAT)g_pInterfaceResMgr->GetScreenWidth() * (1.0f + fx) / 2.0f);
	pos.y =	 ((LTFLOAT)g_pInterfaceResMgr->GetScreenHeight() * (1.0f - fy) / 2.0f);
	pos.z =  tmp.z;

	return pos;
}


// RETAIL: 0x100cf870: a second argument (UpdateInterfaceSFX reads it,
// 0x100ce6e5), and the old effect is hidden before it is shut down
void CInterfaceMgr::SetMouseFX(char* pFXName, bool bTronUnknown)
{
	SAFE_STRCPY(m_szMouseFXName,pFXName);
	m_bTronUnknownF988 = bTronUnknown;
	if (m_MouseFX.IsValid())
	{
		m_MouseFX.GetInstance()->Hide();
		m_InterfaceFXMgr.ShutdownClientFX(&m_MouseFX);
	}
}

void CInterfaceMgr::SetSelectFX(char* pFXName)
{
	SAFE_STRCPY(m_szSelectFXName,pFXName);
}

void CInterfaceMgr::ShowSelectFX(const LTIntPt &pos)
{
	if (!strlen(m_szSelectFXName)) return;
	if (m_SelectFX.IsValid() && GetConsoleInt("ShutdownSelect",1))
	{
		m_InterfaceFXMgr.ShutdownClientFX(&m_SelectFX);
	}
	LTVector vPos = GetWorldFromScreenPos(pos,25.0f);
	CLIENTFX_CREATESTRUCT	fxInit(m_szSelectFXName , LTNULL, vPos ); 

	m_InterfaceFXMgr.CreateClientFX(&m_SelectFX, fxInit, LTTRUE );
	if( !m_SelectFX.IsValid() )
	{
		char szError[256];
		LTSNPrintF( szError, sizeof(szError), "CInterfaceMgr::ShowSelectFX:  Could not create client fx %s", m_szSelectFXName );
		UBER_ASSERT( 0, szError );
		return;
	}

	if(m_SelectFX.IsValid())
		m_SelectFX.GetInstance()->Show();

}
void CInterfaceMgr::ShowMouseFX()
{
	if (!strlen(m_szMouseFXName))
	{
		return;
	}
	LTVector vPos = GetWorldFromScreenPos(m_CursorPos,25.0f);

	if (!m_MouseFX.IsValid())
	{
		CLIENTFX_CREATESTRUCT	fxInit(m_szMouseFXName , FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, vPos ); // RETAIL: 0x100cfb2e 
		m_InterfaceFXMgr.CreateClientFX(&m_MouseFX, fxInit, LTTRUE );
	}

	if (m_MouseFX.IsValid())
	{
		m_MouseFX.GetInstance()->SetPos(vPos,vPos);
		m_MouseFX.GetInstance()->Show();
	}
}


void CInterfaceMgr::NextWeapon(int nCommand)
{
	// RETAIL: 0x100cfb90: Tron asks neither the weapons enabled nor a gadget
	// disabling to wait for

	uint8 nClass = 0;
	switch (nCommand)
	{
	case COMMAND_ID_NEXT_WEAPON	:
		nClass = 0;
		break;
	case COMMAND_ID_NEXT_WEAPON_1 :
		nClass = 1;
		break;
	case COMMAND_ID_NEXT_WEAPON_2 :
		nClass = 2;
		break;
	case COMMAND_ID_NEXT_WEAPON_3 :
		nClass = 3;
		break;
	case COMMAND_ID_NEXT_WEAPON_4 :
		nClass = 4;
		break;
	case COMMAND_ID_NEXT_WEAPON_5 :
		nClass = 5;
		break;
	case COMMAND_ID_NEXT_WEAPON_6 :
		nClass = 6;
		break;
	default:
		return;
		
	}

	if (m_AmmoChooser.IsOpen())
	{
		m_AmmoChooser.Close();
	}
	if (GetWeaponChooser()->Open(nClass))
	{
		GetWeaponChooser()->NextWeapon(nClass);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnObjectRemove()
//
//	PURPOSE:	Handle removal of a server created object...
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnObjectRemove(HLOCALOBJ hObj)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (!hObj) return;

	if (g_pDecision->IsVisible() && g_pDecision->GetObject() == hObj)
		g_pDecision->OnObjectRemove(hObj);

	// RETAIL: 0x100cfc60: no radar here in Tron
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::IsInGame()
//
//	PURPOSE:	Check if we're in one of the "in game" states.
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::IsInGame( )
{
	switch( GetGameState( ))
	{
		case GS_PLAYING:
		case GS_EXITINGLEVEL:
			return true;
		default:
			return false;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleDisplayTimerMsg
//
//	PURPOSE:	Handle update to display timer.
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::HandleDisplayTimerMsg( ILTMessage_Read& msg )
{
    float fGameTimeEnd = msg.Readfloat();
	bool bPaused = !!msg.Readuint8();

	// RETAIL: 0x100cfcc0: one timer, no team id in the message
	m_InterfaceTimer.SetTime( fGameTimeEnd, bPaused );
    return LTTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::LoadFailed
//
//	PURPOSE:	Handle failed load attempt, specify what screen to go to
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::LoadFailed(eScreenID eScreenToShow /*= SCREEN_ID_NONE*/, uint32 nLoadFailedMsgId /* = -1 */ )  
{	
	// RETAIL: 0x100cfd00: not the multiplayer screen after a command line join
	m_nLoadFailedMsgId = nLoadFailedMsgId;
	m_bLoadFailed = LTTRUE;

	if (eScreenToShow == SCREEN_ID_NONE)
	{
		if (IsMultiplayerGame() && !GetCommandLineJoin( ))
		{
			m_eLoadFailedScreen = SCREEN_ID_MULTI;
		}
		else
		{
			m_eLoadFailedScreen = SCREEN_ID_MAIN;
		}
	}
	else
		m_eLoadFailedScreen = eScreenToShow;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TronUnknownVirtual98
//
//	PURPOSE:	RETAIL: 0x100cfd60, slot 0x98 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::TronUnknownVirtual98(bool bTronUnknown, bool bInstant)
{
	if (!g_pPlayerMgr->GetTronUnknown1D7())
	{
		TronUnknownFirstPersonCamera(bInstant);
		return;
	}

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCC");
	if (bInstant && (!hVar || g_pLTClient->GetVarValueFloat(hVar) > 0.0f))
	{
		g_pPlayerMgr->GetPlayerCamera()->TronUnknownStartLCCOrbit();
	}
	else
	{
		TronUnknownThirdPersonCamera(bInstant);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TronUnknownThirdPersonCamera
//
//	PURPOSE:	RETAIL: 0x100cfdf0 (the name is ours): the light cycle FOV
//				and camera
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::TronUnknownThirdPersonCamera(bool bInstant)
{
	if (!g_pPlayerMgr)
		return;

	if (g_pPlayerMgr->GetTronUnknown1D5() && !g_pPlayerMgr->TronUnknownVirtual3C(true))
		return;

	float fFovY = g_vtFOVYCycle.GetFloat();
	float fFovX = g_vtFOVXCycle.GetFloat();
	g_pPlayerMgr->SetCameraFOV(DEG2RAD(fFovX), DEG2RAD(fFovY));
	g_pPlayerMgr->GetPlayerCamera()->TronUnknownGoLightCycleFromProfile(bInstant);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TronUnknownFirstPersonCamera
//
//	PURPOSE:	RETAIL: 0x100cfed0 (the name is ours)
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::TronUnknownFirstPersonCamera(bool bInstant)
{
	if (!g_pPlayerMgr)
		return;

	float fFovY = g_vtFOVYNormal.GetFloat();
	float fFovX = g_vtFOVXNormal.GetFloat();
	g_pPlayerMgr->SetCameraFOV(DEG2RAD(fFovX), DEG2RAD(fFovY));
	g_pPlayerMgr->GetPlayerCamera()->GoFirstPerson(bInstant);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetTronUnknownMessage
//
//	PURPOSE:	RETAIL: 0x100cff80 (the name is ours): a timed message,
//				its text or string (or none)
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::SetTronUnknownMessage(float fDuration, int nStringId, const char* pszText, bool bTronUnknown)
{
	// RETAIL: the time is ILTClient's slot 0xb0 (Tron's; not GetTime)
	m_fTronUnknownF98C = g_pLTClient->TronUnknownVirtual44();
	m_fTronUnknownF990 = fDuration;
	m_bTronUnknownFA54 = bTronUnknown;
	m_szTronUnknownFA14[0] = '\0';

	if (pszText && pszText[0])
	{
		strcpy(m_szTronUnknownF994, pszText);
	}
	else if (nStringId)
	{
		FormatString(nStringId, m_szTronUnknownF994, sizeof(m_szTronUnknownF994));
	}
	else
	{
		m_szTronUnknownF994[0] = '\0';
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawTronUnknownMessage
//
//	PURPOSE:	RETAIL: 0x100d0020 (the name is ours): in the light cycle
//				game types, the timed message counts down its seconds;
//				without a text a 3, 2, 1, 0 fades through its quarters
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DrawTronUnknownMessage()
{
	if (GetGameType() != eGameTypeMultiplayerLightcycles &&
		GetGameType() != eGameTypeSingleLightCycleCircuit &&
		GetGameType() != eGameTypeSingleLightCycleCustom)
	{
		return;
	}

	if (!(m_fTronUnknownF98C < 0.0f))
	{
	float fElapsed = g_pLTClient->TronUnknownVirtual44() - m_fTronUnknownF98C;
	if (fElapsed > m_fTronUnknownF990)
	{
		m_fTronUnknownF98C = -1.0f;
		return;
	}

	char szTex[64];

	if (m_szTronUnknownF994[0])
	{
		uint16 nSeconds = (uint16)(m_fTronUnknownF990 - fElapsed);

		char szMsg[128];
		if (m_bTronUnknownFA54)
		{
			sprintf(szMsg, "%s %s: %d", m_szTronUnknownF994, m_szTronUnknownFA14, nSeconds);
		}
		else
		{
			sprintf(szMsg, "%s %s %d", m_szTronUnknownF994, m_szTronUnknownFA14, nSeconds);
		}

		g_pTransmission->Show(szMsg);
	}
	else
	{
		float fQuarter = m_fTronUnknownF990 * 0.25f;
		float fStart;

		if (fElapsed < fQuarter)
		{
			fStart = m_fTronUnknownF98C;
			sprintf(szTex, "interface\\hud\\tronnumber_3.dtx");
		}
		else
		{
			float fHalf = fQuarter + fQuarter;
			if (fElapsed < fHalf)
			{
				fStart = fQuarter + m_fTronUnknownF98C;
				sprintf(szTex, "interface\\hud\\tronnumber_2.dtx");
			}
			else
			{
				float fThreeQuarters = fQuarter * 3.0f;
				if (fElapsed < fThreeQuarters)
				{
					fStart = fHalf + m_fTronUnknownF98C;
					sprintf(szTex, "interface\\hud\\tronnumber_1.dtx");
				}
				else
				{
					fStart = fThreeQuarters + m_fTronUnknownF98C;
					sprintf(szTex, "interface\\hud\\tronnumber_0.dtx");
				}
			}
		}

		float fAlpha = 1.0f - (g_pLTClient->TronUnknownVirtual44() - fStart) / fQuarter;
		HTEXTURE hTex = g_pInterfaceResMgr->GetTexture(szTex);

		LT_POLYFT4 poly;

		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

		g_pDrawPrim->SetXYWH(&poly, g_pInterfaceResMgr->GetXRatio() * 256.0f, g_pInterfaceResMgr->GetYRatio() * 20.0f,
			g_pInterfaceResMgr->GetXRatio() * 128.0f, g_pInterfaceResMgr->GetYRatio() * 128.0f);
		SetupQuadUVs(poly, hTex, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->SetRGB(&poly, 0xffffff);
		g_pDrawPrim->SetALPHA(&poly, (uint8)(fAlpha * 255.0f));
		g_pDrawPrim->SetTexture(hTex);
		g_pDrawPrim->DrawPrim(&poly, 1);
	}
}
	else
	{
		g_pTransmission->Term();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawSpectatorText
//
//	PURPOSE:	RETAIL: 0x100d03d0 (the name is ours): a light cycle
//				game's spectator sees "SPECTATOR MODE" at the screen's
//				bottom left
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DrawSpectatorText()
{
	if (g_pPlayerMgr->TronUnknownVirtual3C(false) && GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		uint8 nSize = (uint8)((float)g_pLayoutMgr->GetHelpSize() * g_pInterfaceResMgr->GetXRatio());
		CUIFont* pFont = g_pInterfaceResMgr->GetFontForHeight(nSize);

		if (!m_pSpectatorText)
		{
			m_pSpectatorText = g_pFontManager->CreatePolyString(pFont, std::wstring(L""), 0.0f, 0.0f);
		}

		if (m_pSpectatorText)
		{
			m_pSpectatorText->SetColor(0xffffffff);
			m_pSpectatorText->SetCharScreenHeight(nSize);
			m_pSpectatorText->SetText("SPECTATOR MODE");

			uint32 nWidth, nHeight;
			g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);
			float fHeight = (float)nHeight;

			m_pSpectatorText->SetPosition(0, (int)(fHeight - m_pSpectatorText->GetHeight()));
			m_pSpectatorText->Render(0, -1);
		}
	}
	else if (m_pSpectatorText)
	{
		m_pSpectatorText->SetText("");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::IsLoadScreenVisible
//
//	PURPOSE:	RETAIL: 0x100d05e0 (true without a loading screen)
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::IsLoadScreenVisible()
{
	if (m_pLoadingScreen)
		return m_pLoadingScreen->IsVisible() != LTFALSE;

	return true;
}

// RETAIL: 0x100d0620
void CInterfaceMgr::PauseLoadScreen()
{
	if (m_pLoadingScreen)
		m_pLoadingScreen->Pause();
}

// RETAIL: 0x100d0630
void CInterfaceMgr::ResumeLoadScreen()
{
	if (m_pLoadingScreen)
		m_pLoadingScreen->Resume();
}

// RETAIL: 0x100d0640
void CInterfaceMgr::UpdateLoadScreenInfo()
{
	if (m_pLoadingScreen)
		m_pLoadingScreen->UpdateMissionInfo();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::StartScreenFadeIn
//
//	PURPOSE:	RETAIL: 0x100d0650
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::StartScreenFadeIn(LTFLOAT fFadeTime, bool bTronUnknown)
{
	if (g_pMissionMgr->IsExitingLevel())
		return;

	m_bScreenFade = LTTRUE;
	m_bFadeIn = LTTRUE;
	m_bFadeInitialized = LTFALSE;
	m_fTotalFadeTime = fFadeTime;
	m_bTronUnknownFadeF4A0 = bTronUnknown;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::StartScreenFadeOut
//
//	PURPOSE:	RETAIL: 0x100d0690
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::StartScreenFadeOut(LTFLOAT fFadeTime, bool bTronUnknown)
{
	if (m_bScreenFade && (!m_bFadeIn || !g_pMissionMgr->IsExitingLevel()))
		return;

	m_bFadeInitialized = LTFALSE;
	m_bScreenFade = LTTRUE;
	m_fTotalFadeTime = fFadeTime;
	m_bFadeIn = LTFALSE;
	m_bTronUnknownFadeF4A0 = bTronUnknown;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceScreenFadeIn
//
//	PURPOSE:	RETAIL: 0x100d06e0
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ForceScreenFadeIn(LTFLOAT fFadeTime)
{
	if (m_bScreenFade && !m_bFadeIn)
	{
		StartScreenFadeIn(fFadeTime, false);
	}
}

void CInterfaceMgr::HandlePlayerTeamChange()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_CHARACTER_ID);
	if (pList)
	{
		int nNumChars = pList->GetSize();

		for (int i=0; i < nNumChars; i++)
		{
			if ((*pList)[i])
			{
				CCharacterFX* pCFX = (CCharacterFX*)(*pList)[i];
				// PORT: NOLF2 re-added the character to its radar here for
				// its new team colour; Tron has no radar
				(void)pCFX;
			}
		}
	}						
}


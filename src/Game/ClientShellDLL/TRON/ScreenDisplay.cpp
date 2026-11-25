// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenDisplay.cpp
//
// PURPOSE : Interface screen for setting display options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenDisplay.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"
extern CGameClientShell* g_pGameClientShell;

#ifdef TRON20SRC_MODERN
// MODERN.md EN-44: the frame rate limit (the engine's MaxFPS: EN-40). The
// limits on offer, "none" (0) last; a MaxFPS set by hand that is not on
// offer shows as the next higher limit ("none" above the highest), and is
// written again only when the choice is changed. 240 is the engine's
// default (a config without MaxFPS).
namespace
{
	const int	kModernFrameLimits[] = { 60, 120, 144, 240, 0 };
	const int	kNumModernFrameLimits = sizeof(kModernFrameLimits) / sizeof(kModernFrameLimits[0]);
	const float	kModernDefaultMaxFPS = 240.0f;

	uint8		s_nModernFrameLimit = 3;		// the control's choice
	uint8		s_nModernFrameLimitRead = 3;	// the choice when the screen opened

	uint8 ModernFrameLimitIndex(float fMaxFPS)
	{
		int nMaxFPS = (int)(fMaxFPS + 0.5f);
		if (nMaxFPS <= 0)
			return (uint8)(kNumModernFrameLimits - 1);
		for (int i = 0; i < kNumModernFrameLimits - 1; i++)
		{
			if (nMaxFPS <= kModernFrameLimits[i])
				return (uint8)i;
		}
		return (uint8)(kNumModernFrameLimits - 1);
	}
}
#endif

namespace
{
	int kGap = 0;
	int kWidth = 100;
	LTBOOL bInitTex;
	LTBOOL bInitLM;

	const int	kNumSteps = 50;
	const float	kBreakpoint = (float)(kNumSteps/3);
	const float	kUpperSteps = (float)kNumSteps - kBreakpoint;
	const float fLowerRange = 1.0f - kMinGamma;
	const float fUpperRange = kMaxGamma - 1.0f;
	float ConvertToGamma(int slider)
	{
		float gamma = 1.0f;
		if (slider < (int)kBreakpoint)
		{
			float fRatio = (float)slider / kBreakpoint;
			fRatio = (float)sqrt(fRatio);
			gamma = kMinGamma + fRatio * fLowerRange;
		}
		else
		{
			float fRatio = ((float)slider - kBreakpoint) / kUpperSteps;
			fRatio *= fRatio;
			gamma = 1.0f + fRatio * fUpperRange;
		}

//		g_pLTClient->CPrint("Gamma: %0.2f",gamma);
//		g_pLTClient->CPrint("Slider: %d",slider);
		return gamma;

	}

	int	ConvertToSlider(float gamma)
	{
		int slider = 5;
		if (gamma < 1.0f)
		{
			float underVal = gamma - kMinGamma;
			float fRatio = underVal / fLowerRange;
			fRatio *= fRatio;
			slider = (int)(0.5f + kBreakpoint * fRatio);
			
		}
		else
		{
			float overVal = gamma - 1.0f;
			float fRatio = overVal / fUpperRange;
			fRatio = (float)sqrt(fRatio);
			slider = (int)(0.5f + kBreakpoint + kUpperSteps * fRatio);
		}
//		g_pLTClient->CPrint("Gamma: %0.2f",gamma);
//		g_pLTClient->CPrint("Slider: %d",slider);
		return slider;
	}
}

//helper function used for sorting
int ScreenDisplayCompare( const void *arg1, const void *arg2 )
{
	ScreenDisplayResolution *pRes1=(ScreenDisplayResolution *)arg1;
	ScreenDisplayResolution *pRes2=(ScreenDisplayResolution *)arg2;

	if (pRes1->m_dwWidth < pRes2->m_dwWidth)
	{
		return -1;
	}
	else if (pRes1->m_dwWidth > pRes2->m_dwWidth)
	{
		return 1;
	}
	else
	{
		if ( pRes1->m_dwHeight < pRes2->m_dwHeight )
		{
			return -1;
		}
		else if ( pRes1->m_dwHeight > pRes2->m_dwHeight )
		{
			return 1;
		}
		else
		{
			if ( pRes1->m_dwBitDepth < pRes2->m_dwBitDepth )
			{
				return -1;
			}
			else if ( pRes1->m_dwBitDepth > pRes2->m_dwBitDepth )
			{
				return 1;
			}

		}
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenDisplay::CScreenDisplay()
{

	m_bEscape		=	LTFALSE;

    m_pResolutionCtrl   = LTNULL;
#ifdef TRON20SRC_MODERN
	m_nModernWindowMode = 0;
	m_nModernAntiAlias = 0;
	m_nModernFov = 90;
	m_bModernCine43 = LTFALSE;
	m_pModernWindowCtrl = LTNULL;
#endif
}

#ifdef TRON20SRC_MODERN
// MODERN.md CL-33: the field of view's range on the Display screen
static const int kModernFovMin = 60;
static const int kModernFovMax = 120;
static const int kModernFovStep = 5;
// CL-34: the antialiasing cycle's sample counts
static const int kModernAASamples[] = { 0, 2, 4, 8 };
static const int kModernNumAA = sizeof(kModernAASamples) / sizeof(kModernAASamples[0]);
// CL-31, CL-34: ApplyDisplay restarts the renderer once for a new window
// mode or antialiasing (ProfileMgr.cpp)
extern bool g_bModernDisplayRestart;
#endif

CScreenDisplay::~CScreenDisplay()
{

}

// Build the screen
LTBOOL CScreenDisplay::Build()
{

	CreateTitle(IDS_TITLE_DISPLAYOPTIONS);

	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_DISPLAY,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_DISPLAY,"SliderWidth");

	// RETAIL: Tron's screen has no background frame (retail 0x10058a4e)

	// Build the array of renderers
	GetRendererData();

	// Add the "resolution" control
    m_pResolutionCtrl = AddCycle(IDS_DISPLAY_RESOLUTION,IDS_HELP_RESOLUTION,kGap);

	// Setup the resolution control based on the current renderer
	SetupResolutionCtrl();

	// RETAIL: Tron's screen has no hardware cursor control (retail
	// 0x10058a9b)

#ifdef TRON20SRC_MODERN
	// MODERN.md CL-31: the window mode, under the resolution
	m_pModernWindowCtrl = AddCycle(IDS_MODERN_WINDOWMODE,IDS_MODERN_HELP_WINDOWMODE,kGap,&m_nModernWindowMode);
	m_pModernWindowCtrl->AddString(LoadTempWString(IDS_MODERN_FULLSCREEN));
	m_pModernWindowCtrl->AddString(LoadTempWString(IDS_MODERN_WINDOW));
	m_pModernWindowCtrl->AddString(LoadTempWString(IDS_MODERN_BORDERLESS));
#endif

	AddToggle(IDS_VSYNC,IDS_HELP_VSYNC,kGap,&m_bVSync);

#ifdef TRON20SRC_MODERN
	// MODERN.md EN-44: the frame rate limit
	{
		CLTGUICycleCtrl *pModernLimit = AddCycle(IDS_MODERN_FRAMELIMIT,IDS_HELP_MODERN_FRAMELIMIT,kGap,&s_nModernFrameLimit);
		for (int i = 0; i < kNumModernFrameLimits; i++)
		{
			if (kModernFrameLimits[i] > 0)
			{
				char szLimit[16];
				sprintf(szLimit, "%d", kModernFrameLimits[i]);
				pModernLimit->AddString(szLimit);
			}
			else
			{
				pModernLimit->AddString(LoadTempString(IDS_MODERN_FRAMELIMIT_NONE));
			}
		}
	}

	// CL-34: antialiasing, up to 8x
	{
		CLTGUICycleCtrl *pAA = AddCycle(IDS_MODERN_ANTIALIAS,IDS_MODERN_HELP_ANTIALIAS,kGap,&m_nModernAntiAlias);
		pAA->AddString(LoadTempWString(IDS_OFF));
		pAA->AddString(LoadTempWString(IDS_MODERN_2X));
		pAA->AddString(LoadTempWString(IDS_MODERN_4X));
		pAA->AddString(LoadTempWString(IDS_MODERN_8X));
	}

	// CL-33: the field of view
	{
		CLTGUISlider *pFov = AddSlider(IDS_MODERN_FOV,IDS_MODERN_HELP_FOV,kGap,kWidth,-1,&m_nModernFov);
		pFov->SetSliderRange(kModernFovMin,kModernFovMax);
		pFov->SetSliderIncrement(kModernFovStep);
		pFov->SetNumericDisplay(LTTRUE);
	}

	// CL-32: the cinematics in 4:3
	AddToggle(IDS_MODERN_CINE43,IDS_MODERN_HELP_CINE43,kGap,&m_bModernCine43);
#endif



	m_pGamma = AddSlider(IDS_GAMMA,IDS_HELP_GAMMA,kGap,kWidth,-1,&m_nGamma);
	m_pGamma->SetSliderRange(0,kNumSteps);
	m_pGamma->SetSliderIncrement(1);

	// RETAIL: NOLF2 1.3's mark of the default gamma (retail 0x10058b3f)
	int nBaseGamma = ConvertToSlider(1.0f);
	float xoffset =  (m_pGamma->CalculateSliderOffset(nBaseGamma) / m_pGamma->GetScale());
	uint16 nHeight = m_pGamma->GetBarHeight();
	float yoffset = ((( (float)m_pGamma->GetHeight() / m_pGamma->GetScale()) - (float)nHeight)) / 2.0f;
	LTIntPt pos = m_pGamma->GetBasePos();
	pos.x += (int)(xoffset - 1.0f);
	pos.y += (int)(yoffset + 0.5f);
	CLTGUIFrame *pBar = debug_new(CLTGUIFrame);
	pBar->Create(0xBF000000,3,nHeight+1);
	pBar->SetBasePos(pos);
	pBar->SetScale(g_pInterfaceResMgr->GetXRatio());
	AddControl(pBar);

 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

void CScreenDisplay::Escape()
{
	m_bEscape = LTTRUE;
	CBaseScreen::Escape();
}

uint32 CScreenDisplay::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};

// Setup the resolution control based on the currently selected resolution
void CScreenDisplay::SetupResolutionCtrl()
{
	if (!m_pResolutionCtrl)
	{
		return;
	}


	// Get the selected renderer
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	// Clear the current resolutions
	m_pResolutionCtrl->RemoveAll();
    uint32 dwOldWidth=0;
    uint32 dwOldHeight=0;
    uint32 dwOldBitDepth=0;
    uint32 dwCurWidth = 0;
    uint32 dwCurHeight = 0;
    uint32 dwCurBitDepth = 0;
	int	 nNewRes = 0;

	unsigned int nResolutions=m_rendererData.m_resolutionArray.GetSize();

	if ((unsigned int)nResolutionIndex < nResolutions)
	{
		dwOldWidth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwWidth;
		dwOldHeight=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwHeight;
		dwOldBitDepth=m_rendererData.m_resolutionArray[nResolutionIndex].m_dwBitDepth;
	}


	// RETAIL: the text of each resolution is widened in one string
	std::wstring sText;

	// Add each resolution
	unsigned int i;
	for (i=0; i < nResolutions; i++)
	{
        uint32 dwWidth=m_rendererData.m_resolutionArray[i].m_dwWidth;
        uint32 dwHeight=m_rendererData.m_resolutionArray[i].m_dwHeight;
        uint32 dwBitDepth=m_rendererData.m_resolutionArray[i].m_dwBitDepth;

        if (    LTDIFF(dwWidth,dwOldWidth) <= LTDIFF(dwCurWidth,dwOldWidth) &&
                LTDIFF(dwHeight,dwOldHeight) <= LTDIFF(dwCurHeight,dwOldHeight) &&
                LTDIFF(dwBitDepth,dwOldBitDepth) < LTDIFF(dwCurBitDepth,dwOldBitDepth)
				)
		{
			nNewRes = i;
			dwCurWidth = dwWidth;
			dwCurHeight = dwHeight;
			dwCurBitDepth = dwBitDepth;
		}
		// Load the resolution format string.  This is "Resolution: [%dx%dx%d]" in English
		LTGUIWiden(sText, FormatTempString(IDS_DMODE_RESOLUTION, dwWidth, dwHeight, dwBitDepth));
		m_pResolutionCtrl->AddString(sText);
	}

	m_pResolutionCtrl->SetSelIndex(nNewRes);

}
// Build the array of renderers
void CScreenDisplay::GetRendererData()
{
	m_rendererData.m_description[0] = '\0';
	m_rendererData.m_internalName[0] = '\0';

	// Build the list of render modes
    RMode *pRenderModes=g_pLTClient->GetRenderModes();


	// Iterate through the list of render modes adding each one to the array
	RMode *pCurrentMode=pRenderModes;
    while (pCurrentMode != LTNULL)
	{
		if (pCurrentMode->m_Width >= 640 && pCurrentMode->m_Height >= 480 && pCurrentMode->m_BitDepth == 32)
		{
#ifdef TRON20SRC_MODERN
			// MODERN.md CL-30: every mode the display offers, at any shape
			// (16:9, 16:10, 21:9, 32:9, 5:4): the interface and the view
			// fit themselves to it. Retail listed only exact 4:3 modes.
			if (false)
#else
			// disallow non-standard aspect ratios
			uint32 testWidth = (pCurrentMode->m_Height * 4 / 3);
			if (pCurrentMode->m_Width != testWidth)
#endif
			{
			
				// Go to the next render mode
				pCurrentMode=pCurrentMode->m_pNext;
				continue;
			}

			//disallow any that aren't hardware TnL
			// RETAIL: Tron tests each mode alone (not against the current one)
			if(pCurrentMode->m_bHWTnL)
			{
				// Check to see if we need to add this renderer
				if (!m_rendererData.m_description[0])
				{
					m_rendererData.m_bHWTnL = pCurrentMode->m_bHWTnL;
					SAFE_STRCPY(m_rendererData.m_description, pCurrentMode->m_Description);
					SAFE_STRCPY(m_rendererData.m_internalName, pCurrentMode->m_InternalName);

				}

				// Add the display resolutions for this renderer
				ScreenDisplayResolution resolution;
				resolution.m_dwWidth=pCurrentMode->m_Width;
				resolution.m_dwHeight=pCurrentMode->m_Height;
				resolution.m_dwBitDepth=pCurrentMode->m_BitDepth;

				LTBOOL bFound = LTFALSE;
				uint32 i = 0;
				while (!bFound && i < m_rendererData.m_resolutionArray.GetSize())
				{
					bFound = (ScreenDisplayCompare(&(m_rendererData.m_resolutionArray[i]), &resolution) == 0);
					++i;
				}
				if (!bFound)
					m_rendererData.m_resolutionArray.Add(resolution);
			}
		}

		// Go to the next render mode
		pCurrentMode=pCurrentMode->m_pNext;
	}

	// Free the linked list of render modes
    g_pLTClient->RelinquishRenderModes(pRenderModes);

#ifdef TRON20SRC_MODERN
	// MODERN.md CL-31: a window can be any size (EN-31), and a borderless
	// window is the desktop's size: the mode the game runs at is listed
	// even when the adapter has no such mode, so the screen shows it.
	{
		RMode currentMode;
		if (g_pLTClient->GetRenderMode(&currentMode) == LT_OK && currentMode.m_Width >= 640 &&
			currentMode.m_Height >= 480 && currentMode.m_BitDepth == 32)
		{
			ScreenDisplayResolution resolution;
			resolution.m_dwWidth = currentMode.m_Width;
			resolution.m_dwHeight = currentMode.m_Height;
			resolution.m_dwBitDepth = currentMode.m_BitDepth;
			LTBOOL bFound = LTFALSE;
			for (uint32 i = 0; !bFound && i < m_rendererData.m_resolutionArray.GetSize(); i++)
				bFound = (ScreenDisplayCompare(&(m_rendererData.m_resolutionArray[i]), &resolution) == 0);
			if (!bFound)
			{
				if (!m_rendererData.m_description[0])
				{
					m_rendererData.m_bHWTnL = currentMode.m_bHWTnL;
					SAFE_STRCPY(m_rendererData.m_description, currentMode.m_Description);
					SAFE_STRCPY(m_rendererData.m_internalName, currentMode.m_InternalName);
				}
				m_rendererData.m_resolutionArray.Add(resolution);
			}
		}
	}
#endif

	// Sort the render resolution based on screen width and height
	SortRenderModes();
}

#ifdef TRON20SRC_MODERN
// MODERN.md CL-31 to CL-34: the options from their console variables
void CScreenDisplay::ModernReadOptions()
{
	if (GetConsoleInt("Windowed",0) == 0)
		m_nModernWindowMode = 0;
	else if (GetConsoleInt("WindowMode",0) == 2)
		m_nModernWindowMode = 2;
	else
		m_nModernWindowMode = 1;

	int nSamples = GetConsoleInt("AntiAliasFSOverSample",0);
	m_nModernAntiAlias = 0;
	for (int i = 0; i < kModernNumAA; i++)
	{
		if (nSamples >= kModernAASamples[i])
			m_nModernAntiAlias = (uint8)i;
	}

	m_nModernFov = (int)(GetConsoleFloat("FovX",90.0f) + 0.5f);
	if (m_nModernFov < kModernFovMin) m_nModernFov = kModernFovMin;
	if (m_nModernFov > kModernFovMax) m_nModernFov = kModernFovMax;

	m_bModernCine43 = (GetConsoleInt("Cinematics43",0) != 0);

	ModernUpdateResolutionCtrl();
}

// A borderless window is the desktop's size: the resolution is greyed out
void CScreenDisplay::ModernUpdateResolutionCtrl()
{
	if (m_pResolutionCtrl)
		m_pResolutionCtrl->Enable(m_nModernWindowMode != 2);
}

// Writes the options; true when the renderer must restart for them (a new
// window mode or antialiasing). The console variables are written with a
// "+", so autoexec.cfg keeps them; display.cfg keeps the display's too
// (SaveDisplaySettings).
bool CScreenDisplay::ModernApplyOptions()
{
	bool bRestart = false;

	int nWindowed = (m_nModernWindowMode != 0) ? 1 : 0;
	int nWindowMode = (m_nModernWindowMode == 2) ? 2 : 1;
	if (GetConsoleInt("Windowed",0) != nWindowed ||
		(nWindowed && GetConsoleInt("WindowMode",0) != nWindowMode))
		bRestart = true;
	WriteConsoleInt("Windowed",nWindowed);
	WriteConsoleInt("WindowMode",nWindowMode);

	int nSamples = kModernAASamples[(m_nModernAntiAlias < kModernNumAA) ? m_nModernAntiAlias : 0];
	if (GetConsoleInt("AntiAliasFSOverSample",0) != nSamples)
		bRestart = true;
	WriteConsoleInt("AntiAliasFSOverSample",nSamples);

	// CL-33: FovX is the horizontal angle at 4:3; FovY keeps retail's
	// shape of view (90 by 78 degrees: the tangents' ratio), and the
	// engine widens the view for the screen's shape (EN-30)
	float fFovX = (float)m_nModernFov;
	float fFovY = RAD2DEG(2.0f * (float)atan(tan(DEG2RAD(fFovX) * 0.5f) * tan(DEG2RAD(78.0f) * 0.5f) / tan(DEG2RAD(90.0f) * 0.5f)));
	WriteConsoleFloat("FovX",fFovX);
	WriteConsoleFloat("FovY",fFovY);

	WriteConsoleInt("Cinematics43",m_bModernCine43 ? 1 : 0);

	return bRestart;
}
#endif


// Sort the render resolution based on screen width and height
void CScreenDisplay::SortRenderModes()
{
	// Build a temporary array of render modes
	int nResolutions=m_rendererData.m_resolutionArray.GetSize();
	if ( nResolutions < 1 )
	{
		return;
	}

	// RETAIL: debug_newa and debug_deletea (0x100592c0): their inline bodies
	// leave CMoArray::Insert2 out of line (0x10059fb0)
	ScreenDisplayResolution *pResolutions = debug_newa(ScreenDisplayResolution, nResolutions);

	int i;
	for (i=0; i < nResolutions; i++)
	{
		pResolutions[i]=m_rendererData.m_resolutionArray[i];
	}

	// Sort the array
	qsort(pResolutions, nResolutions, sizeof(ScreenDisplayResolution), ScreenDisplayCompare);

	// Clear the current renderer resolutions array
	m_rendererData.m_resolutionArray.SetSize(0);

	// Copy the sorted array back to the resolutions array
	for (i=0; i < nResolutions; i++)
	{
		m_rendererData.m_resolutionArray.Add(pResolutions[i]);
	}

	debug_deletea(pResolutions);
}


// Gets a RMode structure based on a renderer index and a resolution index
RMode CScreenDisplay::GetRendererModeStruct(int nResolutionIndex)
{
	// Copy the renderer information from the renderer structure to the temporary RMode structure
	RMode mode;
	mode.m_bHWTnL = m_rendererData.m_bHWTnL;

	SAFE_STRCPY(mode.m_InternalName, m_rendererData.m_internalName);
	SAFE_STRCPY(mode.m_Description, m_rendererData.m_description);

	ScreenDisplayResolution resolution=m_rendererData.m_resolutionArray[nResolutionIndex];
	mode.m_Width=resolution.m_dwWidth;
	mode.m_Height=resolution.m_dwHeight;
	mode.m_BitDepth=resolution.m_dwBitDepth;

    mode.m_pNext=LTNULL;

	return mode;
}

// Returns TRUE if two renderers are the same

// Called when the screen gains or focus
void CScreenDisplay::OnFocus(LTBOOL bFocus)
{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{
		m_bEscape = LTFALSE;

		pProfile->SetDisplay();

		// RETAIL: Tron's has no hardware cursor control (retail
		// 0x1005946d)
		m_bVSync = pProfile->m_bVSync;

#ifdef TRON20SRC_MODERN
		// MODERN.md EN-44
		s_nModernFrameLimit = ModernFrameLimitIndex(GetConsoleFloat("MaxFPS", kModernDefaultMaxFPS));
		s_nModernFrameLimitRead = s_nModernFrameLimit;
#endif


		float gamma = pProfile->m_fGamma;

		m_nGamma = ConvertToSlider(gamma);

#ifdef TRON20SRC_MODERN
		// MODERN.md CL-30, CL-31: the list again (the mode may be a new
		// window size), then the options
		m_rendererData.m_resolutionArray.SetSize(0);
		m_rendererData.m_description[0] = '\0';
		GetRendererData();
		SetupResolutionCtrl();
		ModernReadOptions();
#endif


		// The current render mode
		RMode currentMode;
		g_pLTClient->GetRenderMode(&currentMode);

		// Set the renderer controls so that they match the currently selected renderer
		unsigned int i;
		for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
		{
			RMode mode=GetRendererModeStruct(i);

			if (IsRendererEqual(&currentMode, &mode))
			{
				// Setup the resolution control
				SetupResolutionCtrl();

				// Set the resolution index
				m_pResolutionCtrl->SetSelIndex(i);
			}
		}
			

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		// RETAIL: Tron's turns the hardware cursor off (retail 0x1005951f)
		g_pCursorMgr->UseHardwareCursor(LTFALSE);
		WriteConsoleInt("HardwareCursor", 0);

		if (m_bEscape)
		{

			pProfile->m_bHardwareCursor = LTFALSE;
			pProfile->m_bVSync = m_bVSync;
			pProfile->m_fGamma = ConvertToGamma(m_nGamma);
#ifdef TRON20SRC_MODERN
			// MODERN.md EN-44: the engine saves MaxFPS in autoexec.cfg
			if (s_nModernFrameLimit != s_nModernFrameLimitRead && s_nModernFrameLimit < kNumModernFrameLimits)
			{
				WriteConsoleFloat("MaxFPS", (float)kModernFrameLimits[s_nModernFrameLimit]);
				s_nModernFrameLimitRead = s_nModernFrameLimit;
			}
#endif
			// Set the render mode if we are losing focus
#ifdef TRON20SRC_MODERN
			// MODERN.md CL-30: and only if one is listed (retail read an
			// empty list at index -1)
			if (m_pResolutionCtrl &&
				(uint32)m_pResolutionCtrl->GetSelIndex() < m_rendererData.m_resolutionArray.GetSize())
#else
			if (m_pResolutionCtrl)
#endif

			{
				// Get the new renderer structure
				RMode newMode=GetRendererModeStruct(m_pResolutionCtrl->GetSelIndex());

				pProfile->m_nScreenWidth = newMode.m_Width;
				pProfile->m_nScreenHeight = newMode.m_Height;
				pProfile->m_nScreenDepth = newMode.m_BitDepth;
			}
	
#ifdef TRON20SRC_MODERN
			// MODERN.md CL-31 to CL-34: a new window mode or antialiasing
			// restarts the renderer once (ApplyDisplay)
			if (ModernApplyOptions())
				g_bModernDisplayRestart = true;
			// CL-31: a framed window must fit the desktop's work area with
			// its frame: a larger size takes the largest listed one that
			// fits, else the largest of the desktop's shape that fits
			if (m_nModernWindowMode == 1)
			{
				RECT rcWork;
				if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0))
					SetRect(&rcWork, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
				RECT rcFrame = { 0, 0, 0, 0 };
				AdjustWindowRect(&rcFrame, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
				int nMaxW = (rcWork.right - rcWork.left) - (rcFrame.right - rcFrame.left);
				int nMaxH = (rcWork.bottom - rcWork.top) - (rcFrame.bottom - rcFrame.top);
				if ((int)pProfile->m_nScreenWidth > nMaxW || (int)pProfile->m_nScreenHeight > nMaxH)
				{
					uint32 nBestW = 0, nBestH = 0;
					for (uint32 i = 0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
					{
						ScreenDisplayResolution res = m_rendererData.m_resolutionArray[i];
						if ((int)res.m_dwWidth <= nMaxW && (int)res.m_dwHeight <= nMaxH &&
							res.m_dwWidth * res.m_dwHeight > nBestW * nBestH)
						{
							nBestW = res.m_dwWidth;
							nBestH = res.m_dwHeight;
						}
					}
					if (!nBestW && nMaxW >= 640 && nMaxH >= 480)
					{
						// the desktop's shape, even sizes
						int nDeskW = GetSystemMetrics(SM_CXSCREEN);
						int nDeskH = GetSystemMetrics(SM_CYSCREEN);
						nBestH = (uint32)nMaxH & ~1;
						nBestW = (nDeskH > 0) ? ((uint32)(nBestH * nDeskW / nDeskH) & ~1) : (uint32)nMaxW;
						if ((int)nBestW > nMaxW)
						{
							nBestW = (uint32)nMaxW & ~1;
							nBestH = (nDeskW > 0) ? ((uint32)(nBestW * nDeskH / nDeskW) & ~1) : (uint32)nMaxH;
						}
					}
					if (nBestW >= 640 && nBestH >= 480)
					{
						pProfile->m_nScreenWidth = nBestW;
						pProfile->m_nScreenHeight = nBestH;
					}
				}
			}

#endif
			pProfile->ApplyDisplay();

			pProfile->Save();
				
		}




	}
	CBaseScreen::OnFocus(bFocus);
}


// Returns the currently selected resolution
ScreenDisplayResolution CScreenDisplay::GetCurrentSelectedResolution()
{
	int nResolutionIndex=m_pResolutionCtrl->GetSelIndex();

	_ASSERT(nResolutionIndex >= 0);
	if (nResolutionIndex >= 0)
		return m_rendererData.m_resolutionArray[nResolutionIndex];
	else
		return m_rendererData.m_resolutionArray[0];
}

// Set the resolution for the resolution control.  If it cannot be found the
// next highest resolution is selected.
void CScreenDisplay::SetCurrentCtrlResolution(ScreenDisplayResolution resolution)
{

	// Go through the resolutions searching for a match
	unsigned int i;
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		ScreenDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth == searchRes.m_dwWidth &&
			resolution.m_dwHeight == searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			m_pResolutionCtrl->SetSelIndex(i);
			return;
		}
	}

	// Since an exact match wasn't found, set it to the next highest resolution
	for (i=0; i < m_rendererData.m_resolutionArray.GetSize(); i++)
	{
		ScreenDisplayResolution searchRes=m_rendererData.m_resolutionArray[i];

		if (resolution.m_dwWidth > searchRes.m_dwWidth ||
			resolution.m_dwHeight > searchRes.m_dwHeight &&
			resolution.m_dwBitDepth == searchRes.m_dwBitDepth)
		{
			if (i > 0)
			{
				m_pResolutionCtrl->SetSelIndex(i-1);
			}
			else
			{
				m_pResolutionCtrl->SetSelIndex(0);
			}
			return;
		}
	}
}





LTBOOL CScreenDisplay::OnLButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		// RETAIL: Tron's (0x10059600, 0x100596c0) has no hardware cursor control
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnLButtonUp(x,y)) return LTFALSE;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return LTTRUE;
		}
	}
#ifdef TRON20SRC_MODERN
	{
		LTBOOL bModernResult = CBaseScreen::OnLButtonUp(x, y);
		if (GetSelectedControl() && GetSelectedControl() == m_pModernWindowCtrl)
		{
			// MODERN.md CL-31: a borderless window greys the resolution out
			m_pModernWindowCtrl->UpdateData(LTTRUE);
			ModernUpdateResolutionCtrl();
		}
		return bModernResult;
	}
#else
	return CBaseScreen::OnLButtonUp(x, y);
#endif
}

LTBOOL CScreenDisplay::OnRButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		// RETAIL: Tron's (0x10059600, 0x100596c0) has no hardware cursor control
		if (pCtrl == m_pGamma)
		{
			if (!pCtrl->OnRButtonUp(x,y)) return LTFALSE;
			pCtrl->UpdateData();
			float fGamma = ConvertToGamma(m_nGamma);
			WriteConsoleFloat("GammaR",fGamma);
			WriteConsoleFloat("GammaG",fGamma);
			WriteConsoleFloat("GammaB",fGamma);
			return LTTRUE;
		}
	}
#ifdef TRON20SRC_MODERN
	{
		LTBOOL bModernResult = CBaseScreen::OnRButtonUp(x, y);
		if (GetSelectedControl() && GetSelectedControl() == m_pModernWindowCtrl)
		{
			// MODERN.md CL-31: a borderless window greys the resolution out
			m_pModernWindowCtrl->UpdateData(LTTRUE);
			ModernUpdateResolutionCtrl();
		}
		return bModernResult;
	}
#else
	return CBaseScreen::OnRButtonUp(x, y);
#endif
}


LTBOOL CScreenDisplay::OnLeft()
{
	if (!CBaseScreen::OnLeft()) return LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	// RETAIL: Tron's (0x10059780, 0x10059810) has no hardware cursor control
	if (GetSelectedControl() == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}

#ifdef TRON20SRC_MODERN
	if (GetSelectedControl() && GetSelectedControl() == m_pModernWindowCtrl)
	{
		// MODERN.md CL-31: a borderless window greys the resolution out
		m_pModernWindowCtrl->UpdateData(LTTRUE);
		ModernUpdateResolutionCtrl();
	}
#endif
	return LTTRUE;
}
LTBOOL CScreenDisplay::OnRight()
{
	if (!CBaseScreen::OnRight()) return LTFALSE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	// RETAIL: Tron's (0x10059780, 0x10059810) has no hardware cursor control
	if (pCtrl == m_pGamma)
	{
		pCtrl->UpdateData();
		float fGamma = ConvertToGamma(m_nGamma);
		WriteConsoleFloat("GammaR",fGamma);
		WriteConsoleFloat("GammaG",fGamma);
		WriteConsoleFloat("GammaB",fGamma);
	}
#ifdef TRON20SRC_MODERN
	if (GetSelectedControl() && GetSelectedControl() == m_pModernWindowCtrl)
	{
		// MODERN.md CL-31: a borderless window greys the resolution out
		m_pModernWindowCtrl->UpdateData(LTTRUE);
		ModernUpdateResolutionCtrl();
	}
#endif
	return LTTRUE;
}
LTBOOL CScreenDisplay::OnEnter()
{
	return CBaseScreen::OnEnter();
}



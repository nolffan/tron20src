/*
 * clientpad.cpp -- the modern build's controller, the client's half. See
 * clientpad.h. #included once, by GameClientShell.cpp, in a
 * TRON20SRC_MODERN block (CShell.dll's project file is the retail one, so a
 * modern source of its own would have no place in it).
 *
 * The pad is read once a frame (Pad_Update), while the game's window is the
 * foreground one, through padread.h -- the reader the engine's ##gamepad
 * device uses, so the test pad (TRON20SRC_PADTEST) drives both.
 *
 * Look (CL-20): the right stick past a radial dead zone (PadLookDeadZone),
 * the rest of its travel rescaled to 0..1 and raised to PadLookCurve (fine
 * aim near the centre), is a turn rate: 180 degrees a second across, 120 up
 * and down, times PadLookSpeed and the frame time. PlayerMgr adds it to the
 * mouse's turn, so it slows with the zoom as the mouse does; PadInvertY
 * turns its up and down over (the mouse's own invert and mouse look
 * settings do not apply to it). On a light cycle it turns the camera, as
 * the mouse does.
 *
 * Movement (CL-20): the left stick past PadMoveDeadZone is a direction and
 * a deflection. The direction sets the movement flags (by octant: what the
 * animations and the server see) and the acceleration; the deflection the
 * speed: past three-quarters it runs (the Walk binding held turns that
 * over, as it does the keyboard's; Toggle walk is not used with the
 * stick), below it walks at a share of the walking speed, never less than
 * a third.
 *
 * Light cycles (CL-25): a turn is a push of the left stick to the left or
 * the right (past half its travel; back near the centre before the next),
 * or the d-pad's left or right: the Strafe left and Strafe right commands
 * that the keyboard's turns are. The right trigger or the left stick up
 * accelerates, the left trigger or the left stick down brakes (the
 * Forward and Backward commands' flags). The bindings of the triggers
 * (Fire, Disc recall / block) do nothing on a cycle.
 *
 * Keys (CL-22): outside play -- the menus, the screens, a message box, the
 * subroutine and rating screens, a paused game, a movie -- the d-pad or the
 * left stick is the arrow keys (repeating while held), A is Enter, B and
 * Start are Escape; in play Start is Escape (the menu), and A, in a
 * cut-scene or once dead, is Space (skips the cut-scene). They go through
 * the client shell's own OnKeyDown / OnKeyUp, as the engine delivers a key.
 * Not while Configure controls waits for a control to bind.
 *
 * Decisions (CL-22): while a decision window is up (a conversation's
 * choices, or an archive bin's downloads), the d-pad up and down move a
 * highlight (amber) over its choices, A takes the highlighted one and B is
 * Escape; the engine's bindings do not see those buttons meanwhile
 * (PadHoldButtons, EN-20), so taking a choice does not jump.
 *
 * Modern build only (TRON20SRC_MODERN).
 */

#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "HUDMgr.h"
#include "HUDDecision.h"
#include "TRON/HUDTronDecision.h"
#include "ProfileMgr.h"
#include "CommandIDs.h"
#include "SharedMovement.h"
#include "VarTrack.h"
#include "ClientUtilities.h"
#include "padread.h"
#include "clientpad.h"

#include <math.h>

extern HWND g_hMainWnd;		// GameClientShell.cpp

namespace
{
	VarTrack	s_vtLookSpeed;
	VarTrack	s_vtLookCurve;
	VarTrack	s_vtLookDeadZone;
	VarTrack	s_vtMoveDeadZone;
	VarTrack	s_vtInvertY;
	bool		s_bVarsInitted;

	const float	c_fYawRate		= 3.14159265f;			// 180 degrees a second
	const float	c_fPitchRate	= 3.14159265f * 2 / 3;	// 120
	const float	c_fRunAt		= 0.75f;				// of the stick's travel past the dead zone
	const float	c_fMinWalk		= 1.0f / 3;
	const float	c_fOctant		= 0.3827f;				// sin(22.5 degrees)
	const float	c_fRepeatDelay	= 0.4f;					// menu keys: the first repeat, then every
	const float	c_fRepeatRate	= 0.12f;
	const float	c_fTurnAt		= 0.5f;					// light cycle: a turn past this...
	const float	c_fTurnRearm	= 0.3f;					// ...and the next once back below this
	const BYTE	c_nTriggerAt	= 64;					// a trigger pressed (of 255)

	// The buttons a decision takes from the bindings while it is up.
	const WORD	c_wDecisionButtons = PAD_A | PAD_B | PAD_DPAD;

	PadState	s_Pad;
	bool		s_bPad;									// connected, and the game has the foreground
	float		s_fFrameTime;
	float		s_fMoveX, s_fMoveY, s_fMoveMag;			// past the dead zone; 0 when not moving
	float		s_fLookX, s_fLookY;
	bool		s_bCycle;								// in play on a light cycle
	int			s_nCycleTurn;							// the stick's turn held: -1, 1, or 0
	bool		s_bCapture;
	int			s_nHold = -1;							// PadHoldButtons as last written

	// The arrow key held, and its repeat timer.
	WORD		s_wPrevButtons;
	int			s_nHeldKey;
	float		s_fHeldTime;
	float		s_fNextRepeat;

	// The decision window up, and its highlighted choice.
	void*		s_pDecision;
	int			s_nPadChoice = -1;

	struct PadDefault
	{
		int			nCommand;
		const char*	pButton;
	};

	// CL-21: the controller's default layout, from Tron 2.0's commands
	// (ProfileMgr.cpp's g_CommandArray: no reload, the disc's recall /
	// block and attack modifier). Start is not bound: it is Escape, the
	// menu. Back is the subroutine screen, a binding of its own
	// (Pad_ApplyBack); the light cycle's turns and speed are the pad's
	// (CL-25).
	const PadDefault s_PadDefaults[] =
	{
		{ COMMAND_ID_FIRING,			"Right Trigger" },
		{ COMMAND_ID_DEFEND,			"Left Trigger" },
		{ COMMAND_ID_JUMP,				"A Button" },
		{ COMMAND_ID_DUCK,				"B Button" },
		{ COMMAND_ID_ACTIVATE,			"X Button" },
		{ COMMAND_ID_DEFEND_MODIFIER,	"Y Button" },
		{ COMMAND_ID_NEXT_WEAPON,		"Right Bumper" },
		{ COMMAND_ID_PREV_WEAPON,		"Left Bumper" },
		{ COMMAND_ID_RUN,				"Left Stick Click" },
		{ COMMAND_ID_CENTERVIEW,		"Right Stick Click" },
		{ COMMAND_ID_ZOOM_IN,			"D-Pad Up" },
		{ COMMAND_ID_ZOOM_OUT,			"D-Pad Down" },
		{ COMMAND_ID_HOLSTER,			"D-Pad Left" },
		{ COMMAND_ID_TOGGLE_PROGRESS,	"D-Pad Right" },
	};

	void InitVars()
	{
		if (s_bVarsInitted)
			return;

		s_vtLookSpeed.Init(g_pLTClient, "PadLookSpeed", LTNULL, 1.0f);
		s_vtLookCurve.Init(g_pLTClient, "PadLookCurve", LTNULL, 2.0f);
		s_vtLookDeadZone.Init(g_pLTClient, "PadLookDeadZone", LTNULL, 0.15f);
		s_vtMoveDeadZone.Init(g_pLTClient, "PadMoveDeadZone", LTNULL, 0.2f);
		s_vtInvertY.Init(g_pLTClient, "PadInvertY", LTNULL, 0.0f);
		s_bVarsInitted = true;
	}

	// A stick, -32768..32767 a side (y up positive), past a radial dead
	// zone: its direction kept, its length rescaled from the dead zone's
	// edge to 1.  Returns the length.
	float Stick(SHORT nX, SHORT nY, float fDeadZone, float* pX, float* pY)
	{
		float fX = nX / 32767.0f, fY = nY / 32767.0f;
		float fLen = (float)sqrt(fX * fX + fY * fY);
		float fScaled;

		if (fDeadZone < 0.0f) fDeadZone = 0.0f;
		if (fDeadZone > 0.9f) fDeadZone = 0.9f;

		if (fLen <= fDeadZone)
		{
			*pX = *pY = 0.0f;
			return 0.0f;
		}

		fScaled = (fLen - fDeadZone) / (1.0f - fDeadZone);
		if (fScaled > 1.0f)
			fScaled = 1.0f;

		*pX = fX / fLen * fScaled;
		*pY = fY / fLen * fScaled;
		return fScaled;
	}

	// A key, as the engine delivers one, and a command: through
	// IClientShell, where they are public.
	void PressKey(int nKey)
	{
		IClientShell* pShell = g_pGameClientShell;

		pShell->OnKeyDown(nKey, 1);
		pShell->OnKeyUp(nKey);
	}

	void PressCommand(int nCommand)
	{
		IClientShell* pShell = g_pGameClientShell;

		pShell->OnCommandOn(nCommand);
		pShell->OnCommandOff(nCommand);
	}

	bool InPlay()
	{
		return g_pInterfaceMgr && g_pInterfaceMgr->GetGameState() == GS_PLAYING;
	}

	bool OnLightCycle()
	{
		return g_pMoveMgr && g_pMoveMgr->GetVehicleMgr() &&
			g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE;
	}

	// The arrow key the d-pad (or, bStick, the left stick) holds, or 0.
	int Direction(bool bStick)
	{
		float fX, fY;

		if (s_Pad.wButtons & PAD_DPAD_UP)		return VK_UP;
		if (s_Pad.wButtons & PAD_DPAD_DOWN)		return VK_DOWN;
		if (s_Pad.wButtons & PAD_DPAD_LEFT)		return VK_LEFT;
		if (s_Pad.wButtons & PAD_DPAD_RIGHT)	return VK_RIGHT;

		if (!bStick || Stick(s_Pad.sThumbLX, s_Pad.sThumbLY, 0.5f, &fX, &fY) <= 0.0f)
			return 0;

		if (fabs(fY) >= fabs(fX))
			return fY > 0.0f ? VK_UP : VK_DOWN;

		return fX > 0.0f ? VK_RIGHT : VK_LEFT;
	}

	// The key held this frame: true once when it goes down, then again
	// after c_fRepeatDelay and every c_fRepeatRate while it is held.
	bool Repeat(int nKey)
	{
		if (nKey != s_nHeldKey)
		{
			s_nHeldKey    = nKey;
			s_fHeldTime   = 0.0f;
			s_fNextRepeat = c_fRepeatDelay;
			return nKey != 0;
		}

		if (!nKey)
			return false;

		s_fHeldTime += s_fFrameTime;

		if (s_fHeldTime >= s_fNextRepeat)
		{
			s_fNextRepeat += c_fRepeatRate;
			return true;
		}

		return false;
	}

	void SetHold(WORD wButtons)
	{
		if (s_nHold != (int)wButtons)
		{
			char szCmd[64];

			// "-": the engine reads it, autoexec.cfg does not keep it
			s_nHold = (int)wButtons;
			sprintf(szCmd, "-PadHoldButtons %d", s_nHold);
			g_pLTClient->RunConsoleString(szCmd);
		}
	}

	// The decision window up, if any: a Tron decision (the archive bins,
	// or a plain one) or the dialogue's.
	template <class T> bool UpdateDecision(T* pDecision, WORD wPressed)
	{
		int nKey, i, nNext, nMax = MAX_DECISION_CHOICES;

		if (!pDecision || !pDecision->IsVisible())
			return false;

		// a new window: the first choice highlighted, so that A takes it
		if (s_pDecision != pDecision || !pDecision->PadIsChoice(s_nPadChoice))
		{
			s_pDecision = pDecision;
			s_nPadChoice = -1;
			for (i = 0; i < nMax && s_nPadChoice < 0; i++)
			{
				if (pDecision->PadIsChoice(i))
					s_nPadChoice = i;
			}
		}

		nKey = Direction(false);

		if (Repeat(nKey) && s_nPadChoice >= 0 && (nKey == VK_UP || nKey == VK_DOWN))
		{
			// the next choice shown, up or down, round the ends
			nNext = s_nPadChoice;
			for (i = 0; i < nMax; i++)
			{
				nNext = (nNext + (nKey == VK_UP ? nMax - 1 : 1)) % nMax;
				if (pDecision->PadIsChoice(nNext))
					break;
			}
			s_nPadChoice = nNext;
		}

		pDecision->PadHighlight(s_nPadChoice);

		if ((wPressed & PAD_A) && s_nPadChoice >= 0)
			pDecision->Choose((uint8)s_nPadChoice);
		else if (wPressed & PAD_B)
			PressKey(VK_ESCAPE);		// cancels it (CInterfaceMgr::OnKeyDown)

		return true;
	}

	// The light cycle's turns: the left stick pushed aside, or the d-pad.
	void UpdateCycle(WORD wPressed)
	{
		float fX = s_Pad.sThumbLX / 32767.0f;

		if (s_nCycleTurn && fabs(fX) < c_fTurnRearm)
			s_nCycleTurn = 0;

		if (!s_nCycleTurn && fX <= -c_fTurnAt)
		{
			s_nCycleTurn = -1;
			PressCommand(COMMAND_ID_STRAFE_LEFT);
		}
		else if (!s_nCycleTurn && fX >= c_fTurnAt)
		{
			s_nCycleTurn = 1;
			PressCommand(COMMAND_ID_STRAFE_RIGHT);
		}

		if (wPressed & PAD_DPAD_LEFT)
			PressCommand(COMMAND_ID_STRAFE_LEFT);
		if (wPressed & PAD_DPAD_RIGHT)
			PressCommand(COMMAND_ID_STRAFE_RIGHT);
	}

	// In play: Start, A in a cut-scene or once dead, a decision's choices,
	// a light cycle's turns.
	void UpdatePlay(WORD wPressed)
	{
		bool bDecision = s_bPad &&
			(UpdateDecision(g_pTronDecision, wPressed) || UpdateDecision(g_pDecision, wPressed));

		SetHold(bDecision ? c_wDecisionButtons : 0);

		if (bDecision)
			return;

		s_pDecision = LTNULL;
		s_nPadChoice = -1;
		s_nHeldKey = 0;

		if (wPressed & PAD_START)
			PressKey(VK_ESCAPE);

		// A skips a cut-scene (Space, CPlayerMgr::OnKeyDown); A's own
		// binding (Jump) is a command, not a key.
		if ((wPressed & PAD_A) && g_pPlayerMgr &&
			(g_pPlayerMgr->IsUsingExternalCamera() || g_pPlayerMgr->IsPlayerDead()))
		{
			PressKey(VK_SPACE);
		}

		if (s_bCycle)
			UpdateCycle(wPressed);
		else
			s_nCycleTurn = 0;
	}

	// Outside play: the keys of the menus and screens.
	void UpdateMenus(WORD wPressed)
	{
		int nKey;

		SetHold(0);
		s_pDecision = LTNULL;
		s_nPadChoice = -1;

		if (s_bCapture)
		{
			s_nHeldKey = 0;
			return;
		}

		if (wPressed & PAD_A)
			PressKey(VK_RETURN);

		if (wPressed & (PAD_B | PAD_START))
			PressKey(VK_ESCAPE);

		nKey = Direction(true);

		if (Repeat(nKey))
			PressKey(nKey);
	}

	int FindCommand(int nCommandID)
	{
		int c;

		for (c = 0; c < g_kNumCommands; c++)
		{
			if (GetCommandData(c)->nCommandID == nCommandID)
				return c;
		}

		return -1;
	}
}

void Pad_Update(float fFrameTime)
{
	WORD wPressed;

	InitVars();

	s_fFrameTime = fFrameTime;

	// not with the launcher's "Disable joysticks" (DisableJoystick 1: no
	// AO_JOYSTICK, and the engine's JoystickDisable, EN-21)
	// MODERN.md EN-70: and not while the engine's console is down (its
	// keys and controls are the console's; GameClientShell.cpp)
	s_bPad = (g_pInterfaceMgr->GetAdvancedOptions() & AO_JOYSTICK) &&
		PadRead_Available() && GetForegroundWindow() == g_hMainWnd &&
		!g_bModernConsoleUp && PadRead_State(&s_Pad);

	if (!s_bPad)
		memset(&s_Pad, 0, sizeof(s_Pad));

	wPressed       = s_Pad.wButtons & ~s_wPrevButtons;
	s_wPrevButtons = s_Pad.wButtons;

	if (InPlay())
	{
		s_bCycle = OnLightCycle();
		s_fMoveMag = Stick(s_Pad.sThumbLX, s_Pad.sThumbLY, s_vtMoveDeadZone.GetFloat(0.2f), &s_fMoveX, &s_fMoveY);
		Stick(s_Pad.sThumbRX, s_Pad.sThumbRY, s_vtLookDeadZone.GetFloat(0.15f), &s_fLookX, &s_fLookY);
		UpdatePlay(wPressed);
	}
	else
	{
		s_bCycle = false;
		s_nCycleTurn = 0;
		s_fMoveX = s_fMoveY = s_fMoveMag = 0.0f;
		s_fLookX = s_fLookY = 0.0f;
		UpdateMenus(wPressed);
	}
}

void Pad_Term()
{
	PadRead_Term();
}

void Pad_GetLook(float* pYaw, float* pPitch)
{
	float fLen = (float)sqrt(s_fLookX * s_fLookX + s_fLookY * s_fLookY);
	float fScale, fCurve;

	*pYaw = *pPitch = 0.0f;

	if (fLen <= 0.0f)
		return;

	// The curve on the length, so that the direction stays the stick's.
	fCurve = s_vtLookCurve.GetFloat(2.0f);
	if (fCurve < 1.0f)
		fCurve = 1.0f;

	fScale = (float)pow(fLen, fCurve) / fLen * s_vtLookSpeed.GetFloat(1.0f) * s_fFrameTime;

	*pYaw = s_fLookX * fScale * c_fYawRate;

	// The stick's up is the camera's pitch up (negative) unless inverted.
	if (s_vtInvertY.GetFloat(0.0f) != 0.0f)
		*pPitch = s_fLookY * fScale * c_fPitchRate;
	else
		*pPitch = -s_fLookY * fScale * c_fPitchRate;
}

bool Pad_GetMove(float* pRight, float* pForward)
{
	if (s_fMoveMag <= 0.0f || s_bCycle)
		return false;

	*pRight   = s_fMoveX;
	*pForward = s_fMoveY;
	return true;
}

void Pad_MoveFlags(uint32* pdwFlags, bool bCanRun)
{
	bool bRun;

	if (s_fMoveMag <= 0.0f || s_bCycle)
		return;

	if (s_fMoveY >  c_fOctant * s_fMoveMag) *pdwFlags |= BC_CFLG_FORWARD;
	if (s_fMoveY < -c_fOctant * s_fMoveMag) *pdwFlags |= BC_CFLG_REVERSE;
	if (s_fMoveX >  c_fOctant * s_fMoveMag) *pdwFlags |= BC_CFLG_STRAFE_RIGHT;
	if (s_fMoveX < -c_fOctant * s_fMoveMag) *pdwFlags |= BC_CFLG_STRAFE_LEFT;

	// The deflection decides walking and running, not Toggle walk; the Walk
	// binding held turns it over.
	bRun = s_fMoveMag >= c_fRunAt;
	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN))
		bRun = !bRun;

	if (bRun && bCanRun)
		*pdwFlags |= BC_CFLG_RUN;
	else
		*pdwFlags &= ~BC_CFLG_RUN;
}

float Pad_SpeedScale()
{
	float fScale;

	if (s_fMoveMag <= 0.0f || s_fMoveMag >= c_fRunAt || s_bCycle)
		return 1.0f;

	fScale = s_fMoveMag / c_fRunAt;
	return fScale < c_fMinWalk ? c_fMinWalk : fScale;
}

void Pad_LightCycleSpeed(bool* pAccelerate, bool* pBrake)
{
	*pAccelerate = *pBrake = false;

	if (!s_bCycle || !s_bPad)
		return;

	*pAccelerate = s_Pad.bRightTrigger >= c_nTriggerAt ||
		(s_fMoveMag > 0.0f && s_fMoveY > c_fOctant * s_fMoveMag);
	*pBrake = s_Pad.bLeftTrigger >= c_nTriggerAt ||
		(s_fMoveMag > 0.0f && s_fMoveY < -c_fOctant * s_fMoveMag);
}

void Pad_SetCapture(bool bCapture)
{
	s_bCapture = bCapture;
}

void Pad_DefaultBindings(CUserProfile* pProfile, bool bReplace)
{
	const int nPad = 2;		// the profile's third column: the joystick or gamepad
	int i, c;

	if (bReplace)
	{
		for (c = 0; c < g_kNumCommands; c++)
		{
			CBindingData* pData = pProfile->FindBinding(c);

			if (pData)
			{
				pData->strRealName[nPad][0]    = '\0';
				pData->strTriggerName[nPad][0] = '\0';
				pData->nDeviceObjectId[nPad]   = 0;
			}
		}
	}

	for (i = 0; i < (int)(sizeof(s_PadDefaults) / sizeof(s_PadDefaults[0])); i++)
	{
		CBindingData* pData;
		bool bTaken = false;

		c = FindCommand(s_PadDefaults[i].nCommand);
		pData = c >= 0 ? pProfile->FindBinding(c) : LTNULL;

		// not over a binding the player made
		if (!pData || (pData->strRealName[nPad][0] && pData->strRealName[nPad][0] != ' '))
			continue;

		for (c = 0; c < g_kNumCommands && !bTaken; c++)
		{
			CBindingData* pOther = pProfile->FindBinding(c);
			bTaken = pOther && !stricmp(pOther->strRealName[nPad], s_PadDefaults[i].pButton);
		}

		if (bTaken)
			continue;

		SAFE_STRCPY(pData->strRealName[nPad], s_PadDefaults[i].pButton);
		SAFE_STRCPY(pData->strTriggerName[nPad], s_PadDefaults[i].pButton);
		pData->nDeviceObjectId[nPad] = 0;
	}
}

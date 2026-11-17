// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDLightCycleCircuitIntro.h"
#include "TronPlayerMgr.h"
#include "TronPlayerStats.h" // needed for energy transfer data
#include "HUDMgr.h"
#include "SubroutineMgr.h"
#include "ClientSoundMgr.h"
#include "LayoutMgr.h"
#include "CMoveMgr.h"
#include "ClientWeaponMgr.h"
#include "InterfaceMgr.h"
#include "RatingMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "VehicleMgr.h"
#include "PlayerCamera.h"
#include "DiscMatchMgr.h"
#include "LightCycleMatchMgr.h"
#include "TRON/LightCycleMgr.h"
#include "TRON/LightCycleButeMgr.h"
#include "ClientLightCycleMgr.h"
#include "LightCycleFX.h"
// RETAIL: its SLICE_SIZE and PRIM_SIZE are set up in this TU too (retail
// 0x100a4310, 0x100a4330: 0x1026ef4c, 0x1026ef48)
#include "ScreenSubroutines.h"
#include "TronHUDMgr.h"	// RETAIL: g_pLightCycleLives
#include "TRON/SubroutineDataMgr.h"
#include "VarTrack.h"

// CPlayerMgr's sway console variables (PlayerMgr.cpp)
extern VarTrack g_vtCameraSwayXFreq;
extern VarTrack g_vtCameraSwayYFreq;
extern VarTrack g_vtCameraSwayXSpeed;
extern VarTrack g_vtCameraSwayYSpeed;
extern VarTrack g_vtCameraSwayDuckMult;
extern VarTrack g_vtFOVXNormal;

CTronPlayerMgr* g_pTronPlayerMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::CTronPlayerMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::CTronPlayerMgr() : CPlayerMgr()
{
	// RETAIL: retail 0x100a4350
	m_fTronUnknown1F4 = 0;
	m_bTransferringEnergy = false;
	m_hTronUnknownSound22C = NULL;
	m_bTronUnknown1E9 = false;
	m_hTronUnknownSound224 = NULL;
	m_bTronUnknown1EA = false;
	m_hTronUnknownSound228 = NULL;
	m_bTronUnknown1EB = false;
	m_bTronUnknown234 = false;
	m_hTronUnknownBC = LTNULL;
	m_bTronUnknownB8 = false;
	m_nTronUnknown230 = 0;
	m_bTronUnknown235 = false;

	m_fFOVZoomLOL = 30.0f;
	m_fFOVZoomTriangulate1 = 20.0f;
	m_fFOVZoomTriangulate2 = 7.0f;
	m_fFOVZoomTriangulate3 = 2.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::~CTronPlayerMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronPlayerMgr::~CTronPlayerMgr()
{
	g_pTronPlayerMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::Init()
{
	g_pTronPlayerMgr = this;

	// RETAIL: retail 0x100a4460
	m_fTronUnknown1F4 = 0;
	m_bTransferringEnergy = false;
	m_hTronUnknownSound22C = NULL;
	m_bTronUnknown1E9 = false;
	m_hTronUnknownSound224 = NULL;
	m_bTronUnknown1EA = false;
	m_hTronUnknownSound228 = NULL;
	m_bTronUnknown1EB = false;

	m_fFOVZoomLOL = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomLOL", 30.0f);
	m_fFOVZoomTriangulate1 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate1", 20.0f);
	m_fFOVZoomTriangulate2 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate2", 7.0f);
	m_fFOVZoomTriangulate3 = g_pLayoutMgr->GetFloat("hudlayout0", "FOVZoomTriangulate3", 2.0f);

	// PORT: retail copies a string of a global (retail 0x10270864, +0x1d4)
	// into CPlayerMgr's +0x1b5 here; not yet identified

	m_bTronUnknown234 = false;
	m_nTronUnknown236 = 1;

	return CPlayerMgr::Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Term()
{
	// RETAIL: retail 0x100a4570
	m_fTronUnknown1F4 = 0;

	m_bTransferringEnergy = false;
	if (m_hTronUnknownSound22C)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
		m_hTronUnknownSound22C = NULL;
	}

	m_bTronUnknown1E9 = false;
	if (m_hTronUnknownSound224)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
		m_hTronUnknownSound224 = NULL;
	}

	m_bTronUnknown1EA = false;
	if (m_hTronUnknownSound228)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
		m_hTronUnknownSound228 = NULL;
	}

	m_bTronUnknown1EB = false;

	g_pTronPlayerMgr = NULL;

	CPlayerMgr::Term();
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOn(int command)
{
	// RETAIL: retail 0x100a4620
	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		case COMMAND_ID_FIRING :
		{
			// RETAIL: on a light cycle, the first press starts the cycle
			if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE &&
				!m_bTronUnknown235)
			{
				// RETAIL: the press uses the cycle's power-up (retail
				// 0x100a48c2)
				LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(g_pMoveMgr->GetObject());
				if (pCyclist && pCyclist->nPowerup != LC_NO_POWERUP)
				{
					if (((CClientLightCycleMgr*)g_pLightCycleMgr)->SendActivatePowerup(pCyclist))
					{
						m_bTronUnknown235 = true;
					}

					return LTTRUE;
				}
			}
		}
		break;

		case COMMAND_ID_ZOOM_IN :
		case COMMAND_ID_ZOOM_OUT :
		{
			// No zooming on a light cycle
			if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE) return LTTRUE;

			m_bTronUnknown210 = false;
			m_bTronUnknown211 = false;

			if (g_pSubroutineMgr->IsUtilitySubroutineActive("triangulate"))
			{
				m_bTronUnknown211 = true;
				// PORT: retail asks the subroutine manager (retail 0x10094fb0,
				// not yet in our tree) a flag of the triangulate subroutine
				m_bTronUnknown212 = false;
			}

			// The LOL zooms too
			if (m_pClientWeaponMgr->WeaponsEnabled() &&
				m_pClientWeaponMgr->GetCurrentClientWeapon()->GetWeapon() &&
				!_stricmp("LOL", m_pClientWeaponMgr->GetCurrentClientWeapon()->GetWeapon()->szName))
			{
				m_bTronUnknown210 = true;
			}

			if (m_bTronUnknown211 || m_bTronUnknown210)
			{
				int nZoomLevels = 1;
				if (m_bTronUnknown211)
				{
					nZoomLevels = g_pSubroutineMgr->GetActiveSubroutineVersion("triangulate") + 1;
				}

				if (!m_bZooming && nZoomLevels > 0)
				{
					int nOldZoom = m_nZoomView;

					if (command == COMMAND_ID_ZOOM_IN)
					{
						m_nZoomView = (m_nZoomView + 1) % (nZoomLevels + 1);
					}
					else if (m_nZoomView > 0)
					{
						m_nZoomView--;
					}

					if (m_nZoomView > nOldZoom)
					{
						m_bZooming   = LTTRUE;
						m_bZoomingIn = LTTRUE;

						if (nOldZoom == 0)
						{
							BeginZoom();
						}

						g_pInterfaceMgr->RemoveOverlay((eOverlayMask)14);
						g_pInterfaceMgr->RemoveOverlay((eOverlayMask)11);
						g_pInterfaceMgr->RemoveOverlay((eOverlayMask)8);

						if (m_bTronUnknown211)
						{
							g_pInterfaceMgr->CreateOverlay((eOverlayMask)(m_bTronUnknown212 ? 12 : 9));
						}

						if (m_bTronUnknown210)
						{
							g_pInterfaceMgr->CreateOverlay((eOverlayMask)6);
						}

						// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
						g_pClientSoundMgr->PlaySoundLocal("lolzoomin", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
					}
					else if (m_nZoomView < nOldZoom)
					{
						m_bZooming   = LTTRUE;
						m_bZoomingIn = LTFALSE;

						if (m_bTronUnknown211)
						{
							if (m_bTronUnknown212)
							{
								g_pInterfaceMgr->RemoveOverlay((eOverlayMask)14);
								g_pInterfaceMgr->CreateOverlay((eOverlayMask)13);
							}
							else
							{
								g_pInterfaceMgr->RemoveOverlay((eOverlayMask)11);
								g_pInterfaceMgr->CreateOverlay((eOverlayMask)10);
							}
						}

						if (m_bTronUnknown210)
						{
							g_pInterfaceMgr->RemoveOverlay((eOverlayMask)8);
							g_pInterfaceMgr->CreateOverlay((eOverlayMask)7);
						}

						g_pClientSoundMgr->PlaySoundLocal("lolzoomout", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
					}
				}
			}

			return LTTRUE;
		}
		break;

		default :
			break;
	}

	return CPlayerMgr::OnCommandOn(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnCommandOff(int command)
{
	// RETAIL: retail 0x100a4960
	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
            if (IsTransferringEnergy())
			{
				// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
				g_pClientSoundMgr->PlaySoundLocal("get_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

				// Clear the transfer flags
				m_bTransferringEnergy = false;
				if (m_hTronUnknownSound22C)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
					m_hTronUnknownSound22C = NULL;
				}

				m_bTronUnknown1E9 = false;
				if (m_hTronUnknownSound224)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
					m_hTronUnknownSound224 = NULL;
				}

				m_bTronUnknown1EA = false;
				if (m_hTronUnknownSound228)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
					m_hTronUnknownSound228 = NULL;
				}

				m_bTronUnknown1EB = false;

				HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
				hObj = m_hTronUnknown200;

				// release the lock on the target
				m_pTargetMgr->LockTarget(NULL);
				m_hTronUnknown200 = NULL;

				// Insufficient transfer.  Abort
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);	// RETAIL: 155 (0x9b) here
				cMsg.Writeuint8(1);
				cMsg.Writefloat(m_fEnergyTransferred);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				return LTTRUE;
			}
		}
		break;

        default :
			break;
	}

	return CPlayerMgr::OnCommandOff(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages specific to TRON
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// RETAIL: retail 0x100a4b80: six of Tron's messages
	switch(messageID)
	{
		// RETAIL: in retail's order (retail 0x100a4ba2)
		case MID_START_ENERGY_TRANSFER:	HandleMsgStartEnergyTransfer(pMsg);		break;
		case MID_STOP_ENERGY_TRANSFER:	HandleMsgStopEnergyTransfer(pMsg);		break;

		case MID_LIGHT_CYCLE_INFO:
			((CClientLightCycleMgr*)g_pLightCycleMgr)->OnMessage(pMsg);
		break;

		case MID_SYSTEM_MEMORY:			HandleMsgSystemMemory(pMsg);			break;
		case MID_TRON_SERVERMESSAGE:	HandleMsgTronUnknown200(pMsg);			break;
		case MID_TRON_UNKNOWN_152:		HandleMsgTronUnknown152(pMsg);			break;
		default:	return CPlayerMgr::OnMessage(messageID, pMsg);	break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Pause()
//
//	PURPOSE:	Note the pausing and unpausing of the game.
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Pause(LTBOOL bPause)
{
	// RETAIL: retail 0x100a4c90
	m_bPause = bPause;
	if (!bPause)
	{
		m_fLastUpdateTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnEnterWorld()
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a4cc0)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::OnEnterWorld()
{
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags2, FLAG2_PLAYERCOLLIDE, FLAG2_PLAYERCOLLIDE);
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY);

	m_bTronUnknown234 = false;
	m_bTronUnknown235 = false;

	// PORT: retail takes, in a light cycle circuit, a count (+0x84) of the
	// current entry of a Tron manager not yet in our tree (0x1026dd34,
	// retail 0x10047270)
	m_nTronUnknown236 = 1;

	m_hTronUnknownBC = LTNULL;
	m_bTronUnknownB8 = false;
	m_nTronUnknown230 = 0;

	CPlayerMgr::OnEnterWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnExitWorld()
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a4d60)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::OnExitWorld()
{
	if (m_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE || TronUnknownVirtual3C(true))
	{
		m_pPlayerCamera->TronUnknownSaveToProfile();
	}

	CPlayerMgr::OnExitWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateCameraZoom()
//
//	PURPOSE:	RETAIL: 0x100a4da0: CPlayerMgr's zoom with the LOL's and
//				the triangulate subroutine's FOVs; the vertical FOV keeps
//				the screen's aspect, and the zoom overlays come back when
//				the zoom ends
//
// ----------------------------------------------------------------------- //

#define TRON_ZOOM_TIME			0.5f
#define TRON_LOD_ZOOMADJUST		-5.0f

void CTronPlayerMgr::UpdateCameraZoom()
{
	char strConsole[30];

	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	float fovX, fovY;
	g_pLTClient->GetCameraFOV(m_hCamera, &fovX, &fovY);

	float fOldFovX = fovX;

	if (!fovX)
	{
		fovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
	}

	m_bZooming = LTTRUE;

	float fFovXZoomed, fZoomDist;

	if (m_bZoomingIn)
	{
		if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(m_bTronUnknown210 ? m_fFOVZoomLOL : m_fFOVZoomTriangulate1);
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - fFovXZoomed;
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(m_fFOVZoomTriangulate2);
			fZoomDist	= DEG2RAD(m_bTronUnknown210 ? m_fFOVZoomLOL : m_fFOVZoomTriangulate1) - fFovXZoomed;
		}
		else if (m_nZoomView == 3)
		{
			fFovXZoomed	= DEG2RAD(m_fFOVZoomTriangulate3);
			fZoomDist	= DEG2RAD(m_fFOVZoomTriangulate2) - fFovXZoomed;
		}
	}
	else
	{
		if (m_nZoomView == 0)
		{
			fFovXZoomed	= DEG2RAD(g_vtFOVXNormal.GetFloat());
			if (m_bTronUnknown210)
			{
				fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(m_fFOVZoomLOL);
			}
			else
			{
				fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(m_fFOVZoomTriangulate1);
			}
		}
		else if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(m_fFOVZoomTriangulate1);
			fZoomDist	= fFovXZoomed - DEG2RAD(m_fFOVZoomTriangulate2);
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(m_fFOVZoomTriangulate2);
			fZoomDist	= fFovXZoomed - DEG2RAD(m_fFOVZoomTriangulate3);
		}
	}

	float fZoomVel = fZoomDist / TRON_ZOOM_TIME;
	float fZoomAmount = fZoomVel * g_pGameClientShell->GetFrameTime();

	// Zoom camera in or out...

	if (m_bZoomingIn)
	{
		if (fovX > fFovXZoomed)
		{
			// Zoom camera in...

			fovX -= fZoomAmount;
		}

		if (fovX <= fFovXZoomed)
		{
			fovX = fFovXZoomed;
			m_bZooming = LTFALSE;
			g_pInterfaceMgr->EndZoom();

			if (m_bTronUnknown210)
			{
				g_pInterfaceMgr->CreateOverlay((eOverlayMask)8);
			}
			if (m_bTronUnknown211)
			{
				g_pInterfaceMgr->CreateOverlay((eOverlayMask)(m_bTronUnknown212 ? 14 : 11));
			}
		}
	}
	else  // Zoom camera out...
	{
		if (fovX < fFovXZoomed)
		{
			// Zoom camera out...

			fovX += fZoomAmount;
		}

		if (fovX >= fFovXZoomed)
		{
			fovX = fFovXZoomed;
			m_bZooming = LTFALSE;
			g_pInterfaceMgr->EndZoom();
			if (m_nZoomView == 0)
			{
				EndZoom();
			}
			else
			{
				if (m_bTronUnknown211)
				{
					g_pInterfaceMgr->CreateOverlay((eOverlayMask)(m_bTronUnknown212 ? 14 : 11));
				}
				if (m_bTronUnknown210)
				{
					g_pInterfaceMgr->CreateOverlay((eOverlayMask)8);
				}
			}
		}
	}

	if (fOldFovX != fovX)
	{
		// RETAIL: the vertical FOV keeps the screen's aspect
		float fHalfHeight = (float)dwHeight * 0.5f;
		float fHalfWidth = (float)dwWidth * 0.5f;
		fovY = (float)(2.0 * atan(fHalfHeight / (fHalfWidth / tan(fovX * 0.5f))));

		SetCameraFOV(fovX, fovY);

		// Update the lod adjustment for models...
		float fZoomAmount = (DEG2RAD(g_vtFOVXNormal.GetFloat()) - fovX) / (DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(m_fFOVZoomTriangulate3));
		float fNewLODOffset = m_fSaveLODScale + (TRON_LOD_ZOOMADJUST * fZoomAmount);

		sprintf(strConsole, "+ModelLODOffset %f", fNewLODOffset);
		g_pLTClient->RunConsoleString(strConsole);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateCameraSway()
//
//	PURPOSE:	RETAIL: 0x100a52a0: CPlayerMgr's sway, scaled by the
//				triangulate subroutine while it zooms
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdateCameraSway()
{
	// Apply...
	float swayAmount = g_pGameClientShell->GetFrameTime() / 1000.0f;

	float tm  = g_pLTClient->GetTime()/10.0f;

	// Adjust if ducking...
	float fMult = (m_dwPlayerFlags & BC_CFLG_DUCK) ? g_vtCameraSwayDuckMult.GetFloat() : 1.0f;

	//Adjust for skills
	// PORT: retail reads the player stats' virtual at +0x13c (a pointer to
	// a float at +4) as CPlayerMgr does; not yet in ours
	fMult *= g_pPlayerStats->GetSkillModifier(SKL_AIM,AimModifiers::eZoomSway);

	if (m_bTronUnknown211 && g_pSubroutineMgr->IsUtilitySubroutineActive("Triangulate"))
	{
		bool bCorrupt = g_pSubroutineMgr->IsSubroutineCorrupt("Triangulate");
		SubVersion eVersion = g_pSubroutineMgr->GetActiveSubroutineVersion("Triangulate");
		const TriangulateData& data = g_pSubroutineDataMgr->GetTriangulateData(SubroutineDataKey((uint8)eVersion, bCorrupt,
			g_pGameClientShell ? g_pGameClientShell->GetDifficulty() : 0));
		fMult *= data.fSwayAmountMultiplier;
		tm *= data.fSwaySpeedMultiplier;
	}

	float faddP = fMult * g_vtCameraSwayYSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayYFreq.GetFloat()) * swayAmount;
	float faddY = fMult * g_vtCameraSwayXSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayXFreq.GetFloat()) * swayAmount;

	m_fPitch += faddP;
	m_fYaw	 += faddY;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::EndZoom()
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a54d0): the LOL and triangulate
//				zoom overlays (Tron's overlay ids 8, 11, 14, not yet in our
//				Overlays.h) go too
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::EndZoom()
{
	g_pInterfaceMgr->EndScope();
	g_pInterfaceMgr->RemoveOverlay((eOverlayMask)8);
	g_pInterfaceMgr->RemoveOverlay((eOverlayMask)11);
	g_pInterfaceMgr->RemoveOverlay((eOverlayMask)14);

	m_bTronUnknown210 = false;
	m_bTronUnknown211 = false;
	m_bTronUnknown212 = false;
	m_bCamera = LTFALSE;

	m_pClientWeaponMgr->ShowWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetTronUnknown204 / GetTronUnknown20C
//
//	PURPOSE:	RETAIL: Tron's accessors (retail 0x100a5530, 0x100a5540;
//				the names are ours)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::GetTronUnknown204()
{
	return m_bTronUnknown204;
}

float CTronPlayerMgr::GetTronUnknown20C()
{
	return m_fTronUnknown20C;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Update()
{
	// RETAIL: retail 0x100a5550
	CPlayerMgr::Update();

	// RETAIL: the cycle's fx show when the camera is out of first person:
	// the player's own cycle, or the watched one (0x100a5559..0x100a55d0)
	if (m_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE)
	{
		m_pMoveMgr->GetVehicleMgr()->ShowTronUnknownFX2138(g_pMoveMgr->GetObject(),
			m_pPlayerCamera->GetCameraMode() != CPlayerCamera::FIRSTPERSON);
	}
	else if (m_nTronUnknown230 && m_bTronUnknownB8)
	{
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (psfxMgr)
		{
			CLightCycleFX* pFX = psfxMgr->GetLightCycleFX(((LIGHT_CYCLIST*)m_nTronUnknown230)->hObject);
			pFX->ShowTronUnknownFX48(((LIGHT_CYCLIST*)m_nTronUnknown230)->hObject,
				m_pPlayerCamera->GetCameraMode() != CPlayerCamera::FIRSTPERSON);
		}
	}

	if (m_bTronUnknown1D5 || m_bTronUnknown1D6 || m_bPause)
		return;

	// advance any timers.
	float fNow = g_pLTClient->GetTime();
	float fDeltaTime = fNow - m_fLastUpdateTime;
	m_fLastUpdateTime = fNow;

	if (m_bTransferringEnergy)
	{
		UpdateEnergyTransfer(fDeltaTime);
	}
	else if (m_bTronUnknown1E9)
	{
		UpdateEnergyGain(fDeltaTime);
	}
	else if (m_bTronUnknown1EA)
	{
		UpdateHealthGain(fDeltaTime);
	}
	else if (m_bTronUnknown1EB)
	{
		UpdateTronUnknownTransfer1EB(fDeltaTime);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateEnergyTransfer()
//
//	PURPOSE:	RETAIL: the energy the player gives (retail 0x100a5690)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdateEnergyTransfer(float fDeltaTime)
{
	// PORT: retail compares two of Tron's rating arrays (+0x1137, +0x1141:
	// the rating and its surge level); our CRatingMgr's IsSurgeRating
	// stands in
	float fEnergyPerSecond = g_pRatingMgr->IsSurgeRating(Rating_Stealth) ? 50.0f : (m_fTronUnknown1F4 * 15.0f + 5.0f);

	m_fEnergyTransferred += fEnergyPerSecond * fDeltaTime;
	g_pHUDMgr->QueueUpdate(kHUDEnergyTrans);
	g_pHUDMgr->QueueUpdate(kHUDEnergy);

	if (m_fEnergyTransferred > m_fEnergyRequired)
	{
		// TRANSFER COMPLETE
		m_fEnergyTransferred = 0;
		m_bTransferringEnergy = false;
		if (m_hTronUnknownSound22C)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
			m_hTronUnknownSound22C = NULL;
		}

		HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
		hObj = m_hTronUnknown200;
		m_pTargetMgr->LockTarget(NULL);
		m_hTronUnknown200 = NULL;

		// Tell the playerstatsmgr about the energy spent
		g_pTronPlayerStats->UseEnergy(m_fEnergyRequired);

		// Notify the binary bit that the energy transfer is complete
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		cMsg.Writeuint8(0);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateHealthGain()
//
//	PURPOSE:	RETAIL: the health the player takes (retail 0x100a5860)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdateHealthGain(float fDeltaTime)
{
	int nOldTransferred = (int)floor(m_fEnergyTransferred);

	// PORT: see UpdateEnergyTransfer
	float fPerSecond = g_pRatingMgr->IsSurgeRating(Rating_Stealth) ? 100.0f : (m_fTronUnknown1F4 * 30.0f + 20.0f);

	m_fEnergyTransferred += fPerSecond * fDeltaTime;
	bool bDone = false;
	if (m_fEnergyTransferred > m_fEnergyRequired)
	{
		bDone = true;
		m_fEnergyTransferred = m_fEnergyRequired;
	}

	int nTransferred = (int)floor(m_fEnergyTransferred);
	if (nTransferred != nOldTransferred)
	{
		int nDelta = nTransferred - nOldTransferred;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(m_hTronUnknown200);
		cMsg.Writeuint32(MID_ENERGY_TRANSFER_COMPLETE);
		cMsg.Writeuint32(nDelta);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	if (bDone)
	{
		m_bTronUnknown1EA = false;
		if (m_hTronUnknownSound228)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
			m_hTronUnknownSound228 = NULL;
		}

		// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
		g_pClientSoundMgr->PlaySoundLocal("get_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

		HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
		hObj = m_hTronUnknown200;
		m_pTargetMgr->LockTarget(NULL);
		m_hTronUnknown200 = NULL;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fEnergyTransferred);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		m_fEnergyTransferred = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateEnergyGain()
//
//	PURPOSE:	RETAIL: the energy the player takes (retail 0x100a5b20)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdateEnergyGain(float fDeltaTime)
{
	int nOldTransferred = (int)floor(m_fEnergyTransferred);

	// PORT: see UpdateEnergyTransfer
	float fPerSecond = g_pRatingMgr->IsSurgeRating(Rating_Stealth) ? 100.0f : (m_fTronUnknown1F4 * 30.0f + 20.0f);

	m_fEnergyTransferred += fPerSecond * fDeltaTime;
	bool bDone = false;
	if (m_fEnergyTransferred > m_fEnergyRequired)
	{
		bDone = true;
		m_fEnergyTransferred = m_fEnergyRequired;
	}

	int nTransferred = (int)floor(m_fEnergyTransferred);
	if (nTransferred != nOldTransferred)
	{
		int nDelta = nTransferred - nOldTransferred;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(m_hTronUnknown200);
		cMsg.Writeuint32(MID_ENERGY_TRANSFER_COMPLETE);
		cMsg.Writeuint32(nDelta);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	if (bDone)
	{
		m_bTronUnknown1E9 = false;
		if (m_hTronUnknownSound224)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
			m_hTronUnknownSound224 = NULL;
		}

		g_pClientSoundMgr->PlaySoundLocal("get_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

		HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
		hObj = m_hTronUnknown200;
		m_pTargetMgr->LockTarget(NULL);
		m_hTronUnknown200 = NULL;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fEnergyTransferred);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		m_fEnergyTransferred = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdateTronUnknownTransfer1EB()
//
//	PURPOSE:	RETAIL: the fourth, silent transfer (retail 0x100a5de0)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdateTronUnknownTransfer1EB(float fDeltaTime)
{
	int nOldTransferred = (int)floor(m_fEnergyTransferred);

	// PORT: see UpdateEnergyTransfer
	float fPerSecond = g_pRatingMgr->IsSurgeRating(Rating_Stealth) ? 20.0f : (m_fTronUnknown1F4 * 8.0f + 2.0f);

	m_fEnergyTransferred += fPerSecond * fDeltaTime;
	g_pHUDMgr->QueueUpdate(kHUDEnergyTrans);

	int nTransferred = (int)floor(m_fEnergyTransferred);
	if (m_fEnergyTransferred > m_fEnergyRequired)
	{
		m_fEnergyTransferred = m_fEnergyRequired;
		m_bTronUnknown1EB = false;

		HOBJECT hObj = m_pTargetMgr->GetLockedTarget();
		hObj = m_hTronUnknown200;
		m_pTargetMgr->LockTarget(NULL);
		m_hTronUnknown200 = NULL;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fEnergyTransferred);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		m_fEnergyTransferred = 0;
	}
	else if (nTransferred != nOldTransferred)
	{
		int nDelta = nTransferred - nOldTransferred;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(m_hTronUnknown200);
		cMsg.Writeuint32(MID_ENERGY_TRANSFER_COMPLETE);
		cMsg.Writeuint32(nDelta);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::InitTargetMgr()
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::InitTargetMgr()
{
	m_pTargetMgr = debug_new( CTronTargetMgr );
	ASSERT( 0 != m_pTargetMgr );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::HandleMsgStartEnergyTransfer()
//
//	PURPOSE:	RETAIL: Tron's message 154 (retail 0x100a6080): one of the
//				four transfers starts
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::HandleMsgStartEnergyTransfer(ILTMessage_Read * pMsg)
{
	uint8 nType = pMsg->Readuint8();
	HOBJECT hObj = pMsg->ReadObject();
	HOBJECT hTarget = pMsg->ReadObject();
	int iEnergyRequired = pMsg->Readuint32();

	if (!hTarget || !hObj)
		return;

	m_fEnergyTransferred = 0.0f;
	m_fEnergyRequired = 0.0f;

	// PORT: retail scales the rating (3) between two of Tron's rating
	// arrays (+0x113c, +0x1141: its least and its surge level), which our
	// CRatingMgr has not; the plain rating stands in
	m_fTronUnknown1F4 = (float)g_pRatingMgr->GetRating(Rating_Stealth) / 100.0f;

	switch (nType)
	{
		case 0:
		{
			// The player gives energy
			if (iEnergyRequired > (int)g_pTronPlayerStats->GetEnergy())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
				cMsg.Writeuint8(2);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

				g_pPickupMsgs->AddMessage(IDS_INSUFFICIENT_ENERGY, (const char*)LTNULL);
				// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
				g_pClientSoundMgr->PlaySoundLocal("insufficient_energy", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
				return;
			}

			m_bTransferringEnergy = true;
			m_hTronUnknownSound22C = g_pClientSoundMgr->PlaySoundLocal("bitpanel_dl", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}
		break;

		case 1:
		{
			// The player takes energy, unless full
			if ((int)g_pTronPlayerStats->GetEnergy() >= (int)g_pTronPlayerStats->GetMaxEnergy())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
				cMsg.Writeuint8(1);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				return;
			}

			m_bTronUnknown1E9 = true;
			m_hTronUnknownSound224 = g_pClientSoundMgr->PlaySoundLocal("get_energy", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}
		break;

		case 2:
		{
			// The player takes health, unless full
			if (g_pTronPlayerStats->GetHealth() >= g_pTronPlayerStats->GetMaxHealth())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_OBJECT_MESSAGE);
				cMsg.WriteObject(hObj);
				cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
				cMsg.Writeuint8(1);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
				return;
			}

			m_bTronUnknown1EA = true;
			m_hTronUnknownSound228 = g_pClientSoundMgr->PlaySoundLocal("get_health", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
		}
		break;

		case 3:
		{
			m_bTronUnknown1EB = true;
		}
		break;
	}

	m_fEnergyRequired = (float)iEnergyRequired;

	// lock targetmgr on this object until the player releases the activate key.
	m_pTargetMgr->LockTarget(hTarget);
	m_hTronUnknown200 = hObj;

	m_fLastUpdateTime = g_pLTClient->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::HandleMsgStopEnergyTransfer()
//
//	PURPOSE:	RETAIL: Tron's message 155 (retail 0x100a6520): the server
//				ends the transfers
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::HandleMsgStopEnergyTransfer(ILTMessage_Read * pMsg)
{
	uint8 nType = pMsg->Readuint8();
	HOBJECT hObj = pMsg->ReadObject();
	HOBJECT hTarget = pMsg->ReadObject();

	m_fEnergyTransferred = 0.0f;

	// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
	g_pClientSoundMgr->PlaySoundLocal("get_done", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);

	m_bTransferringEnergy = false;
	if (m_hTronUnknownSound22C)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound22C);
		m_hTronUnknownSound22C = NULL;
	}

	m_bTronUnknown1E9 = false;
	if (m_hTronUnknownSound224)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound224);
		m_hTronUnknownSound224 = NULL;
	}

	m_bTronUnknown1EA = false;
	if (m_hTronUnknownSound228)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSound228);
		m_hTronUnknownSound228 = NULL;
	}

	m_bTronUnknown1EB = false;

	if (m_hTronUnknown200 == hObj && m_pTargetMgr->GetLockedTarget() == hTarget)
	{
		m_pTargetMgr->GetLockedTarget();
		m_pTargetMgr->LockTarget(NULL);
		m_hTronUnknown200 = NULL;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_OBJECT_MESSAGE);
		cMsg.WriteObject(hObj);
		cMsg.Writeuint32(MID_STOP_ENERGY_TRANSFER);
		cMsg.Writeuint8(nType);
		cMsg.Writefloat(m_fEnergyTransferred);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::HandleMsgSystemMemory()
//
//	PURPOSE:	RETAIL: Tron's message 193 (retail 0x100a6740): the system
//				memory configuration (sectors of "alpha", "beta", "gold"
//				subroutines, bad blocks, base code), given to the subroutine
//				manager. PORT: not yet written; the message is skipped
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::HandleMsgSystemMemory(ILTMessage_Read *pMsg)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::HandleMsgTronUnknown200()
//
//	PURPOSE:	RETAIL: Tron's message 200 (retail 0x100a6e50)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::HandleMsgTronUnknown200(ILTMessage_Read *pMsg)
{
	GameType eGameType = g_pGameClientShell->GetGameType();

	if (pMsg->Peekuint8() != 100)
	{
		// RETAIL: 0x100a6e71 asks IsDiscGameType
		if (IsDiscGameType())
		{
			g_pDiscMatchMgr->OnMessage(pMsg);
		}
		else if (eGameType == eGameTypeMultiplayerLightcycles)
		{
			g_pLightCycleMatchMgr->OnMessage(pMsg);
		}
	}
	else
	{
		pMsg->Readuint8();
		ShowTronUnknownMessageBox(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::HandleMsgTronUnknown152()
//
//	PURPOSE:	RETAIL: Tron's message 152 (retail 0x100a6ec0)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::HandleMsgTronUnknown152(ILTMessage_Read *pMsg)
{
	m_bTronUnknown234 = pMsg->Readbool();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::ShowTronUnknownMessageBox()
//
//	PURPOSE:	RETAIL: retail 0x100a6ee0: a message box of Tron's string
//				3482
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::ShowTronUnknownMessageBox(ILTMessage_Read *pMsg)
{
	MBCreate mb;
	mb.eType = LTMB_OK;
	g_pInterfaceMgr->ShowMessageBox(3482, &mb);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetPercentEnergyTransferred()
//
// ----------------------------------------------------------------------- //

float CTronPlayerMgr::GetPercentEnergyTransferred()
{
	// RETAIL: retail 0x100a6f30
	if (!IsTransferringEnergy())
		return 0.0f;

	if (!(int)m_fEnergyRequired)
		return 0.0f;

	return (m_fEnergyTransferred / m_fEnergyRequired);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::GetJumpVelocity
//
//	PURPOSE:	Gets the player's jump velocity which is dependent
//				on certain subroutines
//
// ----------------------------------------------------------------------- //
float CTronPlayerMgr::GetJumpVelocity(float fJumpVel, float fSuperJumpVel)
{
	// Check for existence of the jump enhancing subroutine
	if(g_pSubroutineMgr->IsUtilitySubroutineActive("YAmp"))
	{
		float fDelta = fSuperJumpVel - fJumpVel;

		// Check the state of it
		switch(g_pSubroutineMgr->GetActiveSubroutineVersion("YAmp"))
		{
			case VERSION_ALPHA:
			{
				return(fJumpVel + (fDelta * 0.33f));
			}
			case VERSION_BETA:
			{
				return(fJumpVel + (fDelta * 0.66f));
			}
			case VERSION_GOLD:
			{
				return(fSuperJumpVel);
			}
			break;
		}
	}

	return fJumpVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual50
//
//	PURPOSE:	RETAIL: retail 0x100a7030 (slot 0x50): onto a light cycle
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual50()
{
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags2, 0, FLAG2_PLAYERCOLLIDE);
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags, 0, FLAG_GRAVITY);

	if (m_pClientWeaponMgr)
	{
		m_pClientWeaponMgr->HideWeapons();
	}

	HOBJECT hObj = g_pMoveMgr->GetObject();
	LTVector vPos;
	g_pLTClient->GetObjectPos(hObj, &vPos);

	// PORT: retail has the client light cycle manager (0x10236d98, its
	// 0x100ffec0) place the position on the grid
	LTVector vNewPos = vPos;
	g_pPhysicsLT->MoveObject(hObj, &vNewPos, MOVEOBJECT_TELEPORT);
	g_pLTClient->GetObjectPos(hObj, &vPos);

	m_bTronUnknown1D9 = false;
	m_hTronUnknownBC = LTNULL;
	m_bTronUnknownB8 = false;
	m_nTronUnknown230 = 0;

	// PORT: retail clears a flag of the game client shell (+0x833a) here

	// RETAIL: starts the circuit intro (0x1026ee28, its 0x1002cdf0)
	g_pLightCycleCircuitIntro->Start();

	TronUnknownVirtualA8(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual54
//
//	PURPOSE:	RETAIL: retail 0x100a7130 (slot 0x54): off the light cycle
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual54()
{
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags2, FLAG2_PLAYERCOLLIDE, FLAG2_PLAYERCOLLIDE);
	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY);

	if (m_pClientWeaponMgr)
	{
		m_pClientWeaponMgr->ShowWeapons();
	}

	m_bTronUnknown1D9 = false;
	m_hTronUnknownBC = LTNULL;
	m_bTronUnknownB8 = false;
	m_nTronUnknown230 = 0;

	m_pPlayerCamera->TronUnknownSaveToProfile();

	TronUnknownVirtualA8(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::UpdatePlaying
//
//	PURPOSE:	RETAIL: retail 0x100a71b0 (slot 0x20): on a light cycle, the
//				grid sound's volume goes with the camera's height
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::UpdatePlaying()
{
	CPlayerMgr::UpdatePlaying();

	if (g_pMoveMgr && g_pMoveMgr->GetVehicleMgr() &&
		g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE)
	{
		ShowPlayer(LTFALSE);

		LTVector vPos;
		g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);

		float fCameraY = m_pPlayerCamera->GetPos().y;

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(g_pMoveMgr->GetObject(), &vDims);

		float fHeight = fCameraY - (vPos.y - vDims.y);

		float fMin = 300.0f;
		float fMax = 1500.0f;

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("lcgridsoundmin");
		if (hVar)
		{
			fMin = g_pLTClient->GetVarValueFloat(hVar);
		}

		hVar = g_pLTClient->GetConsoleVar("lcgridsoundmax");
		if (hVar)
		{
			fMax = g_pLTClient->GetVarValueFloat(hVar);
		}

		uint8 nVolume;
		if (fHeight <= fMin)
		{
			nVolume = 100;
		}
		else if (fHeight >= fMax)
		{
			nVolume = 0;
		}
		else
		{
			nVolume = 100 - (int)((fHeight - fMin) / (fMax - fMin) * 100.0f);
		}

		// PORT: retail sets the volume (the sound manager's slot 0x84, not
		// in our SDK) of a sound of Tron's vehicle manager (+0x2c)
		nVolume;
	}

	// PORT: unless a flag of the game client shell (+0x7e49) is set,
	// retail updates the client light cycle manager (0x10236d98, its slot
	// 0x1c), not yet in our tree

	if (GetGameType() == eGameTypeMultiplayerLightcycles && g_pPlayerMgr->TronUnknownVirtual3C(true))
	{
		TronUnknownVirtual40();
		UpdateCamera();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual60
//
//	PURPOSE:	RETAIL: retail 0x100a7360 (slot 0x60): the player's light
//				cycle is derezzed
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual60()
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(g_pMoveMgr->GetObject());
	if (pCyclist)
	{
		g_pLightCycleMgr->DerezCyclist(pCyclist, LTNULL, true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual64
//
//	PURPOSE:	RETAIL: retail 0x100a7390 (slot 0x64): moves the player as
//				the physics would, kept on the light cycle grid
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual64()
{
	if (TronUnknownVirtual3C(false))
		return true;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	MoveInfo info;
	info.m_hObject = hObj;

	LTVector vVel;
	g_pPhysicsLT->GetVelocity(hObj, &vVel);

	info.m_dt = g_pGameClientShell->GetFrameTime();
	((ILTClientPhysics*)g_pPhysicsLT)->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(hObj, &vPos);

		LTVector vNewPos = vPos + info.m_Offset;

		// PORT: retail has the client light cycle manager (0x10236d98, its
		// 0x100ffec0) place the position on the grid

		g_pPhysicsLT->MoveObject(hObj, &vNewPos, 0);

		LTVector vMovedPos;
		g_pLTClient->GetObjectPos(hObj, &vMovedPos);

		if ((int)vNewPos.x != (int)vMovedPos.x || (int)vNewPos.z != (int)vMovedPos.z)
		{
			TronUnknownVirtual60();
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual68
//
//	PURPOSE:	RETAIL: retail 0x100a74d0 (slot 0x68): the light cycle's
//				controls, given to the client light cycle manager
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual68(int nFlags)
{
	HOBJECT hObj = g_pMoveMgr->GetObject();
	if (!hObj)
		return;

	LTVector vZeroAccel(0.0f, 0.0f, 0.0f);
	LTVector vZeroVel(0.0f, 0.0f, 0.0f);

	// PORT: retail tests a member of Tron's CMoveMgr (+0xd0) here, not yet
	// in our CMoveMgr

	if (m_ePlayerState == PS_DEAD || m_ePlayerState == PS_DYING || !m_bAllowPlayerMovement)
	{
		g_pPhysicsLT->SetAcceleration(hObj, &vZeroAccel);
		g_pPhysicsLT->SetVelocity(hObj, &vZeroVel);
		return;
	}

	if ((nFlags & 0x10) || (nFlags & 0x100))
	{
		char szSound[128];
		char szUnused[128];
		TronUnknownVirtual7C(hObj, szUnused, szSound, LTNULL);

		// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
		g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT, 100, 1.0f, 0);
	}

	// PORT: retail gives the flags (with 0x100000 in a speed zone, 0x1000000
	// in a slow zone) to the client light cycle manager (0x10236d98, its
	// slot 0x30)
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtualA0
//
//	PURPOSE:	RETAIL: retail 0x100a7650 (slot 0xa0). PORT: retail returns a
//				member of Tron's interface manager (+0x28af0), not yet in
//				ours
//
// ----------------------------------------------------------------------- //

int CTronPlayerMgr::TronUnknownVirtualA0()
{
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual9C
//
//	PURPOSE:	RETAIL: retail 0x100a7660 (slot 0x9c): fills a light cycle's
//				data (+0x4c name, +0x104c and +0x204c lists) from the
//				model attributes of the player's light cycle color (+0x1b5).
//				PORT: Tron's model attributes (0x10044f40, 0x10044f20) are
//				not yet in our CModelButeMgr
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual9C(void* pData, int nArg)
{
	g_pLTClient->CPrint("ERROR - Couldn't find Light Cycle Color %s\n", m_szTronUnknown1B5);
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::OnSubroutineChanged
//
//	PURPOSE:	RETAIL: retail 0x100a7720: the subroutine manager tells of a
//				subroutine's state; the server hears too
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::OnSubroutineChanged(const char* pszName, bool bActive, bool bVersion, int nVersion)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_MESSAGE);
	cMsg.WriteObject(g_pMoveMgr->GetServerObject());
	cMsg.Writeuint32(137);	// RETAIL: Tron's subroutine message
	cMsg.WriteString(pszName);
	cMsg.Writebool(bActive);
	cMsg.Writebool(bVersion);
	cMsg.Writeuint8(nVersion);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	if (!_strcmpi(pszName, "profiler"))
	{
		m_bTronUnknown204 = bActive;
		m_bTronUnknown205 = bVersion;
		m_nTronUnknown208 = nVersion;
		// PORT: retail asks the subroutine manager (its 0x10095730, not yet
		// in our tree) for a float
		m_fTronUnknown20C = 0.0f;
	}
	else if (!_strcmpi(pszName, "triangulate") && m_bTronUnknown211)
	{
		if (bActive)
		{
			if (m_bTronUnknown212 && !bVersion)
			{
				g_pInterfaceMgr->RemoveOverlay((eOverlayMask)14);
				g_pInterfaceMgr->CreateOverlay((eOverlayMask)11);
			}
			else if (bVersion && m_bTronUnknown212)
			{
				g_pInterfaceMgr->RemoveOverlay((eOverlayMask)11);
				g_pInterfaceMgr->CreateOverlay((eOverlayMask)14);
			}
		}
		else
		{
			HandleZoomChange(LTTRUE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtualA4
//
//	PURPOSE:	RETAIL: retail 0x100a7920 (slot 0xa4): an amount scaled by
//				the Fuzzy_signature subroutine. PORT: retail looks up a
//				table of a Tron manager (0x1026ec34) not yet in our tree
//
// ----------------------------------------------------------------------- //

int CTronPlayerMgr::TronUnknownVirtualA4(int nArg)
{
	char szName[16] = "Fuzzy_signature";
	if (!g_pSubroutineMgr->IsUtilitySubroutineActive(szName))
		return nArg;

	g_pSubroutineMgr->GetActiveSubroutineVersion(szName);
	return nArg;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::DoActivate
//
//	PURPOSE:	RETAIL: retail 0x100a7a20 (slot 0x4c): the target's
//				permissions and energy are checked first
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::DoActivate()
{
	if (g_pTronPlayerMgr->GetTronTargetMgr()->IsPermissionRequired())
	{
		uint8 nRequired = g_pTronPlayerMgr->GetTronTargetMgr()->GetPermissionRequired();
		uint8 nPermissions, nRequiredPermissions;
		g_pTronPlayerStats->GetPermissions(nPermissions, nRequiredPermissions);
		if ((uint8)((nPermissions | nRequired) - nPermissions))
		{
			g_pPickupMsgs->AddMessage(3430, (const char*)LTNULL);
		}
	}

	if (g_pTronPlayerMgr->GetTronTargetMgr()->IsEnergyRequired())
	{
		float fEnergyRequired = (float)g_pTronPlayerMgr->GetTronTargetMgr()->GetEnergyRequired();
		if (g_pTronPlayerStats->GetEnergy() < fEnergyRequired)
		{
			g_pPickupMsgs->AddMessage(IDS_INSUFFICIENT_ENERGY, (const char*)LTNULL);
			// PORT: Tron's PlaySoundLocal has a seventh argument (1.0f)
			g_pClientSoundMgr->PlaySoundLocal("insufficient_energy", SOUNDPRIORITY_MISC_MEDIUM, 0, 100, 1.0f, 0);
			return;
		}
	}

	CPlayerMgr::DoActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::ChangeWeapon
//
//	PURPOSE:	RETAIL: retail 0x100a7b00 (slot 0x44)
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::ChangeWeapon(uint8 nWeaponId, uint8 nAmmoId, int dwAmmo)
{
	if (g_pTronPlayerStats->CanChangeToWeapon(nWeaponId))
	{
		return m_pClientWeaponMgr->ChangeWeapon(nWeaponId, nAmmoId, dwAmmo, true);
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::ToggleHolster
//
//	PURPOSE:	RETAIL: retail 0x100a7b40 (slot 0x48)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::ToggleHolster(bool bPlayDeselect)
{
	if (!IsHolsterLocked())
	{
		CPlayerMgr::ToggleHolster(bPlayDeselect);
		return;
	}

	// RETAIL: 0x100a7b64
	g_pInterfaceMgr->TronUnknownVirtual88();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::IsHolsterLocked
//
//	PURPOSE:	RETAIL: retail 0x100a7b70: locked by +0x234, or while
//				the current weapon's slot 0xc8 is false
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::IsHolsterLocked()
{
	if (m_bTronUnknown234)
		return true;

	if (m_pClientWeaponMgr && !m_pClientWeaponMgr->CurrentWeaponTronUnknownVirtualC8())
		return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::Save / Load
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a7ba0, 0x100a7c00): three of its
//				flags before CPlayerMgr's
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	pMsg->Writebool(m_bTronUnknown210);
	pMsg->Writebool(m_bTronUnknown211);
	pMsg->Writebool(m_bTronUnknown234);

	CPlayerMgr::Save(pMsg, eSaveDataState);
}

void CTronPlayerMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	m_bTronUnknown210 = pMsg->Readbool();
	m_bTronUnknown211 = pMsg->Readbool();
	m_bTronUnknown234 = pMsg->Readbool();

	CPlayerMgr::Load(pMsg, eLoadDataState);

	m_bTronUnknown1D5 = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual70 / 7C / 74 / 78
//
//	PURPOSE:	RETAIL: retail 0x100a7c60, 0x100a7cd0, 0x100a7d50, 0x100a7e10
//				(slots 0x70, 0x7c, 0x74, 0x78): a light cycle's model, skin,
//				sounds and fx from the light cycle attributes
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual70(HOBJECT hObj, char* pszModel, char* pszSkin)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (!pCyclist)
		return false;

	// RETAIL: the colour is read before it is tested (docs/RETAIL_BUGS.md)
	LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(pCyclist->szCycleColor);
	LIGHTCYCLETYPE* pType = g_pLightCycleButeMgr->GetCycleType(pColor->nCycleType);
	if (!pColor || !pType)
		return false;

	sprintf(pszModel, pColor->szIntroFX);
	sprintf(pszSkin, pType->szIntroSound);
	return true;
}

bool CTronPlayerMgr::TronUnknownVirtual7C(HOBJECT hObj, char* psz1, char* psz2, char* psz3)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (pCyclist)
	{
		LIGHTCYCLETYPE* pType = g_pLightCycleButeMgr->GetCycleType(pCyclist->nCycleType);
		if (pType)
		{
			sprintf(psz1, pType->szCruiseSound);
			sprintf(psz2, pType->szTurnSound);
			if (psz3)
			{
				sprintf(psz3, pType->szAltCruiseSound);
			}

			return true;
		}

		return true;
	}

	return false;
}

bool CTronPlayerMgr::TronUnknownVirtual74(HOBJECT hObj, char* pszBuf, int nBufLen)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (pCyclist)
	{
		LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(pCyclist->szCycleColor);
		if (pColor)
		{
			int nNumFX = pColor->aDeathFX.size();
			if (nNumFX < 0)
			{
				// RETAIL: SAFE_STRCPY of a pointer: four bytes
				// (docs/RETAIL_BUGS.md)
				strncpy(pszBuf, "", sizeof(pszBuf) - 1);
				pszBuf[sizeof(pszBuf) - 1] = '\0';
				return true;
			}

			const char* pszFX = pColor->aDeathFX[GetRandom(0, nNumFX - 1)].c_str();
			LTStrCpy(pszBuf, pszFX, nBufLen);
			return true;
		}
	}

	return false;
}

bool CTronPlayerMgr::TronUnknownVirtual78(HOBJECT hObj, char* pszBuf)
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(hObj);
	if (pCyclist)
	{
		LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(pCyclist->szCycleColor);
		if (pColor)
		{
			sprintf(pszBuf, pColor->szRiderFX);
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual24
//
//	PURPOSE:	RETAIL: retail 0x100a7e60 (slot 0x24)
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual24(bool b1D7, bool b1D9)
{
	CPlayerMgr::TronUnknownVirtual24(b1D7, b1D9);

	if (g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LIGHTCYCLE || TronUnknownVirtual3C(true))
	{
		// PORT: retail calls Tron's interface manager's slot 0x98 (0, 0)
		// here
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownLightCycleSpectate
//
//	PURPOSE:	RETAIL: retail 0x100a7eb0 (called by the client light cycle
//				manager): in a light cycle game, the player watches the
//				next spectator object, else the next light cycle (third
//				person), else the first spectator object
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownLightCycleSpectate()
{
	if (IsMultiplayerGame() && g_pGameClientShell->GetGameType() == eGameTypeMultiplayerLightcycles)
	{
		HOBJECT hWatch = LTNULL;
		LIGHT_CYCLIST* pCyclist = LTNULL;

		if (m_hTronUnknownBC)
		{
			CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->GetNextSpectatorFX(m_hTronUnknownBC, false, false);
			if (pFX)
			{
				hWatch = pFX->GetServerObj();
			}
		}

		if (!hWatch)
		{
			HOBJECT hCurrent = LTNULL;
			if (g_pLightCycleMgr->FindCyclist(m_hTronUnknownBC))
			{
				hCurrent = m_hTronUnknownBC;
			}

			LIGHT_CYCLIST* pNext = g_pLightCycleMgr->GetNextCyclist(hCurrent, false);
			if (pNext && pNext->hObject != g_pMoveMgr->GetObject())
			{
				hWatch = pNext->hObject;
				pCyclist = pNext;
			}

			if (!hWatch)
			{
				CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->GetNextSpectatorFX(LTNULL, true, true);
				if (!pFX)
				{
					pFX = g_pGameClientShell->GetSFXMgr()->GetNextSpectatorFX(LTNULL, true, false);
				}

				if (pFX)
				{
					hWatch = pFX->GetServerObj();
				}
			}
		}

		m_hTronUnknownBC = hWatch;
		m_nTronUnknown230 = (uint32)pCyclist;
		m_bTronUnknownB8 = true;

		g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags, 0, FLAG_GRAVITY);

		g_pGameClientShell->m_bTronUnknown833A = false;
		g_pMoveMgr->GetVehicleMgr()->ResetPhysicsModel();

		if (pCyclist)
		{
			g_pInterfaceMgr->TronUnknownThirdPersonCamera(true);
		}
		else
		{
			g_pInterfaceMgr->TronUnknownFirstPersonCamera(true);
		}

		TronUnknownVirtual40();
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownLightCycleRemoved
//
//	PURPOSE:	RETAIL: retail 0x100a8000 (called by the client light cycle
//				manager): the watched cycle went
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownLightCycleRemoved(HOBJECT hObj)
{
	if (hObj && hObj == m_hTronUnknownBC)
	{
		m_hTronUnknownBC = LTNULL;
		m_bTronUnknownB8 = false;
		m_nTronUnknown230 = 0;
		g_pPlayerMgr->SetTronUnknown1D5(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual6C
//
//	PURPOSE:	RETAIL: retail 0x100a8040 (slot 0x6c): empty
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual6C(int nArg)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual8C
//
//	PURPOSE:	RETAIL: retail 0x100a8050 (slot 0x8c): a light cycle life
//				lost
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual8C()
{
	if (m_nTronUnknown236 > 0)
	{
		m_nTronUnknown236--;
	}

	// RETAIL: flashes Tron's light cycle lives HUD item (0x1026ee24)
	if (g_pLightCycleLives)
		g_pLightCycleLives->Flash();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual90
//
//	PURPOSE:	RETAIL: retail 0x100a8080 (slot 0x90): the player's light
//				cycle color, told to the server and kept in the profile.
//				PORT: Tron's model attributes (0x10044f40, 0x10044f20) and
//				its profile's color (+0x1d4) are not yet in our tree
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual90(const char* pszColor, bool bSend, bool bSave)
{
	if (!pszColor)
	{
		pszColor = m_szTronUnknown1B5;
	}

	if (_stricmp(pszColor, m_szTronUnknown1B5))
	{
		strcpy(m_szTronUnknown1B5, pszColor);
	}

	if (bSend)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(6);
		cMsg.WriteObject(LTNULL);
		cMsg.WriteString(pszColor);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	g_pLTClient->CPrint("Light Cycle set to Color:%s\n", pszColor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual40
//
//	PURPOSE:	RETAIL: retail 0x100a8270 (slot 0x40): the camera behind the
//				watched light cycle. PORT: not yet written
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtual40()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtualA8
//
//	PURPOSE:	RETAIL: retail 0x100a86b0 (slot 0xa8): Tron's HUD items for
//				walking off, the light cycle's on (or back). PORT: Tron's
//				HUD item globals (0x1026edb8..0x1026ee2c) are not yet in our
//				tree; the chat messages are
//
// ----------------------------------------------------------------------- //

void CTronPlayerMgr::TronUnknownVirtualA8(bool bHide)
{
	if (bHide)
	{
		if (!IsMultiplayerGame())
		{
			g_pChatMsgs->SetEnabled(true);
		}
	}
	else
	{
		if (!IsMultiplayerGame())
		{
			g_pChatMsgs->SetEnabled(false);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual94 / 98
//
//	PURPOSE:	RETAIL: retail 0x100a8810, 0x100a8860 (slots 0x94, 0x98):
//				whether the player's light cycle trail has two points or
//				more; all the cyclists go
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual94()
{
	LIGHT_CYCLIST* pCyclist = g_pLightCycleMgr->FindCyclist(g_pMoveMgr->GetObject());
	if (pCyclist && pCyclist->pCurTrail && pCyclist->pCurTrail->collTrailPoints.size() > 1)
	{
		return true;
	}

	return false;
}

void CTronPlayerMgr::TronUnknownVirtual98(int nArg)
{
	g_pLightCycleMgr->RemoveAllCyclists();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerMgr::TronUnknownVirtual3C
//
//	PURPOSE:	RETAIL: Tron's (retail 0x100a8870): +0xb8 set and, when asked,
//				+0x230 too. Not yet named
//
// ----------------------------------------------------------------------- //

bool CTronPlayerMgr::TronUnknownVirtual3C(bool b)
{
	return (m_bTronUnknownB8 && (!b || m_nTronUnknown230));
}

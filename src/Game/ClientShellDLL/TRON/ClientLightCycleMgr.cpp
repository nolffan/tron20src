// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightCycleMgr.cpp
//
// PURPOSE : The client's light cycle manager
//
// RETAIL: Tron 2.0's, retail-only; written from retail CShell.dll
// 0x100106c0..0x100160a0 (see ClientLightCycleMgr.h). The functions are in
// retail's order.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientLightCycleMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "InterfaceResMgr.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "ProfileMgr.h"
#include "ClientSoundMgr.h"
#include "ClientFXMgr.h"
#include "HUDMgr.h"
#include "HUDMessageQueue.h"
#include "HUDSubtitles.h"
#include "ClientMultiplayerMgr.h"
#include "MsgIDs.h"
#include "ScreenSubroutines.h"	// RETAIL: its constants' initializers (0x10010650..0x100106b2)
#include "TronHUDMgr.h"	// RETAIL: g_pLightCycleRoster
#include "TronPlayerMgr.h"
#include "VehicleMgr.h"
#include "LightCycleFX.h"

extern CGameClientShell* g_pGameClientShell;
extern bool GroundFilterFn(HOBJECT hObj, void *pUserData);	// BodyFX.cpp

// RETAIL: 0x10236d9c, the canvas the trails are drawn on (the name is ours)
static HOBJECT s_hTrailCanvas = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsLocalCyclist
//
//	PURPOSE:	Is it the local player's (server) object. RETAIL: 0x100106c0
//				(the name is ours)
//
// ----------------------------------------------------------------------- //

static bool IsLocalCyclist(HOBJECT hObj)
{
	return (hObj == g_pMoveMgr->GetServerObject());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayCycleDialogue
//
//	PURPOSE:	Plays a light cycle dialogue line with its subtitle.
//				RETAIL: 0x100106e0 (the name is ours). PORT: Tron's
//				CHUDSubtitles::Show takes the sound; NOLF2's stands in.
//
// ----------------------------------------------------------------------- //

static void PlayCycleDialogue(int nStringId)
{
	char szSound[48];
	g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nStringId, szSound, sizeof(szSound));

	HLTSOUND hSnd = g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_PLAYER_HIGH, 0, 100, 1.0f, 0);

	float fDuration = -1.0f;
	g_pLTClient->SoundMgr()->GetSoundDuration(hSnd, fDuration);

	g_pSubtitles->Show(nStringId, LTVector(0.0f, 0.0f, 0.0f), hSnd, 0.0f, fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightCycleCanvasFn
//
//	PURPOSE:	The trail canvas' draw function. RETAIL: 0x10010790 (the
//				name is ours)
//
// ----------------------------------------------------------------------- //

static void LightCycleCanvasFn(HLOCALOBJ hObj, void *pUser)
{
	if (hObj && pUser)
	{
		((CClientLightCycleMgr*)pUser)->DrawTrails();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::CClientLightCycleMgr
//
//	PURPOSE:	RETAIL: 0x100107b0
//
// ----------------------------------------------------------------------- //

CClientLightCycleMgr::CClientLightCycleMgr()
{
	m_nTrailPolys = 0;
	m_nTrailBuffers = 0;
	m_nStartPolys = 0;
	m_nTrailBuffer = 0;
	m_hTailTex = LTNULL;
	m_hTailStartTex = LTNULL;
	m_fLastPredictTime = 0.0f;
	m_fPredictInterval = 1.0f / 15.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::~CClientLightCycleMgr
//
//	PURPOSE:	RETAIL: 0x10010a90
//
// ----------------------------------------------------------------------- //

CClientLightCycleMgr::~CClientLightCycleMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::Init
//
//	PURPOSE:	RETAIL: 0x10010bc0
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::Init()
{
	if (!CLightCycleMgr::Init())
	{
		return false;
	}

	g_pLightCycleMgr = this;

	m_hTailTex = g_pInterfaceResMgr->GetTexture("props\\skins\\Tail.dtx");
	m_hTailStartTex = g_pInterfaceResMgr->GetTexture("props\\skins\\TailStart.dtx");

	// Setup the trail canvas...
	if (!s_hTrailCanvas)
	{
		ObjectCreateStruct createStruct;

		createStruct.m_ObjectType = OT_CANVAS;
		createStruct.m_Flags = FLAG_VISIBLE;

		s_hTrailCanvas = g_pLTClient->CreateObject(&createStruct);
		if (!s_hTrailCanvas)
		{
			return false;
		}

		g_pLTClient->SetCanvasFn(s_hTrailCanvas, LightCycleCanvasFn, this);
		g_pLTClient->SetCanvasRadius(s_hTrailCanvas, 100000.0f);
		g_pCommonLT->SetObjectFlags(s_hTrailCanvas, OFT_Client, CF_SOLIDCANVAS, CF_SOLIDCANVAS);
	}

	AllocTrailBuffer();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::Term
//
//	PURPOSE:	RETAIL: 0x10010f50
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::Term()
{
	CLightCycleMgr::Term();

	for (int i = 0; i < m_nTrailBuffers; i++)
	{
		debug_deletea(m_apTrailPolys[i]);
		m_apTrailPolys[i] = LTNULL;
	}

	m_nTrailBuffers = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::DerezCyclist
//
//	PURPOSE:	Derezzes the crashed cyclist. RETAIL: 0x10010fb0. PORT: left
//				out are CTronPlayerMgr's 0x100a8000 and
//				the local vehicle manager's two calls (0x1011ed70,
//				0x10122140) for the local cycle, the CLightCycleFX's derez
//				(its slot 1) for the others, and the multiplayer score
//				call (0x10016ad0 on 0x10236df0): none of them is in our
//				build.
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::DerezCyclist(LIGHT_CYCLIST* pCyclist, LIGHT_CYCLIST* pOther, bool bNotify)
{
	if (pCyclist->fStartDerezTime > 0.0f)
	{
		return false;
	}

	pCyclist->fStartDerezTime = g_pLTClient->GetTime();
	pCyclist->bUpdating = false;

	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for (iter = pCyclist->collTrails.begin(); iter != pCyclist->collTrails.end(); iter++)
	{
		if ((*iter)->fDerezStartTime == 0.0f)
		{
			(*iter)->fDerezStartTime = g_pLTClient->GetTime();
		}
	}

	pCyclist->pCurTrail = LTNULL;
	pCyclist->fTrailCheckTime = 0.0f;
	pCyclist->m_vTronUnknownE4.Init(0.0f, 0.0f, 0.0f);

	// RETAIL: the light cycle roster flashes it out (g_pLightCycleRoster,
	// 0x1026ee18)
	if (g_pLightCycleRoster)
		g_pLightCycleRoster->DerezCyclist(pCyclist);

	HOBJECT hObj = pCyclist->hObject;

	if (pCyclist->hObject == g_pMoveMgr->GetObject())
	{
		hObj = g_pMoveMgr->GetServerObject();

		if (GetGameType() == eGameTypeSingleLightCycleCircuit)
		{
			g_pPlayerMgr->TronUnknownVirtual8C();
		}

		ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_DerezzLocalCyclist),
			GetCyclistName(pCyclist->hObject).c_str()));
	}
	else
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(hObj, &vPos);

		// The player manager names the cycle's derez FX
		char szFX[512];
		g_pPlayerMgr->TronUnknownVirtual74(hObj, szFX, sizeof(szFX));

		CLIENTFX_CREATESTRUCT fxInit;
		LTStrCpy(fxInit.m_sName, szFX, sizeof(fxInit.m_sName));
		fxInit.m_dwFlags = FXFLAG_NOSMOOTHSHUTDOWN;
		fxInit.m_hParent = pCyclist->hObject;
		g_pLTClient->GetObjectPos(pCyclist->hObject, &fxInit.m_vPos);
		g_pClientFXMgr->CreateClientFX(LTNULL, fxInit, LTTRUE);

		ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_DerezzGenericCyclist),
			GetCyclistName(pCyclist->hObject).c_str()));
	}

	HOBJECT hOther = LTNULL;
	if (pOther)
	{
		hOther = pOther->hObject;
	}

	if (bNotify)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_DEREZ_CYCLIST);
		cMsg.WriteObject(hObj);
		cMsg.WriteObject(hOther);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::Update
//
//	PURPOSE:	Moves the cycles and their trails; the local cycle sends
//				its position. RETAIL: 0x10011410. PORT: left out are the
//				turbo FX shutdowns when the turbo runs out (the vehicle
//				manager's FX links, +0x2140/+0x2144, or the cycle's
//				CLightCycleFX's, +0x50/+0x54) and the remote cycles'
//				CLightCycleFX move: neither is in our build.
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::Update()
{
	if (!m_bUpdating)
	{
		return;
	}

	UpdateDerez();

	// RETAIL: Tron's light cycle HUD update flag
	g_pHUDMgr->QueueUpdate(0x40000000);

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for (iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		LIGHT_CYCLIST* pCyclist = *iter;

		bool bLocal = (pCyclist->hObject == g_pMoveMgr->GetObject());
		LTVector vForward = pCyclist->vForward;

		if (pCyclist->bTurbo || pCyclist->bTurboCursed)
		{
			if (pCyclist->fPowerupDuration + pCyclist->fPowerupStartTime <= g_pLTClient->TronUnknownVirtual44())	// TRON: the game clock (iltclient.h)
			{
				pCyclist->bTurbo = false;
				pCyclist->bTurboCursed = false;
				pCyclist->bTurboCurse = false;
			}
		}

		if (pCyclist->bTrailSpike)
		{
			UpdateTrailSpike(pCyclist);
		}

		if (!pCyclist->bUpdating)
		{
			continue;
		}

		LTVector* pvForward = LTNULL;

		if (bLocal)
		{
			g_pLTClient->GetObjectPos(pCyclist->hObject, &pCyclist->vLastPos);

			LIGHT_CYCLE_TRAIL* pTrail = pCyclist->pCurTrail;
			LTVector vDir;
			if (pTrail && pTrail->collTrailPoints.size() > 1)
			{
				LIGHT_CYCLE_TRAIL_POINT* pLast = &pTrail->collTrailPoints[pTrail->collTrailPoints.size() - 1];
				vDir = pCyclist->vForward;

				// Head along the axis from the trail's last point
				if (pLast->vPoint.x < pCyclist->vLastPos.x)
				{
					vDir.x = 1.0f;
					vDir.z = 0.0f;
				}
				else if (pLast->vPoint.x > pCyclist->vLastPos.x)
				{
					vDir.x = -1.0f;
					vDir.z = 0.0f;
				}
				else if (pLast->vPoint.z < pCyclist->vLastPos.z)
				{
					vDir.x = 0.0f;
					vDir.z = 1.0f;
				}
				else if (pLast->vPoint.z > pCyclist->vLastPos.z)
				{
					vDir.x = 0.0f;
					vDir.z = -1.0f;
				}

				pvForward = &vDir;
			}
		}
		else if (GetGameType() == eGameTypeMultiplayerLightcycles)
		{
			float fSinceCheck = g_pLTClient->GetTime() - pCyclist->fTrailCheckTime;
			if (pCyclist->fTrailCheckTime > 0.0f && (fSinceCheck > 0.0f || !pCyclist->m_bTronUnknown130))
			{
				// Predict the remote cycle from its last known position
				HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LCHostPredict");
				float fHostPredict = g_pLTClient->GetVarValueFloat(hVar);
				if (!hVar)
				{
					fHostPredict = 2.0f;
				}

				hVar = g_pLTClient->GetConsoleVar("LCClientPredict");
				float fClientPredict = g_pLTClient->GetVarValueFloat(hVar);
				if (!hVar)
				{
					fClientPredict = 10.0f;
				}

				float fPredict;
				if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer())
				{
					fPredict = fClientPredict;
				}
				else
				{
					fPredict = fHostPredict;
				}

				if (fPredict != 0.0f)
				{
					float fInterp = 0.2f;
					hVar = g_pLTClient->GetConsoleVar("LCInterpTime");
					if (hVar)
					{
						fInterp = g_pLTClient->GetVarValueFloat(hVar);
					}

					LTVector vVel = pCyclist->vForward * pCyclist->fSpeed;
					LTVector vTarget = vVel * (g_pLTClient->GetFrameTime() * fPredict + fSinceCheck) + pCyclist->m_vTronUnknownE4;

					pCyclist->vLastPos.x += (vTarget.x - pCyclist->vLastPos.x) * fInterp;
					pCyclist->vLastPos.y += (vTarget.y - pCyclist->vLastPos.y) * fInterp;
					pCyclist->vLastPos.z += (vTarget.z - pCyclist->vLastPos.z) * fInterp;
				}

				pvForward = &pCyclist->vForward;
			}
		}

		pCyclist->m_bTronUnknown130 = false;

		if (bLocal || pCyclist->fTrailCheckTime != 0.0f)
		{
			UpdateLightCycleTrail(pCyclist, false, pvForward, bLocal);

			if (bLocal)
			{
				float fTime = g_pLTClient->GetTime();
				float fSinceSend = fTime - m_fLastPredictTime;

				if (GetGameType() == eGameTypeMultiplayerLightcycles && fSinceSend <= m_fPredictInterval &&
					vForward.x == pCyclist->vForward.x && vForward.y == pCyclist->vForward.y &&
					vForward.z == pCyclist->vForward.z)
				{
					continue;
				}

				// Tell the server where we are
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
				cMsg.Writeuint8(LCI_POSITION);
				cMsg.WriteObject(g_pMoveMgr->GetServerObject());
				cMsg.Writefloat(pCyclist->vLastPos.x);
				cMsg.Writefloat(pCyclist->vLastPos.z);
				cMsg.Writeint8((int8)pCyclist->vForward.x);
				cMsg.Writeint8((int8)pCyclist->vForward.z);
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

				m_fLastPredictTime = fTime;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::DrawTrails
//
//	PURPOSE:	RETAIL: 0x10011ac0
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::DrawTrails()
{
	if (!m_collCyclists.size())
	{
		return;
	}

	m_nTrailPolys = 0;
	m_nStartPolys = 0;
	m_nTrailBuffer = 0;

	std::vector<LIGHT_CYCLIST*>::iterator iter;
	for (iter = m_collCyclists.begin(); iter != m_collCyclists.end(); iter++)
	{
		if (*iter)
		{
			BuildCyclistPolys(*iter);
		}
	}

	if (m_nTrailBuffer <= 0 && m_nTrailPolys <= 0)
	{
		return;
	}

	g_pDrawPrim->SetCamera(g_pPlayerMgr->GetCamera());
	g_pDrawPrim->SetFogEnable(GetConsoleInt("FogEnable", 1) != 0);
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_WORLD);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_ZRW);
	g_pDrawPrim->SetClipMode(DRAWPRIM_FULLCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
	g_pDrawPrim->SetTexture(m_hTailTex);

	int nCount = LCM_TRAIL_POLYS_PER_BUFFER;
	for (int i = 0; i <= m_nTrailBuffer; i++)
	{
		if (i == m_nTrailBuffer)
		{
			nCount = m_nTrailPolys;
		}

		g_pDrawPrim->SetTexture(m_hTailTex);
		g_pDrawPrim->DrawPrim(m_apTrailPolys[i], nCount);
	}

	if (m_nStartPolys > 0)
	{
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_ALPHATEST_GREATER);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		g_pDrawPrim->SetTexture(m_hTailStartTex);
		g_pDrawPrim->DrawPrim(m_aStartPolys, m_nStartPolys);
	}

	g_pDrawPrim->SetFogEnable(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::OnMessage
//
//	PURPOSE:	A MID_LIGHT_CYCLE_INFO message from the server. RETAIL:
//				0x10011c80 (the name is ours). PORT: three kinds of retail
//				call code our build has not:
//				- Tron's CLightCycleFX (the special FX of the other
//				  players' cycles; RTTI .?AVCLightCycleFX@@, vtable
//				  0x101eb140), found by the SFX manager's 0x10108650:
//				  LCI_POSITION turns its object to the new forward, and
//				  LCI_TURBO hands its engine sound (+0x44) to
//				  UpdateCycleEffects;
//				- CTronInterfaceMgr's turn arrows (0x1009e850: kinds 4
//				  and 5 show the left or the right one).
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::OnMessage(ILTMessage_Read* pMsg)
{
	uint8 nType = pMsg->Readuint8();

	switch (nType)
	{
		case LCI_POSITION:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				return true;
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				float fX = pMsg->Readfloat();
				float fZ = pMsg->Readfloat();
				int8 nForwardX = pMsg->Readint8();
				int8 nForwardZ = pMsg->Readint8();

				LTVector vPos;
				g_pLTClient->GetObjectPos(pCyclist->hObject, &vPos);

				LTVector vForward((float)nForwardX, 0.0f, (float)nForwardZ);

				// The players' cycles count the distance they moved
				if (pCyclist->bIsPlayer && pCyclist->pCurTrail)
				{
					pCyclist->pCurTrail->fLength += (float)fabs(pCyclist->m_vTronUnknownE4.z - fZ) +
						(float)fabs(pCyclist->m_vTronUnknownE4.x - fX);
				}

				pCyclist->m_vTronUnknownE4 = LTVector(fX, vPos.y, fZ);

				// Still going the same way: just the position
				if (pCyclist->fTrailCheckTime != 0.0f && pCyclist->vForward == vForward)
				{
					if (!pCyclist->bIsPlayer)
					{
						pCyclist->vLastPos = LTVector(fX, vPos.y, fZ);
					}

					pCyclist->fTrailCheckTime = g_pLTClient->GetTime();
					return true;
				}

				pCyclist->vLastPos = LTVector(fX, vPos.y, fZ);
				pCyclist->m_bTronUnknown130 = true;

				// PORT: retail turns the cycle's CLightCycleFX object to
				// vForward here (LTRotation(vForward, LTVector(0, 1, 0)),
				// SetObjectRotation): see above

				pCyclist->fTrailCheckTime = g_pLTClient->GetTime();
				UpdateLightCycleTrail(pCyclist, false, &vForward, false);
			}

			return true;
		}

		case LCI_TRONUNKNOWN_4:
		{
			// PORT: retail shows the left turn arrow
			// (g_pInterfaceMgr's 0x1009e850, 0): see above
			return true;
		}

		case LCI_TRONUNKNOWN_5:
		{
			// PORT: retail shows the right turn arrow
			// (g_pInterfaceMgr's 0x1009e850, 1): see above
			return true;
		}

		case LCI_ADD_CYCLIST:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (!hObj)
			{
				return true;
			}

			char szCycleColor[32];
			pMsg->ReadString(szCycleColor, sizeof(szCycleColor));
			bool bUpdate = pMsg->Readbool();
			float fForwardX = (float)pMsg->Readint8();
			float fForwardZ = (float)pMsg->Readint8();
			bool bRoster = pMsg->Readbool();
			uint32 nClientID = pMsg->Readuint32();
			uint32 nNameId = pMsg->Readuint32();

			LTVector vDims;
			LTVector* pvDims;
			if (pMsg->Readbool())
			{
				vDims = pMsg->ReadLTVector();
				pvDims = &vDims;
			}
			else
			{
				pvDims = LTNULL;
			}

			bool bLocal = IsLocalCyclist(hObj);
			if (bLocal)
			{
				hObj = g_pMoveMgr->GetObject();
				g_pPlayerMgr->TronUnknownVirtual90(szCycleColor, false, false);

				if (FindCyclist(hObj))
				{
					g_pMoveMgr->GetVehicleMgr()->ResetLightCyclePhysics();
				}

				if (m_hDialogSnd)
				{
					g_pLTClient->SoundMgr()->KillSound(m_hDialogSnd);
					m_hDialogSnd = LTNULL;
				}

				m_fLastPredictTime = 0.0f;
			}

			LIGHT_CYCLIST* pCyclist = AddCyclist(hObj, szCycleColor, bUpdate, pvDims, 0, true);
			pCyclist->nClientID = nClientID;
			pCyclist->nNameId = nNameId;
			pCyclist->vForward = LTVector(fForwardX, 0.0f, fForwardZ);
			UpdateCyclistDims(pCyclist, vDims, true);

			if (bRoster && g_pLightCycleRoster)
			{
				g_pLightCycleRoster->AddCyclist(pCyclist);
			}

			g_pHUDMgr->QueueUpdate(kHUDAll);

			if (bLocal)
			{
				// The player faces the cycle's way
				float fYaw;
				if (pCyclist->vForward.x == 1.0f)
				{
					fYaw = MATH_HALFPI;
				}
				else if (pCyclist->vForward.x == -1.0f)
				{
					fYaw = MATH_PI + MATH_HALFPI;
				}
				else if (pCyclist->vForward.z == 1.0f)
				{
					fYaw = 0.0f;
				}
				else if (pCyclist->vForward.z == -1.0f)
				{
					fYaw = MATH_PI;
				}
				else
				{
					fYaw = 0.0f;
				}

				g_pPlayerMgr->SetPlayerYaw(fYaw);

				LTVector vPYR;
				g_pPlayerMgr->GetPlayerPitchYawRoll(vPYR);
				g_pPlayerMgr->SetPlayerPitch(vPYR.x);
				g_pPlayerMgr->SetPlayerYaw(vPYR.y);
				g_pPlayerMgr->SetPlayerRoll(vPYR.z);
				g_pPlayerMgr->SetPitch(vPYR.x);
				g_pPlayerMgr->SetYaw(vPYR.y);
				g_pPlayerMgr->SetRoll(vPYR.z);
				g_pMoveMgr->GetVehicleMgr()->SetTronUnknown2128(vPYR);

				LTRotation rRot(vPYR.x, vPYR.y, vPYR.z);
				g_pLTClient->SetObjectRotation(g_pMoveMgr->GetObject(), &rRot);
			}

			return true;
		}

		case LCI_REMOVE_CYCLIST:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
				g_pMoveMgr->GetVehicleMgr()->ShutdownLightCycleFX();
			}
			else
			{
				((CTronPlayerMgr*)g_pPlayerMgr)->TronUnknownLightCycleRemoved(hObj);
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist && g_pLightCycleRoster)
			{
				g_pLightCycleRoster->DerezCyclist(pCyclist);
			}

			RemoveCyclist(hObj);
		}
		break;

		case LCI_DEREZ_CYCLIST:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				DerezCyclist(pCyclist, FindCyclist(pMsg->ReadObject()), false);
			}

			g_pHUDMgr->QueueUpdate(kHUDAll);
			return true;
		}

		case LCI_TURBO:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				float fAccelRate = pMsg->Readfloat();
				float fSpeed = pMsg->Readfloat();
				bool bTurned = pMsg->Readbool();
				float fAccel = pMsg->Readfloat();
				float fWallAccel = pMsg->Readfloat();
				uint32 nFlags = pMsg->Readuint32();

				pCyclist->fAccelRate = fAccelRate;
				pCyclist->fSpeed = fSpeed;

				// PORT: for the others' cycles retail hands the cycle's
				// CLightCycleFX and its engine sound to UpdateCycleEffects
				// (bTurned, fAccel, fWallAccel, nFlags): see above
			}

			return true;
		}

		case LCI_SET_POWERUP:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				SetCyclistPowerup(pCyclist, pMsg->Readuint8());
			}

			return true;
		}

		case LCI_ACTIVATE_POWERUP:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
				((CTronPlayerMgr*)g_pPlayerMgr)->SetTronUnknown235(false);
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				LTVector vPos = pMsg->ReadLTVector();
				ActivatePowerup(pCyclist, &vPos);
			}

			return true;
		}

		case LCI_TRONUNKNOWN_13:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist && pCyclist->bShield)
			{
				DeactivateShield(pCyclist, false);
			}
		}
		break;

		case LCI_TRIM_TRAIL:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				int nTrail = pMsg->Readuint32();
				int nPoint = pMsg->Readuint32();
				TrimTrail(pCyclist, nTrail, nPoint, false);
			}

			return true;
		}

		case LCI_UPDATING_STARTED:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
			if (pCyclist)
			{
				pCyclist->bUpdating = (pMsg->Readuint8() != 0);
			}

			return true;
		}

		case LCI_TRONUNKNOWN_18:
		{
			g_pPlayerMgr->TronUnknownVirtual24(false, true);
			return true;
		}

		case LCI_TRONUNKNOWN_19:
		{
			g_pPlayerMgr->TronUnknownVirtual24(true, false);
			return true;
		}

		case LCI_TRONUNKNOWN_20:
		{
			g_pPlayerMgr->SetTronUnknown1DC(pMsg->Readfloat());
			return true;
		}

		case LCI_TRONUNKNOWN_21:
		{
			g_pPlayerMgr->TronUnknownVirtual84(pMsg->Readbool());
			return true;
		}

		case LCI_DIALOG:
		{
			LTVector vPos = pMsg->ReadLTVector();
			char szSound[256];
			pMsg->ReadString(szSound, sizeof(szSound) - 1);
			PlayDialogSound(vPos, szSound);
			return true;
		}

		case LCI_COLLISION:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);

			LightCycleCollisionInfo info;
			info.nSegment = pMsg->Readuint32();
			info.nTrail = pMsg->Readuint32();

			HOBJECT hOther = pMsg->ReadObject();
			if (IsLocalCyclist(hOther))
			{
				hOther = g_pMoveMgr->GetObject();
			}

			LIGHT_CYCLIST* pOther = FindCyclist(hOther);
			if (pOther)
			{
				info.pCyclist = pOther;
				info.vCollisionPoint.x = pMsg->Readfloat();
				info.vCollisionPoint.y = pOther->vLastPos.y;
				info.vCollisionPoint.z = pMsg->Readfloat();
				bool bNotify = pMsg->Readbool();
				bool bCurrentTrail = pMsg->Readbool();
				HandleShieldCollision(pCyclist, &info, bNotify, bCurrentTrail);
			}

			return true;
		}

		case LCI_TRONUNKNOWN_26:
		{
			DerezOldTrails();
			return true;
		}

		case LCI_TRAIL_LENGTH:
		{
			m_fTrailLength = pMsg->Readfloat();
			return true;
		}

		case LCI_GAME_SPEED:
		{
			m_fGameSpeedScalar = pMsg->Readfloat();
			return true;
		}

		case LCI_TRONUNKNOWN_33:
		{
			m_nTronUnknown1338 = pMsg->Readint8();
		}
		break;

		case LCI_POWERUP_GRABBED:
		{
			HOBJECT hObj = pMsg->ReadObject();
			HOBJECT hVictim = pMsg->ReadObject();

			if (IsLocalCyclist(hVictim))
			{
				PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_AnnouncePowerupGrab));
				ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateGrabFromSelf),
					GetCyclistName(hObj).c_str()));
			}
			else if (IsLocalCyclist(hObj))
			{
				PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_AnnouncePowerupGrab));
				ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateGrabFromCyclist),
					GetCyclistName(hVictim).c_str()));
			}
		}
		break;

		case LCI_POWERUP_TAKEN:
		{
			HOBJECT hObj = pMsg->ReadObject();
			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
			}

			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_AnnouncePowerupGrab));
			ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateGrabFromFloor),
				GetCyclistName(hObj).c_str()));
		}
		break;

		case LCI_TRONUNKNOWN_7:
		{
			HOBJECT hObj = pMsg->ReadObject();
			char szCycleColor[32];
			pMsg->ReadString(szCycleColor, sizeof(szCycleColor));

			if (IsLocalCyclist(hObj))
			{
				hObj = g_pMoveMgr->GetObject();
				g_pPlayerMgr->TronUnknownVirtual90(szCycleColor, false, false);

				LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
				if (!pCyclist)
				{
					return true;
				}

				LIGHTCYCLECOLOR* pColor = g_pLightCycleButeMgr->GetCycleColor(szCycleColor);
				pCyclist->nRed = pColor->nRed;
				pCyclist->nGreen = pColor->nGreen;
				pCyclist->nBlue = pColor->nBlue;
				return true;
			}

			return true;
		}

		case LCI_CYCLISTS:
		{
			ReadCyclists(pMsg);
			return true;
		}

		case LCI_GRAB_FAILED:
		{
			if (IsLocalCyclist(pMsg->ReadObject()))
			{
				((CTronPlayerMgr*)g_pPlayerMgr)->SetTronUnknown235(false);

				LIGHT_CYCLIST* pCyclist = FindCyclist(g_pMoveMgr->GetObject());
				if (pCyclist && pCyclist->nPowerup == kLCPowerup_Grab)
				{
					char szMsg[128];
					FormatString(0x2204, szMsg, sizeof(szMsg));
					g_pChatMsgs->AddMessage(szMsg, kMsgDefault);
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
				}
			}
		}
		break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::ShowMessage
//
//	PURPOSE:	RETAIL: 0x10012db0
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::ShowMessage(const char* szMsg)
{
	g_pChatMsgs->AddMessage(szMsg, kMsgDefault);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::GetCyclistName
//
//	PURPOSE:	RETAIL: 0x10012dd0. PORT: retail copies the profile's name
//				(a caseless string at CProfileMgr +0x200) by its range
//
// ----------------------------------------------------------------------- //

std::string CClientLightCycleMgr::GetCyclistName(HOBJECT hObj)
{
	if (IsLocalCyclist(hObj))
	{
		hObj = g_pMoveMgr->GetObject();
	}

	LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
	if (!pCyclist)
	{
		return std::string("");
	}

	// RETAIL: +0xd0 is set for the players' cycles
	if (pCyclist->bIsPlayer)
	{
		if (IsMultiplayerGame())
		{
			const char* szName = g_pInterfaceMgr->GetClientInfoMgr()->GetPlayerName(pCyclist->nClientID);
			if (!szName)
			{
				return std::string("");
			}

			return std::string(szName);
		}

		if (GetGameType() != eGameTypeSingleLightCycleCircuit && GetGameType() != eGameTypeSingleLightCycleCustom)
		{
			char szName[80] = "";
			FormatString(0x15ff, szName, sizeof(szName));
			return std::string(szName);
		}

		if (g_pProfileMgr)
		{
			return std::string(g_pProfileMgr->GetCurrentProfileName());
		}

		return std::string("Unknown");
	}

	char szName[80] = "";
	FormatString(pCyclist->nNameId, szName, sizeof(szName));
	return std::string(szName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsCycleDialogOn
//
//	PURPOSE:	Does the profile want the riders' lines ("LCDialog").
//				RETAIL: 0x100130e0 (the name is ours)
//
// ----------------------------------------------------------------------- //

static bool IsCycleDialogOn()
{
	if (!g_pProfileMgr)
	{
		return true;
	}

	return (g_pProfileMgr->GetCurrentProfile()->m_bLCDialog != LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::PlayDialogSound
//
//	PURPOSE:	RETAIL: 0x10013100
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::PlayDialogSound(LTVector vPos, const char* szSound)
{
	if (!IsCycleDialogOn())
	{
		return;
	}

	// RETAIL: function statics (0x10236d78..0x10236d84); the names are
	// ours. The sound debug flag is read but not used.
	static uint32 s_dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_ONCE;
	static float s_fRadius = g_pLightCycleButeMgr->GetDialogSoundRadius();
	static bool s_bSoundDebug = g_pLightCycleButeMgr->GetEnableSoundDebugInfo();

	m_hDialogSnd = g_pClientSoundMgr->PlaySoundFromPos(vPos, szSound, s_fRadius, SOUNDPRIORITY_PLAYER_LOW,
		s_dwFlags, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f, 2, 1.0f);

	int nStringId = g_pClientSoundMgr->GetSoundIdFromFilename(szSound);

	float fDuration = -1.0f;
	g_pLTClient->SoundMgr()->GetSoundDuration(m_hDialogSnd, fDuration);

	g_pSubtitles->Show(nStringId, vPos, m_hDialogSnd, s_fRadius, fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::BuildCyclistPolys
//
//	PURPOSE:	RETAIL: 0x10013210
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::BuildCyclistPolys(LIGHT_CYCLIST* pCyclist)
{
	std::vector<LIGHT_CYCLE_TRAIL*>::iterator iter;
	for (iter = pCyclist->collTrails.begin(); iter != pCyclist->collTrails.end(); iter++)
	{
		LIGHT_CYCLE_TRAIL* pTrail = *iter;
		if (!pTrail)
		{
			continue;
		}

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(pCyclist->hObject, &vDims);

		float fOffset = 15.0f;
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LightCycleWallOffset");
		if (hVar)
		{
			fOffset = g_pLTClient->GetVarValueFloat(hVar);
		}

		float fHeight = vDims.y + fOffset;

		bool bAlive = true;
		if (pCyclist->fStartDerezTime > 0.0f)
		{
			bAlive = false;
		}

		BuildTrailPolys(pTrail, pCyclist->nRed, pCyclist->nGreen, pCyclist->nBlue, fHeight, vDims.y,
			(pTrail == pCyclist->pCurTrail), bAlive);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::BuildTrailPolys
//
//	PURPOSE:	Adds the trail's wall quads, a notch quad every so often and,
//				for the current trail, the head quad. RETAIL: 0x100132f0
//
// ----------------------------------------------------------------------- //

#define LCM_HEAD_LENGTH		52.0f	// RETAIL: 0x101e5798 (the names are ours)
#define LCM_HEAD_FADE		128.0f	// RETAIL: 0x101e5794
#define LCM_NOTCH_LENGTH	8.0f	// RETAIL: 0x101e578c

void CClientLightCycleMgr::BuildTrailPolys(LIGHT_CYCLE_TRAIL* pTrail, uint8 nRed, uint8 nGreen, uint8 nBlue,
	float fHeight, float fBase, bool bCurrentTrail, bool bAlive)
{
	float fCarry = 0.0f;

	int nPoints = pTrail->collTrailPoints.size();
	if (nPoints < 2)
	{
		return;
	}

	bool bFirst = true;
	float fDerez = 0.0f;
	if (pTrail->fDerezStartTime > 0.0f)
	{
		float fDelay;
		if (bAlive)
		{
			fDelay = m_ButeMgr.GetTrailDerezDelay();
		}
		else
		{
			fDelay = 0.0f;
		}

		float fDerezTime = m_ButeMgr.GetTrailDerezTime();
		fDerez = (g_pLTClient->GetTime() - pTrail->fDerezStartTime - fDelay) / fDerezTime;
		if (fDerez >= 1.0f)
		{
			return;
		}
	}

	LIGHT_CYCLE_TRAIL_POINT vHeadStart, vHeadEnd;

	for (int i = 0; i < nPoints - 1; i++)
	{
		LIGHT_CYCLE_TRAIL_POINT* pCur = &pTrail->collTrailPoints[i];
		LIGHT_CYCLE_TRAIL_POINT* pNext = &pTrail->collTrailPoints[i + 1];

		if (bCurrentTrail && pNext == &pTrail->collTrailPoints[nPoints - 1])
		{
			// The last segment ends in the head quad
			if ((float)fabs(pCur->GetX() - pNext->GetX()) > (float)fabs(pCur->GetZ() - pNext->GetZ()))
			{
				vHeadStart.SetZ(pCur->GetZ());
				vHeadEnd.SetZ(pNext->GetZ());

				if (pCur->GetX() < pNext->GetX())
				{
					vHeadStart.SetX(LTMAX(pCur->GetX(), pNext->GetX() - LCM_HEAD_LENGTH - LCM_HEAD_FADE));
					vHeadEnd.SetX(LTMAX(pCur->GetX(), pNext->GetX() - LCM_HEAD_LENGTH));
				}
				else
				{
					vHeadStart.SetX(LTMIN(pCur->GetX(), pNext->GetX() + (LCM_HEAD_LENGTH + LCM_HEAD_FADE)));
					vHeadEnd.SetX(LTMIN(pCur->GetX(), pNext->GetX() + LCM_HEAD_LENGTH));
				}
			}
			else
			{
				vHeadStart.SetX(pCur->GetX());
				vHeadEnd.SetX(pNext->GetX());

				if (pCur->GetZ() < pNext->GetZ())
				{
					vHeadStart.SetZ(LTMAX(pCur->GetZ(), pNext->GetZ() - LCM_HEAD_LENGTH - LCM_HEAD_FADE));
					vHeadEnd.SetZ(LTMAX(pCur->GetZ(), pNext->GetZ() - LCM_HEAD_LENGTH));
				}
				else
				{
					vHeadStart.SetZ(LTMIN(pCur->GetZ(), pNext->GetZ() + (LCM_HEAD_LENGTH + LCM_HEAD_FADE)));
					vHeadEnd.SetZ(LTMIN(pCur->GetZ(), pNext->GetZ() + LCM_HEAD_LENGTH));
				}
			}

			vHeadStart.SetY(pCur->GetY());
			vHeadEnd.SetY(pNext->GetY());

			pNext = &vHeadStart;
		}

		float fDX = (float)fabs(pCur->GetX() - pNext->GetX());
		float fDZ = (float)fabs(pCur->GetZ() - pNext->GetZ());
		float fLength = (fDX > fDZ) ? fDX : fDZ;

		float fNotch = (bFirst ? pTrail->m_fTronUnknown14 : m_fTronUnknown20) - fCarry;
		fCarry = fLength + fCarry;

		LIGHT_CYCLE_TRAIL_POINT vCur;
		while (fLength > fNotch && fLength > LCM_NOTCH_LENGTH)
		{
			float fOver = 0.0f;
			LIGHT_CYCLE_TRAIL_POINT vNotch, vNotchEnd;

			if (fDX > fDZ)
			{
				float fOffset = fNotch / fDX * fDX;
				if (fOffset + LCM_NOTCH_LENGTH > fDX)
				{
					fOver = -(fOffset + LCM_NOTCH_LENGTH - fDX);
					fOffset = fDX - LCM_NOTCH_LENGTH;
				}

				if (pNext->GetX() > pCur->GetX())
				{
					vNotch.SetX(pCur->GetX() + fOffset);
					vNotchEnd.SetX(vNotch.GetX() + LCM_NOTCH_LENGTH);
				}
				else
				{
					vNotch.SetX(pCur->GetX() - fOffset);
					vNotchEnd.SetX(vNotch.GetX() - LCM_NOTCH_LENGTH);
				}

				vNotch.SetY(pCur->GetY());
				vNotch.SetZ(pCur->GetZ());
				vNotchEnd.SetY(vNotch.GetY());
				vNotchEnd.SetZ(vNotch.GetZ());
			}
			else
			{
				float fOffset = fNotch / fDZ * fDZ;
				if (fOffset + LCM_NOTCH_LENGTH > fDZ)
				{
					fOver = -(fOffset + LCM_NOTCH_LENGTH - fDZ);
					fOffset = fDZ - LCM_NOTCH_LENGTH;
				}

				if (pNext->GetZ() > pCur->GetZ())
				{
					vNotch.SetZ(pCur->GetZ() + fOffset);
					vNotchEnd.SetZ(vNotch.GetZ() + LCM_NOTCH_LENGTH);
				}
				else
				{
					vNotch.SetZ(pCur->GetZ() - fOffset);
					vNotchEnd.SetZ(vNotch.GetZ() - LCM_NOTCH_LENGTH);
				}

				vNotch.SetX(pCur->GetX());
				vNotch.SetY(pCur->GetY());
				vNotchEnd.SetX(vNotch.GetX());
				vNotchEnd.SetY(vNotch.GetY());
			}

			AddTrailPoly(pCur, &vNotch, nRed, nGreen, nBlue, fHeight, fBase, fDerez, pTrail->fDerezStartTime, bCurrentTrail);
			AddTrailPoly(&vNotch, &vNotchEnd, nRed, nGreen, nBlue, fHeight, fBase, fDerez, pTrail->fDerezStartTime, bCurrentTrail);

			// The notch uses its own part of the texture
			LT_POLYFT4* pPoly = &m_apTrailPolys[m_nTrailBuffer][m_nTrailPolys - 1];
			pPoly->verts[0].u = 0.671875f;
			pPoly->verts[1].u = 0.953125f;
			pPoly->verts[2].u = 0.953125f;
			pPoly->verts[3].u = 0.671875f;

			vCur = vNotchEnd;
			pCur = &vCur;

			fDX = (float)fabs(vCur.GetX() - pNext->GetX());
			fDZ = (float)fabs(vCur.GetZ() - pNext->GetZ());
			fLength = (fDX > fDZ) ? fDX : fDZ;
			fCarry = fLength;

			fNotch = fOver + m_fTronUnknown20;
			bFirst = false;
		}

		AddTrailPoly(pCur, pNext, nRed, nGreen, nBlue, fHeight, fBase, fDerez, pTrail->fDerezStartTime, bCurrentTrail);
	}

	if (bCurrentTrail)
	{
		AddStartPoly(&vHeadStart, &vHeadEnd, nRed, nGreen, nBlue, fHeight, fBase, fDerez);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::AddTrailPoly
//
//	PURPOSE:	RETAIL: 0x10013970 (bAlive is not read)
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::AddTrailPoly(LIGHT_CYCLE_TRAIL_POINT* pvStart, LIGHT_CYCLE_TRAIL_POINT* pvEnd, uint8 nRed, uint8 nGreen, uint8 nBlue,
	float fHeight, float fBase, float fDerez, float fDerezStartTime, bool bAlive)
{
	if (m_nTrailPolys >= LCM_TRAIL_POLYS_PER_BUFFER)
	{
		m_nTrailPolys = 0;
		if (++m_nTrailBuffer >= m_nTrailBuffers)
		{
			AllocTrailBuffer();
		}
	}

	// A derezzing wall sinks
	float fSink = 0.0f;
	if (fDerez > 0.0f)
	{
		fSink = (fHeight + fBase) * fDerez;
	}

	LT_POLYFT4* pPoly = &m_apTrailPolys[m_nTrailBuffer][m_nTrailPolys];

	pPoly->verts[0].x = pvStart->GetX();
	pPoly->verts[0].y = pvStart->GetY() + fHeight - fSink;
	pPoly->verts[0].z = pvStart->GetZ();
	pPoly->verts[0].u = 0.1f;
	pPoly->verts[0].v = 0.00390625f;

	pPoly->verts[1].x = pvEnd->GetX();
	pPoly->verts[1].y = pvEnd->GetY() + fHeight - fSink;
	pPoly->verts[1].z = pvEnd->GetZ();
	pPoly->verts[1].u = 0.3f;
	pPoly->verts[1].v = 0.00390625f;

	pPoly->verts[2].x = pvEnd->GetX();
	pPoly->verts[2].y = pvEnd->GetY() - fBase;
	pPoly->verts[2].z = pvEnd->GetZ();
	pPoly->verts[2].u = 0.3f;
	pPoly->verts[2].v = 0.99609375f;

	pPoly->verts[3].x = pvStart->GetX();
	pPoly->verts[3].y = pvStart->GetY() - fBase;
	pPoly->verts[3].z = pvStart->GetZ();
	pPoly->verts[3].u = 0.1f;
	pPoly->verts[3].v = 0.99609375f;

	if (fDerezStartTime > 0.0f)
	{
		// Fade to grey in the first two seconds of the derez
		float fTime = g_pLTClient->GetTime() - fDerezStartTime;
		if (fTime > 2.0f)
		{
			pPoly->rgba.r = 0x30;
			pPoly->rgba.g = 0x30;
			pPoly->rgba.b = 0x30;
		}
		else
		{
			float fFade = fTime * 0.5f;
			pPoly->rgba.r = nRed + (uint8)((0x30 - nRed) * fFade);
			pPoly->rgba.g = nGreen + (uint8)((0x30 - nGreen) * fFade);
			pPoly->rgba.b = nBlue + (uint8)((0x30 - nBlue) * fFade);
		}
	}
	else
	{
		pPoly->rgba.r = nRed;
		pPoly->rgba.g = nGreen;
		pPoly->rgba.b = nBlue;
	}

	pPoly->rgba.a = 0xFF;

	m_nTrailPolys++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::AddStartPoly
//
//	PURPOSE:	RETAIL: 0x10013b80
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::AddStartPoly(LIGHT_CYCLE_TRAIL_POINT* pvStart, LIGHT_CYCLE_TRAIL_POINT* pvEnd, uint8 nRed, uint8 nGreen, uint8 nBlue,
	float fHeight, float fBase, float fDerez)
{
	float fSink = 0.0f;
	if (fDerez > 0.0f)
	{
		fSink = (fHeight + fBase) * fDerez;
	}

	LT_POLYFT4* pPoly = &m_aStartPolys[m_nStartPolys];

	pPoly->verts[0].x = pvStart->GetX();
	pPoly->verts[0].y = pvStart->GetY() + fHeight - fSink;
	pPoly->verts[0].z = pvStart->GetZ();
	pPoly->verts[0].u = 0.99609375f;
	pPoly->verts[0].v = 0.00390625f;

	pPoly->verts[1].x = pvEnd->GetX();
	pPoly->verts[1].y = pvEnd->GetY() + fHeight - fSink;
	pPoly->verts[1].z = pvEnd->GetZ();
	pPoly->verts[1].u = 0.0078125f;
	pPoly->verts[1].v = 0.00390625f;

	pPoly->verts[2].x = pvEnd->GetX();
	pPoly->verts[2].y = pvEnd->GetY() - fBase;
	pPoly->verts[2].z = pvEnd->GetZ();
	pPoly->verts[2].u = 0.0078125f;
	pPoly->verts[2].v = 0.99609375f;

	pPoly->verts[3].x = pvStart->GetX();
	pPoly->verts[3].y = pvStart->GetY() - fBase;
	pPoly->verts[3].z = pvStart->GetZ();
	pPoly->verts[3].u = 0.99609375f;
	pPoly->verts[3].v = 0.99609375f;

	pPoly->rgba.r = nRed;
	pPoly->rgba.g = nGreen;
	pPoly->rgba.b = nBlue;
	pPoly->rgba.a = 0xFF;

	if (++m_nStartPolys >= LCM_MAX_START_POLYS)
	{
		m_nStartPolys = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::CyclistMovementChanged
//
//	PURPOSE:	Sends the local cycle's turn or speed change. RETAIL:
//				0x10013c90
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::CyclistMovementChanged(LIGHT_CYCLIST* pCyclist, bool bTurned, float fAccel, float fWallAccel, uint32 nFlags)
{
	UpdateCycleEffects(pCyclist, g_pMoveMgr->GetVehicleMgr()->GetVehicleCruiseSnd(), bTurned, fAccel, fWallAccel, nFlags, LTNULL);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_TURBO);
	cMsg.WriteObject(g_pMoveMgr->GetServerObject());
	cMsg.Writefloat(pCyclist->fAccelRate);
	cMsg.Writefloat(pCyclist->fSpeed);
	cMsg.Writebool(bTurned);
	cMsg.Writefloat(fAccel);
	cMsg.Writefloat(fWallAccel);
	cMsg.Writeuint32(nFlags);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::UpdateCycleEffects
//
//	PURPOSE:	The sparks of a cycle near a wall and the pitch of its
//				engine sound. RETAIL: 0x10013e40. PORT: for the others'
//				cycles (pFX) retail works on their CLightCycleFX (its
//				sparks FX link at +0x58 and its 0x1012d8c0, which starts
//				them), and for the local cycle it starts the sparks with
//				the vehicle manager's 0x101224f0: neither is in our build.
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::UpdateCycleEffects(LIGHT_CYCLIST* pCyclist, HLTSOUND hSound, bool bTurned,
	float fAccel, float fWallAccel, uint32 nFlags, CSpecialFX* pFX)
{
	// The sparks along the wall the cycle accelerates on
	if (fWallAccel > 0.0f && pCyclist->nWallDist < 20)
	{
		if (bTurned)
		{
			if (pCyclist->hObject == g_pMoveMgr->GetObject())
			{
				CLIENTFX_LINK* pLink = g_pMoveMgr->GetVehicleMgr()->GetTronUnknownFX2148();
				if (pLink->IsValid())
				{
					g_pClientFXMgr->ShutdownClientFX(pLink);
					pLink->ClearLink();
				}
			}
		}

		if (pCyclist->nWallDist > 0)
		{
			LTVector vOffset = pCyclist->vForward.Cross(LTVector(0.0f, 1.0f, 0.0f));
			if (pCyclist->m_bTronUnknownAC)
			{
				vOffset = -vOffset;
			}

			vOffset *= (float)(pCyclist->nWallDist - 1);

			// PORT: retail starts "LC_sparks" at vOffset (see above)
		}
	}
	else
	{
		if (pCyclist->hObject == g_pMoveMgr->GetObject())
		{
			CLIENTFX_LINK* pLink = g_pMoveMgr->GetVehicleMgr()->GetTronUnknownFX2148();
			if (pLink->IsValid())
			{
				g_pClientFXMgr->ShutdownClientFX(pLink);
				pLink->ClearLink();
			}
		}
	}

	if (!hSound)
	{
		return;
	}

	float fTurnPitch;
	if (bTurned)
	{
		fTurnPitch = GetSpeedPitch(pCyclist, pCyclist->fSpeed);

		// The others' cycles play their turn sound
		HOBJECT hObj = pCyclist->hObject;
		if (hObj != g_pMoveMgr->GetObject())
		{
			char szModel[128];
			char szSound[128];
			g_pPlayerMgr->TronUnknownVirtual7C(hObj, szModel, szSound, LTNULL);
			g_pClientSoundMgr->PlaySoundFromObject(hObj, szSound, -1.0f, SOUNDPRIORITY_PLAYER_HIGH,
				PLAYSOUND_3D | PLAYSOUND_ATTACHED, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f, 0, 1.0f);
		}
	}

	float fPitch;
	int nTime;

	if (pCyclist->fAccelRate > 0.0f)
	{
		float fTargetSpeed = pCyclist->fBaseSpeed;
		if (nFlags & LCMF_SPEEDZONE)
		{
			fTargetSpeed = m_ButeMgr.GetMaxSpeedZoneSpeed() * m_fGameSpeedScalar;
		}
		else if (pCyclist->bTurboCursed)
		{
			fTargetSpeed = m_ButeMgr.GetMaxTurboCurseSpeed();
		}
		else if (pCyclist->bTurbo)
		{
			fTargetSpeed = pCyclist->fMaxTurboSpeed;
		}
		else
		{
			if (fAccel > 0.0f)
			{
				fTargetSpeed += pCyclist->fMaxManualSpeedAdd;
			}

			if (fWallAccel > 0.0f)
			{
				fTargetSpeed += pCyclist->fMaxExternalSpeedAdd;
			}
		}

		nTime = (int)((fTargetSpeed - pCyclist->fSpeed) / pCyclist->fAccelRate * 1000.0f);
		fPitch = GetSpeedPitch(pCyclist, fTargetSpeed);
	}
	else if (pCyclist->fAccelRate < 0.0f)
	{
		float fTargetSpeed;
		if (nFlags & LCMF_SLOWZONE)
		{
			fTargetSpeed = m_ButeMgr.GetMinSlowZoneSpeed() * m_fGameSpeedScalar;
		}
		else
		{
			fTargetSpeed = pCyclist->fMinSpeed;
		}

		nTime = (int)((pCyclist->fSpeed - fTargetSpeed) / pCyclist->fAccelRate * -1000.0f);
		fPitch = GetSpeedPitch(pCyclist, pCyclist->fMinSpeed);
	}
	else
	{
		if (bTurned)
		{
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual23(hSound, 0, *(uint32*)&fTurnPitch, 0);
		}
		else
		{
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual22(hSound, 0);
		}

		return;
	}

	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual22(hSound, 1);

	if (bTurned)
	{
		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual24(hSound, 0, fTurnPitch, *(uint32*)&fPitch, nTime);

		if (pCyclist->hObject == g_pMoveMgr->GetObject())
		{
			HLTSOUND hSnd2c = g_pMoveMgr->GetVehicleMgr()->GetTronUnknownSnd2c();
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual24(hSnd2c, 0, fTurnPitch, *(uint32*)&fPitch, nTime);
		}
	}
	else
	{
		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual23(hSound, 0, *(uint32*)&fPitch, nTime);

		if (pCyclist->hObject == g_pMoveMgr->GetObject())
		{
			HLTSOUND hSnd2c = g_pMoveMgr->GetVehicleMgr()->GetTronUnknownSnd2c();
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->TronUnknownVirtual23(hSnd2c, 0, *(uint32*)&fPitch, nTime);
		}
	}
}

// RETAIL: an inline linear interpolation (its use in 0x10014360 keeps both
// ends in memory); the name is ours
template<class T>
inline T LCLerp(const T& tMin, const T& tMax, float fT)
{
	return tMin + (tMax - tMin) * fT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::GetSpeedPitch
//
//	PURPOSE:	0.8 at the min speed to 1.0 at the base speed, then up to
//				1.4 at the most speed the rider and the walls add.
//				RETAIL: 0x10014360
//
// ----------------------------------------------------------------------- //

float CClientLightCycleMgr::GetSpeedPitch(LIGHT_CYCLIST* pCyclist, float fSpeed)
{
	if (fSpeed < pCyclist->fBaseSpeed)
	{
		float fT = (fSpeed - pCyclist->fMinSpeed) / (pCyclist->fBaseSpeed - pCyclist->fMinSpeed);
		return LCLerp(0.8f, 1.0f, fT);
	}

	float fT = (fSpeed - pCyclist->fBaseSpeed) / (pCyclist->fMaxExternalSpeedAdd + pCyclist->fMaxManualSpeedAdd);
	return LCLerp(1.0f, 1.4f, fT);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::UpdateCyclistDims
//
//	PURPOSE:	RETAIL: 0x100143d0; the local cycle's server object gets
//				the dims too, and with bTronUnknown the cycle is put on
//				the ground.
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::UpdateCyclistDims(LIGHT_CYCLIST* pCyclist, LTVector & vDims, bool bTronUnknown)
{
	g_pPhysicsLT->GetObjectDims(pCyclist->hObject, &vDims);

	LTVector vPos;
	g_pLTClient->GetObjectPos(pCyclist->hObject, &vPos);

	CLightCycleMgr::UpdateCyclistDims(pCyclist, vDims, false);

	g_pLTClient->GetObjectPos(pCyclist->hObject, &vPos);

	if (pCyclist->hObject != g_pMoveMgr->GetObject())
	{
		return;
	}

	g_pPhysicsLT->SetObjectDims(g_pMoveMgr->GetServerObject(), &vDims, 0);

	if (!bTronUnknown)
	{
		return;
	}

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To.Init(vPos.x, vPos.y - 10000.0f, vPos.z);
	IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GroundFilterFn;

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		vPos.y -= (vPos.y - IInfo.m_Point.y) - (vDims.y + 0.1f);
		g_pLTClient->SetObjectPos(pCyclist->hObject, &vPos, LTTRUE);
	}

	pCyclist->vLastPos = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::RemoveAllCyclists
//
//	PURPOSE:	RETAIL: 0x100145f0: also resets the light cycle roster
//				(g_pLightCycleRoster, 0x1026ee18)
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::RemoveAllCyclists()
{
	m_nTrailPolys = 0;
	m_nStartPolys = 0;
	m_nTrailBuffer = 0;

	if (g_pLightCycleRoster)
		g_pLightCycleRoster->Reset();

	CLightCycleMgr::RemoveAllCyclists();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::TrimTrail
//
//	PURPOSE:	RETAIL: 0x10014620
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::TrimTrail(LIGHT_CYCLIST* pCyclist, int nTrail, int nPoint, bool bNotify)
{
	if (CLightCycleMgr::TrimTrail(pCyclist, nTrail, nPoint, bNotify))
	{
		if (bNotify)
		{
			HOBJECT hObj = pCyclist->hObject;
			if (hObj == g_pMoveMgr->GetObject())
			{
				hObj = g_pMoveMgr->GetServerObject();
			}

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
			cMsg.Writeuint8(LCI_TRIM_TRAIL);
			cMsg.WriteObject(hObj);
			cMsg.Writeuint32(nTrail);
			cMsg.Writeuint32(nPoint);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::DeactivateShield
//
//	PURPOSE:	RETAIL: 0x10014770. Then shut the shield's client FX
//				down: the vehicle manager's link (+0x213c) for the local
//				cycle, the cycle's CLightCycleFX's (+0x4c) for the others.
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::DeactivateShield(LIGHT_CYCLIST* pCyclist, bool bNotify)
{
	CLightCycleMgr::DeactivateShield(pCyclist, bNotify);

	if (pCyclist->hObject == g_pMoveMgr->GetObject())
	{
		// RETAIL: 0x1001478e calls this and drops what it returns
		g_pMoveMgr->GetServerObject();
		CVehicleMgr* pVehicleMgr = g_pMoveMgr->GetVehicleMgr();
		if (pVehicleMgr->GetTronUnknownFX213c()->IsValid())
		{
			CLIENTFX_LINK* pLink = pVehicleMgr->GetTronUnknownFX213c();
			g_pClientFXMgr->ShutdownClientFX(pLink);
			pLink->ClearLink();
		}
	}
	else
	{
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		CLightCycleFX* pFX = psfxMgr->GetLightCycleFX(pCyclist->hObject);
		if (!pFX) return;

		if (pFX->GetTronUnknownFX4C()->IsValid())
		{
			CLIENTFX_LINK* pLink = pFX->GetTronUnknownFX4C();
			g_pClientFXMgr->ShutdownClientFX(pLink);
			pLink->ClearLink();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::TrailCut
//
//	PURPOSE:	RETAIL: 0x100147f0. PORT: Tron's CLIENTFX_CREATESTRUCT has
//				a 128 character name (NOLF2's: 32)
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::TrailCut(LightCycleCollisionInfo* pInfo)
{
	CLIENTFX_CREATESTRUCT fxInit;
	strncpy(fxInit.m_sName, "LC_DEREZ_BLUE", sizeof(fxInit.m_sName) - 1);
	fxInit.m_vPos = pInfo->vCollisionPoint;

	g_pClientFXMgr->CreateClientFX(LTNULL, fxInit, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::SetCyclistPowerup
//
//	PURPOSE:	RETAIL: 0x100148c0. PORT: retail also shows the power-up's
//				name (string 0x44c + nPowerup, or 0x47f for none) on a HUD
//				item of Tron's (0x1026edf0, its slot 0x54): not in our
//				build.
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::SetCyclistPowerup(LIGHT_CYCLIST* pCyclist, int nPowerup)
{
	if (!CLightCycleMgr::SetCyclistPowerup(pCyclist, nPowerup))
	{
		return false;
	}

	if (pCyclist->hObject != g_pMoveMgr->GetObject())
	{
		return false;
	}

	if (nPowerup != LC_NO_POWERUP)
	{
		LCPOWERUPDATA* pData = g_pLightCycleButeMgr->GetPowerupData(nPowerup);
		if (pData->nActivateTextId != kNumLCTextIds)
		{
			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(pData->nActivateTextId));
		}
	}

	g_pHUDMgr->QueueUpdate(kHUDAll);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::ActivatePowerup
//
//	PURPOSE:	RETAIL: 0x10014990. PORT: the shield, the turbo and the
//				turbo curse start their FX with the vehicle manager's
//				0x10122280, 0x10122350 and 0x10122420 for the local cycle
//				and with the cycle's CLightCycleFX (0x1012d650, 0x1012d720,
//				0x1012d7f0) for the others: none of them is in our build.
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::ActivatePowerup(LIGHT_CYCLIST* pCyclist, LTVector* pvPos)
{
	if (!pCyclist)
	{
		return false;
	}

	int nPowerup = pCyclist->nPowerup;
	if (!CLightCycleMgr::ActivatePowerup(pCyclist, pvPos))
	{
		return false;
	}

	bool bCreateFX = true;

	char szFX[512];
	sprintf(szFX, "%s%s%s", LCPOWERUP_SOUND_PREFIX, g_aszLCPowerupNames[nPowerup], LCPOWERUP_SOUND_ACTIVE);

	switch (nPowerup)
	{
		case kLCPowerup_Shield:
		{
			if (pCyclist->hObject != g_pMoveMgr->GetObject())
			{
				// PORT: the cycle's CLightCycleFX (see above)
				bCreateFX = false;
				break;
			}

			// PORT: the vehicle manager's shield FX (see above)
			ShowMessage(LoadTempString(0x460));
			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateShieldBreak));
			bCreateFX = false;
		}
		break;

		case kLCPowerup_Turbo:
		{
			if (pCyclist->hObject != g_pMoveMgr->GetObject())
			{
				// PORT: the cycle's CLightCycleFX (see above)
				bCreateFX = false;
				break;
			}

			// PORT: the vehicle manager's turbo FX (see above)
			ShowMessage(LoadTempString(0x461));
			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateTurboBoost));
			bCreateFX = false;
		}
		break;

		case kLCPowerup_TurboCurse:
		{
			LIGHT_CYCLIST* pFirst = GetNextCyclist(LTNULL, true);
			LIGHT_CYCLIST* pOther = pFirst;
			while (pOther)
			{
				// PORT: the cursed cycle's FX (see above)
				pOther = GetNextCyclist(pOther->hObject, true);
				if (pOther == pFirst)
				{
					break;
				}
			}

			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_AnnounceTurboCurse));
			ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateTurboCurse),
				GetCyclistName(pCyclist->hObject).c_str()));
		}
		break;

		case kLCPowerup_Missile:
		{
			if (pCyclist->hObject == g_pMoveMgr->GetObject())
			{
				ShowMessage(LoadTempString(0x463));
				PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateMissileBlaster));
			}

			bCreateFX = false;
		}
		break;

		case kLCPowerup_TrailSpike:
		{
			if (pCyclist->hObject == g_pMoveMgr->GetObject())
			{
				ShowMessage(LoadTempString(0x462));
				PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateWallSpike));
			}
		}
		break;

		case kLCPowerup_WallExtender:
		{
			if (pCyclist->hObject == g_pMoveMgr->GetObject())
			{
				ShowMessage(LoadTempString(0x464));
				PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateWallExtender));
			}
		}
		break;

		case kLCPowerup_TrailReset:
		{
			PlayCycleDialogue(g_pLightCycleButeMgr->GetRandomTextId(kLCText_AnnounceWallReset));
			ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateTrailReset),
				GetCyclistName(pCyclist->hObject).c_str()));
		}
		break;

		case kLCPowerup_ForceUse:
		{
			ShowMessage(FormatTempString(g_pLightCycleButeMgr->GetRandomTextId(kLCText_ActivateForceUse),
				GetCyclistName(pCyclist->hObject).c_str()));
		}
		break;

		case kLCPowerup_Grab:
		case kNumLCPowerupTypes:
		break;

		case kLCPowerup_Random:
		default:
			return false;
	}

	char szSound[512];
	sprintf(szSound, "lcpu_%s", g_aszLCPowerupNames[nPowerup]);
	g_pClientSoundMgr->PlaySoundFromObject(pCyclist->hObject, szSound, -1.0f, SOUNDPRIORITY_PLAYER_HIGH,
		PLAYSOUND_3D | PLAYSOUND_ATTACHED, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f, 0, 1.0f);

	if (bCreateFX)
	{
		CLIENTFX_CREATESTRUCT fxInit(szFX, 0, pCyclist->hObject);
		g_pLTClient->GetObjectPos(pCyclist->hObject, &fxInit.m_vPos);
		g_pClientFXMgr->CreateClientFX(LTNULL, fxInit, LTTRUE);
	}

	g_pHUDMgr->QueueUpdate(kHUDAll);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::SendActivatePowerup
//
//	PURPOSE:	RETAIL: 0x10014f30
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::SendActivatePowerup(LIGHT_CYCLIST* pCyclist)
{
	if (!pCyclist)
	{
		return false;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
	cMsg.Writeuint8(LCI_ACTIVATE_POWERUP);
	cMsg.WriteObject(g_pMoveMgr->GetServerObject());
	cMsg.WriteLTVector(pCyclist->vLastPos);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::ReadCyclists
//
//	PURPOSE:	RETAIL: 0x10015050
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::ReadCyclists(ILTMessage_Read* pMsg)
{
	uint8 nCyclists = pMsg->Readuint8();
	for (int i = 0; i < nCyclists; i++)
	{
		ReadCyclist(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::ReadCyclist
//
//	PURPOSE:	A cyclist the client does not know yet (the server's
//				WriteCyclist). RETAIL: 0x10015090
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::ReadCyclist(ILTMessage_Read* pMsg)
{
	HOBJECT hObj = pMsg->ReadObject();
	if (hObj == g_pMoveMgr->GetServerObject())
	{
		hObj = g_pMoveMgr->GetObject();
	}

	LIGHT_CYCLIST* pCyclist = FindCyclist(hObj);
	if (pCyclist)
	{
		return;
	}

	pCyclist = debug_new(LIGHT_CYCLIST);
	m_collCyclists.push_back(pCyclist);

	pCyclist->hObject = hObj;
	pCyclist->fStartDerezTime = pMsg->Readfloat();
	pCyclist->bUpdating = pMsg->Readbool();
	pCyclist->bRemove = pMsg->Readbool();
	pCyclist->vForward = pMsg->ReadLTVector();
	pCyclist->fMaxTrailLength = pMsg->Readfloat();
	pCyclist->nCycleType = pMsg->Readuint8();
	pMsg->ReadString(pCyclist->szCycleColor, sizeof(pCyclist->szCycleColor));
	pCyclist->nRed = pMsg->Readuint8();
	pCyclist->nGreen = pMsg->Readuint8();
	pCyclist->nBlue = pMsg->Readuint8();
	pCyclist->fAccelRate = pMsg->Readfloat();
	pCyclist->fBaseSpeed = pMsg->Readfloat();
	pCyclist->fSpeed = pMsg->Readfloat();
	pCyclist->fMaxSpeed = pMsg->Readfloat();
	pCyclist->fTurboAccelRate = pMsg->Readfloat();
	pCyclist->fBrakeDecelRate = pMsg->Readfloat();
	pCyclist->fMaxManualSpeedAdd = pMsg->Readfloat();
	pCyclist->fMaxExternalSpeedAdd = pMsg->Readfloat();
	pCyclist->fMaxTurboSpeed = pMsg->Readfloat();
	pCyclist->fMinSpeed = pMsg->Readfloat();
	pCyclist->fNormalAccelRate = pMsg->Readfloat();
	pCyclist->fNormalDecelRate = pMsg->Readfloat();
	pCyclist->fMaxWallAccelRate = pMsg->Readfloat();
	pCyclist->fTurnSpeedLossPercent = pMsg->Readfloat();
	pCyclist->nMaxWallDist = pMsg->Readuint32();
	pCyclist->fHeight = pMsg->Readfloat();
	pCyclist->fWidth = pMsg->Readfloat();
	pCyclist->fLength = pMsg->Readfloat();
	pCyclist->bIsPlayer = pMsg->Readbool();
	pCyclist->nCycleDialog = pMsg->Readuint32();
	pCyclist->nPowerup = pMsg->Readuint8();
	pCyclist->nFlags = pMsg->Readuint32();
	pCyclist->fPowerupStartTime = pMsg->Readfloat();
	pCyclist->fPowerupDuration = pMsg->Readfloat();
	pCyclist->nClientID = pMsg->Readuint32();
	pCyclist->m_bTronUnknownAC = pMsg->Readbool();

	if (pMsg->Readbool())
	{
		pCyclist->vSpikeDir = pMsg->ReadLTVector();
		pCyclist->vSpikeDir2 = pMsg->ReadLTVector();
		pCyclist->vSpikePos = pMsg->ReadLTVector();
		pCyclist->fSpikeFullTime = pMsg->Readfloat();
	}

	uint8 nTrails = pMsg->Readuint8();
	for (int i = 0; i < nTrails; i++)
	{
		ReadTrail(pCyclist, pMsg);
	}

	if (g_pLightCycleRoster)
	{
		g_pLightCycleRoster->AddCyclist(pCyclist);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::ReadTrail
//
//	PURPOSE:	One trail of the cyclist (the server's WriteTrail, after
//				the trail's kind: 0 the current trail, 1 and 2 the trail
//				spike's). RETAIL: 0x100155f0
//
// ----------------------------------------------------------------------- //

void CClientLightCycleMgr::ReadTrail(LIGHT_CYCLIST* pCyclist, ILTMessage_Read* pMsg)
{
	LIGHT_CYCLE_TRAIL* pTrail = debug_new(LIGHT_CYCLE_TRAIL);
	pCyclist->collTrails.push_back(pTrail);

	uint8 nKind = pMsg->Readuint8();
	if (nKind == 0)
	{
		pCyclist->pCurTrail = pTrail;
	}
	else if (nKind == 1)
	{
		pCyclist->pSpikeTrail = pTrail;
	}
	else if (nKind == 2)
	{
		pCyclist->pSpikeTrail2 = pTrail;
	}

	pTrail->m_bTronUnknownC = pMsg->Readbool();
	pTrail->fDerezStartTime = pMsg->Readfloat();

	uint32 nPoints = pMsg->Readuint32();
	for (uint32 i = 0; i < nPoints; i++)
	{
		LTVector vPoint = pMsg->ReadLTVector();
		AddLightCycleTrailPoint(pCyclist, pTrail, vPoint);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientLightCycleMgr::AllocTrailBuffer
//
//	PURPOSE:	RETAIL: 0x100156f0
//
// ----------------------------------------------------------------------- //

bool CClientLightCycleMgr::AllocTrailBuffer()
{
	m_apTrailPolys[m_nTrailBuffers] = debug_newa(LT_POLYFT4, LCM_TRAIL_POLYS_PER_BUFFER);
	m_nTrailBuffers++;
	return true;
}

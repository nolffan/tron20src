// ----------------------------------------------------------------------- //
//
// MODULE  : LightCycleFX.cpp
//
// PURPOSE : A light cycle's special fx - Implementation
//
// RETAIL: Tron 2.0's, written from retail CShell.dll (0x1012cf00..
// 0x1012da52). The names of the members and of the non-virtual functions
// are ours.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LightCycleFX.h"
#include "CMoveMgr.h"
#include "PlayerMgr.h"
#include "GameClientShell.h"
#include "ClientSoundMgr.h"
#include "MsgIDs.h"
#include "FxFlags.h"
#include "CharacterFX.h"

extern CMoveMgr* g_pMoveMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::~CLightCycleFX
//
//	PURPOSE:	Destructor (RETAIL: 0x1012cf00)
//
// ----------------------------------------------------------------------- //

CLightCycleFX::~CLightCycleFX()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::Init
//
//	PURPOSE:	Init the fx from the server's message (RETAIL: 0x1012cfc0)
//
// ----------------------------------------------------------------------- //

LTBOOL CLightCycleFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!hServObj) return LTFALSE;
	if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;

	m_cs.hServerObj = hServObj;
	m_cs.Read(pMsg);

	m_hTronUnknownSnd44	= LTNULL;
	m_bTronUnknown6C	= false;

	FindCyclist();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::OnServerMessage
//
//	PURPOSE:	Read the new rider (RETAIL: 0x1012d010)
//
// ----------------------------------------------------------------------- //

LTBOOL CLightCycleFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	m_cs.Read(pMsg);
	FindCyclist();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::Update
//
//	PURPOSE:	Keep the cycle on its rider; play the derez and the
//				respawn fx (RETAIL: 0x1012d040)
//
// ----------------------------------------------------------------------- //

LTBOOL CLightCycleFX::Update()
{
	if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	if ((!m_hTronUnknown70 && !FindCyclist()) || m_bTronUnknown6A)
	{
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		return LTTRUE;
	}

	HOBJECT hCyclist = m_hTronUnknown70;
	if (hCyclist == g_pMoveMgr->GetServerObject())
	{
		// The local player's cycle is the vehicle manager's...
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		m_bTronUnknown68 = false;
		m_bTronUnknown69 = false;
		return LTTRUE;
	}

	if (hCyclist && !m_bTronUnknown6C)
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(hCyclist, &vPos);
		g_pLTClient->SetObjectPos(m_hServerObject, &vPos, true);
	}

	if (m_bTronUnknown69)
	{
		if (m_TronUnknownFX48.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX48);
			m_TronUnknownFX48.ClearLink();
		}

		HOBJECT hObj = m_hServerObject;
		if (m_hTronUnknown70) hObj = m_hTronUnknown70;

		char szFX[512];
		char szSound[512];
		if (!g_pPlayerMgr->TronUnknownVirtual70(hObj, szFX, szSound))
		{
			return LTTRUE;
		}

		CLIENTFX_CREATESTRUCT fxInit;
		LTStrCpy(fxInit.m_sName, szFX, sizeof(fxInit.m_sName));
		fxInit.m_dwFlags = FXFLAG_NOSMOOTHSHUTDOWN;
		fxInit.m_hParent = m_hServerObject;
		g_pLTClient->GetObjectPos(m_hServerObject, &fxInit.m_vPos);
		g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX48, fxInit, LTTRUE);

		g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, szSound);

		m_bTronUnknown69 = false;
		m_bTronUnknown68 = true;

		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		return LTTRUE;
	}
	else if (m_bTronUnknown68)
	{
		if (m_TronUnknownFX48.GetInstance() && !m_TronUnknownFX48.GetInstance()->IsDone())
		{
			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
			return LTTRUE;
		}

		if (m_TronUnknownFX48.IsValid())
		{
			g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX48);
			m_TronUnknownFX48.ClearLink();
		}

		HOBJECT hObj = m_hServerObject;
		if (m_hTronUnknown70) hObj = m_hTronUnknown70;
		m_bTronUnknown68 = false;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_LIGHT_CYCLE_INFO);
		cMsg.Writeuint8(LCI_START_UPDATING);
		cMsg.WriteObject(hObj);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

		if (!m_hTronUnknownSnd44)
		{
			char szSound[256];
			char szBuf[128];
			g_pPlayerMgr->TronUnknownVirtual7C(hObj, szSound, szBuf, LTNULL);
			m_hTronUnknownSnd44 = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, szSound, -1.0f,
				SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_3D | PLAYSOUND_LOOP | PLAYSOUND_ATTACHED | PLAYSOUND_GETHANDLE);
		}

		return LTTRUE;
	}

	if (m_bTronUnknown6B && !m_TronUnknownFX48.IsValid())
	{
		HOBJECT hObj = m_hServerObject;
		if (m_hTronUnknown70) hObj = m_hTronUnknown70;

		char szFX[512];
		if (g_pPlayerMgr->TronUnknownVirtual78(hObj, szFX))
		{
			CLIENTFX_CREATESTRUCT fxInit(szFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hServerObject);
			g_pLTClient->GetObjectPos(m_hServerObject, &fxInit.m_vPos);
			g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX48, fxInit, LTTRUE);
		}

		m_bTronUnknown6B = false;
	}

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundPosition(m_hTronUnknownSnd44, &vPos);

	if (m_TronUnknownFX58.GetInstance())
	{
		LTVector vFXPos = vPos + m_vTronUnknownFXOffset;
		m_TronUnknownFX58.GetInstance()->SetPos(vFXPos, LTVector(0.0f, 0.0f, 0.0f));
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::Term
//
//	PURPOSE:	Hide the cycle; stop its sound and its fx
//				(RETAIL: 0x1012d560)
//
// ----------------------------------------------------------------------- //

void CLightCycleFX::Term()
{
	if (m_hServerObject)
	{
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
	}

	if (m_hTronUnknownSnd44 && g_pLTClient->SoundMgr())
	{
		g_pLTClient->SoundMgr()->KillSound(m_hTronUnknownSnd44);
		m_hTronUnknownSnd44 = LTNULL;
	}

	if (m_TronUnknownFX48.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX48);
		m_TronUnknownFX48.ClearLink();
	}

	if (m_TronUnknownFX4C.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX4C);
		m_TronUnknownFX4C.ClearLink();
	}

	if (m_TronUnknownFX50.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX50);
		m_TronUnknownFX50.ClearLink();
	}

	if (m_TronUnknownFX54.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX54);
		m_TronUnknownFX54.ClearLink();
	}

	if (m_TronUnknownFX58.IsValid())
	{
		g_pClientFXMgr->ShutdownClientFX(&m_TronUnknownFX58);
		m_TronUnknownFX58.ClearLink();
	}

	m_bTronUnknown6A = true;
	m_bTronUnknown6C = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::CreateTronUnknownFX4C/50/54
//
//	PURPOSE:	Start a looping client fx on the cycle, unless one runs
//				already (RETAIL: 0x1012d650, 0x1012d720, 0x1012d7f0)
//
// ----------------------------------------------------------------------- //

void CLightCycleFX::CreateTronUnknownFX4C(const char* pszFX)
{
	if (m_TronUnknownFX4C.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hServerObject);
	g_pLTClient->GetObjectPos(m_hServerObject, &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX4C, fxInit, LTTRUE);
}

void CLightCycleFX::CreateTronUnknownFX50(const char* pszFX)
{
	if (m_TronUnknownFX50.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hServerObject);
	g_pLTClient->GetObjectPos(m_hServerObject, &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX50, fxInit, LTTRUE);
}

void CLightCycleFX::CreateTronUnknownFX54(const char* pszFX)
{
	if (m_TronUnknownFX54.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN, m_hServerObject);
	g_pLTClient->GetObjectPos(m_hServerObject, &fxInit.m_vPos);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX54, fxInit, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::CreateTronUnknownFX58
//
//	PURPOSE:	Start a looping client fx on the cycle and keep its
//				offset (RETAIL: 0x1012d8c0)
//
// ----------------------------------------------------------------------- //

void CLightCycleFX::CreateTronUnknownFX58(const char* pszFX, const LTVector &vOffset)
{
	if (m_TronUnknownFX58.IsValid()) return;

	CLIENTFX_CREATESTRUCT fxInit(pszFX, FXFLAG_LOOP, m_hServerObject);
	g_pClientFXMgr->CreateClientFX(&m_TronUnknownFX58, fxInit, LTTRUE);

	m_vTronUnknownFXOffset = vOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::ShowTronUnknownFX48
//
//	PURPOSE:	Show or hide the cycle's fx (RETAIL: 0x1012d990)
//
// ----------------------------------------------------------------------- //

void CLightCycleFX::ShowTronUnknownFX48(HOBJECT hObj, bool bShow)
{
	char szBuf[512];
	if (!g_pPlayerMgr->TronUnknownVirtual78(hObj, szBuf)) return;
	if (m_bTronUnknown68) return;
	if (m_bTronUnknown69) return;
	if (!m_TronUnknownFX48.GetInstance()) return;

	if (bShow)
	{
		m_TronUnknownFX48.GetInstance()->Show();
	}
	else
	{
		m_TronUnknownFX48.GetInstance()->Hide();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightCycleFX::FindCyclist
//
//	PURPOSE:	Find the rider's object: the local player's out of the
//				multiplayer light cycle game (RETAIL: 0x1012da00)
//
// ----------------------------------------------------------------------- //

bool CLightCycleFX::FindCyclist()
{
	if (m_cs.nClientID == 0xff) return true;

	if (GetGameType() != eGameTypeMultiplayerLightcycles)
	{
		m_hTronUnknown70 = g_pMoveMgr->GetServerObject();
		return true;
	}

	CSFXMgr* pSFXMgr = g_pGameClientShell->GetSFXMgr();
	if (pSFXMgr)
	{
		CCharacterFX* pCharFX = pSFXMgr->GetCharacterFromClientID(m_cs.nClientID);
		if (pCharFX)
		{
			m_hTronUnknown70 = pCharFX->GetServerObj();
			return true;
		}
	}

	return false;
}
